// ==================== CONSTANTS ====================
const CONFIG = {
  POLL_INTERVAL: 500,
  POLL_TIMEOUT: 60000,
  MAX_POLL_ATTEMPTS: 120,
  MAX_FILE_SIZE: 10 * 1024 * 1024, // 10MB
  RELOAD_DELAY: 3000,
  AUTH_STORAGE_KEY: "ota_auth",
};

// ==================== AUTHENTICATION ====================
class Auth {
  static login(username, password) {
    const credentials = btoa(`${username}:${password}`);
    sessionStorage.setItem(CONFIG.AUTH_STORAGE_KEY, credentials);
    return credentials;
  }

  static logout() {
    sessionStorage.removeItem(CONFIG.AUTH_STORAGE_KEY);
  }

  static getAuthHeader() {
    const credentials = sessionStorage.getItem(CONFIG.AUTH_STORAGE_KEY);
    return credentials ? `Basic ${credentials}` : null;
  }

  static isAuthenticated() {
    return sessionStorage.getItem(CONFIG.AUTH_STORAGE_KEY) !== null;
  }
}

// ==================== UI HELPERS ====================
class UI {
  static showMessage(message, container, type = "info") {
    const el = document.getElementById(container);
    if (!el) return;

    el.className = `message ${type}`;
    el.textContent = message;
    el.classList.remove("hidden");

    // Auto-hide after 5 seconds for non-error messages
    if (type !== "error") {
      setTimeout(() => el.classList.add("hidden"), 5000);
    }
  }

  static clearMessage(container) {
    const el = document.getElementById(container);
    if (el) {
      el.classList.add("hidden");
      el.textContent = "";
    }
  }

  static setLoading(buttonId, isLoading) {
    const btn = document.getElementById(buttonId);
    if (!btn) return;

    if (isLoading) {
      btn.disabled = true;
      btn.classList.add("loading");
      btn.dataset.originalText = btn.textContent;
    } else {
      btn.disabled = false;
      btn.classList.remove("loading");
      if (btn.dataset.originalText) {
        btn.textContent = btn.dataset.originalText;
      }
    }
  }

  static updateProgress(percent) {
    const bar = document.getElementById("progress-bar");
    const label = document.getElementById("progress-percent");
    const container = document.getElementById("progress-container");

    if (bar && label && container) {
      container.classList.remove("hidden");
      bar.style.width = `${percent}%`;
      label.textContent = `${percent}%`;
    }
  }

  static setLockState(locked) {
    if (locked) {
      document.body.classList.add("locked");
      // Disable all inputs and buttons
      const inputs = document.querySelectorAll("input, button");
      inputs.forEach((el) => (el.disabled = true));
    } else {
      document.body.classList.remove("locked");
      // Enable all inputs and buttons
      const inputs = document.querySelectorAll("input, button");
      inputs.forEach((el) => (el.disabled = false));
    }
  }

  static updateBadge(status) {
    const badge = document.getElementById("badge");
    if (!badge) return;

    badge.className = "";

    switch (status) {
      case "UPDATING":
        badge.textContent = "UPDATING";
        badge.classList.add("updating");
        break;
      case "SUCCESS":
        badge.textContent = "SUCCESS";
        badge.classList.add("success");
        break;
      case "ERROR":
        badge.textContent = "ERROR";
        badge.classList.add("error");
        break;
      default:
        badge.textContent = "IDLE";
    }
  }
}

// ==================== TAB SWITCHING ====================
function switchTab(tabName) {
  // Update tab buttons
  document.querySelectorAll(".tab-btn").forEach((btn) => {
    btn.classList.remove("active");
  });
  document.getElementById(`tab-${tabName}`).classList.add("active");

  // Update tab content
  document.querySelectorAll(".tab-content").forEach((content) => {
    content.classList.remove("active");
  });
  document.getElementById(`content-${tabName}`).classList.add("active");
}

// ==================== VALIDATION ====================
class Validator {
  static isValidFirmware(file) {
    if (!file) {
      return { valid: false, error: "⚠️ Please select a firmware file." };
    }

    if (!file.name.endsWith(".bin")) {
      return { valid: false, error: "⚠️ Only .bin files are allowed." };
    }

    if (file.size > CONFIG.MAX_FILE_SIZE) {
      const sizeMB = (CONFIG.MAX_FILE_SIZE / 1024 / 1024).toFixed(0);
      return { valid: false, error: `⚠️ File size exceeds ${sizeMB}MB limit.` };
    }

    if (file.size === 0) {
      return { valid: false, error: "⚠️ File is empty." };
    }

    return { valid: true };
  }

  static isValidURL(url) {
    if (!url || url.trim() === "") {
      return { valid: false, error: "⚠️ Please enter a URL." };
    }

    try {
      const urlObj = new URL(url);
      if (!["http:", "https:"].includes(urlObj.protocol)) {
        return {
          valid: false,
          error: "⚠️ URL must use HTTP or HTTPS protocol.",
        };
      }

      if (!url.endsWith(".bin")) {
        return { valid: false, error: "⚠️ URL must point to a .bin file." };
      }

      return { valid: true };
    } catch (e) {
      return { valid: false, error: "⚠️ Invalid URL format." };
    }
  }
}

// ==================== API CALLS ====================
async function fetchWithAuth(url, options = {}) {
  const authHeader = Auth.getAuthHeader();

  const headers = {
    ...options.headers,
  };

  if (authHeader) {
    headers["Authorization"] = authHeader;
  }

  const response = await fetch(url, {
    ...options,
    headers,
  });

  // Check for authentication errors
  if (response.status === 401) {
    Auth.logout();
    showLoginScreen();
    throw new Error("Authentication failed. Please login again.");
  }

  return response;
}

async function loadStatus() {
  try {
    const response = await fetchWithAuth("/status");
    const data = await response.json();

    document.getElementById("version").textContent = data.version || "-";
    document.getElementById("uptime").textContent = formatUptime(
      data.uptime || 0
    );
  } catch (err) {
    console.error("Failed to load status:", err);
  }
}

async function loadOTAVersion() {
  try {
    const response = await fetch("/version");
    const data = await response.json();
    document.getElementById("ota-version").textContent = data.version || "-";
  } catch (err) {
    console.error("Failed to load OTA version:", err);
    document.getElementById("ota-version").textContent = "Unknown";
  }
}

// ==================== POLLING ====================
let pollAttempts = 0;
let lastProgress = 0;
let currentButtonId = null;
let uploadStarted = false;

async function poll() {
  if (pollAttempts >= CONFIG.MAX_POLL_ATTEMPTS) {
    UI.updateBadge("ERROR");
    UI.showMessage(
      "❌ Update timeout. Please check device status.",
      "upload-msg",
      "error"
    );
    pollAttempts = 0;
    if (currentButtonId) {
      UI.updateProgress(0);
    }
    UI.setLockState(false);
    return;
  }

  try {
    const response = await fetchWithAuth("/ota-progress");
    const data = await response.json();
    const progress = data.progress || 0;

    UI.updateProgress(progress);
    lastProgress = progress;

    if (progress < 100) {
      UI.updateBadge("UPDATING");
      pollAttempts++;
      setTimeout(poll, CONFIG.POLL_INTERVAL);
    } else {
      handleUpdateSuccess();
    }
  } catch (err) {
    console.log("Poll error:", err.message);

    if (uploadStarted && pollAttempts < 5) {
      console.log("Upload was successful, ESP32 is restarting...");
      handleUpdateSuccess();
      return;
    }

    if (lastProgress >= 95) {
      console.log("Update complete, ESP32 is restarting...");
      handleUpdateSuccess();
      return;
    }

    pollAttempts++;

    if (pollAttempts > 10) {
      UI.updateBadge("ERROR");
      UI.showMessage(
        "❌ Connection lost. Device may be restarting.",
        "upload-msg",
        "warning"
      );
      if (currentButtonId) {
        UI.updateProgress(0);
      }
      UI.setLockState(false);
      return;
    }

    setTimeout(poll, CONFIG.POLL_INTERVAL);
  }
}

function handleUpdateSuccess() {
  UI.updateBadge("SUCCESS");
  UI.updateProgress(100);

  const statusText = document.getElementById("progress-status");
  if (statusText) statusText.textContent = "✅ Update Complete! Restarting...";

  const progressBar = document.getElementById("progress-bar");
  if (progressBar) progressBar.style.backgroundColor = "var(--success)";

  const msgContainer =
    currentButtonId === "upload-btn" ? "upload-msg" : "url-msg";
  UI.showMessage(
    "✅ Update successful! Restarting...",
    msgContainer,
    "success"
  );

  setTimeout(() => location.reload(), CONFIG.RELOAD_DELAY);
}

// ==================== EVENT HANDLERS ====================
async function handleLogin(event) {
  event.preventDefault();

  const username = document.getElementById("user").value;
  const password = document.getElementById("pass").value;

  UI.clearMessage("login-msg");
  UI.setLoading("login-btn", true);

  try {
    const credentials = Auth.login(username, password);
    const response = await fetchWithAuth("/status");

    if (!response.ok) {
      throw new Error("Authentication failed");
    }

    document.getElementById("login").classList.add("hidden");
    document.getElementById("panel").classList.remove("hidden");

    loadStatus();
  } catch (err) {
    Auth.logout();
    UI.showMessage("❌ Invalid username or password.", "login-msg", "error");
  } finally {
    UI.setLoading("login-btn", false);
  }
}

async function handleUpload() {
  const fileInput = document.getElementById("file");
  const file = fileInput.files[0];

  UI.clearMessage("upload-msg");

  const validation = Validator.isValidFirmware(file);
  if (!validation.valid) {
    UI.showMessage(validation.error, "upload-msg", "error");
    return;
  }

  UI.setLockState(true);
  UI.updateBadge("UPDATING");
  currentButtonId = "upload-btn";

  document.getElementById("progress-status").textContent =
    "Uploading Firmware...";
  UI.updateProgress(0);

  const formData = new FormData();
  formData.append("update", file);

  const xhr = new XMLHttpRequest();

  xhr.upload.onprogress = function (e) {
    if (e.lengthComputable) {
      const percent = Math.round((e.loaded / e.total) * 100);
      UI.updateProgress(percent);
    }
  };

  xhr.onload = function () {
    if (xhr.status === 200) {
      handleUpdateSuccess();
    } else {
      UI.showMessage(
        "❌ Upload failed: " + xhr.statusText,
        "upload-msg",
        "error"
      );
      UI.updateBadge("ERROR");
      UI.setLockState(false);
      document.getElementById("progress-container").classList.add("hidden");
    }
  };

  xhr.onerror = function () {
    handleUpdateSuccess();
  };

  xhr.open("POST", "/update");
  const authHeader = Auth.getAuthHeader();
  if (authHeader) {
    xhr.setRequestHeader("Authorization", authHeader);
  }

  xhr.send(formData);
}

async function handleUpdateFromURL() {
  const urlInput = document.getElementById("url");
  const url = urlInput.value.trim();

  UI.clearMessage("url-msg");

  const validation = Validator.isValidURL(url);
  if (!validation.valid) {
    UI.showMessage(validation.error, "url-msg", "error");
    return;
  }

  UI.setLockState(true);
  UI.updateBadge("UPDATING");
  currentButtonId = "url-btn";

  document.getElementById("progress-status").textContent =
    "Requesting Update...";
  UI.updateProgress(0);

  try {
    const response = await fetchWithAuth("/update-url", {
      method: "POST",
      headers: {
        "Content-Type": "application/x-www-form-urlencoded",
      },
      body: `url=${encodeURIComponent(url)}`,
    });

    if (!response.ok) {
      throw new Error("URL update failed");
    }

    UI.showMessage("✅ Update from URL started!", "url-msg", "success");

    currentButtonId = "url-btn";
    uploadStarted = true;
    pollAttempts = 0;
    lastProgress = 0;
    setTimeout(poll, CONFIG.POLL_INTERVAL);
  } catch (err) {
    console.error("Update error:", err);
    UI.showMessage(
      "❌ Update failed. Please check the URL.",
      "url-msg",
      "error"
    );
    UI.updateBadge("ERROR");
    UI.setLockState(false);
    document.getElementById("progress-container").classList.add("hidden");
    uploadStarted = false;
  }
}

function formatUptime(seconds) {
  const days = Math.floor(seconds / 86400);
  const hours = Math.floor((seconds % 86400) / 3600);
  const minutes = Math.floor((seconds % 3600) / 60);
  const secs = seconds % 60;

  if (days > 0) {
    return `${days}d ${hours}h ${minutes}m`;
  } else if (hours > 0) {
    return `${hours}h ${minutes}m ${secs}s`;
  } else if (minutes > 0) {
    return `${minutes}m ${secs}s`;
  } else {
    return `${secs}s`;
  }
}

function showLoginScreen() {
  document.getElementById("panel").classList.add("hidden");
  document.getElementById("login").classList.remove("hidden");
  UI.clearMessage("login-msg");
  UI.clearMessage("panel-msg");
  UI.clearMessage("upload-msg");
  UI.clearMessage("url-msg");
}

document.addEventListener("DOMContentLoaded", () => {
  loadOTAVersion();
  if (Auth.isAuthenticated()) {
    fetchWithAuth("/status")
      .then((response) => {
        if (response.ok) {
          document.getElementById("login").classList.add("hidden");
          document.getElementById("panel").classList.remove("hidden");
          loadStatus();
        } else {
          Auth.logout();
        }
      })
      .catch(() => {
        Auth.logout();
      });
  }
});
