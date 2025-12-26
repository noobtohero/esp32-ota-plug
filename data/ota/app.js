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

  static updateProgress(percent, buttonId = null) {
    // Update button progress if buttonId is provided
    if (buttonId) {
      const btn = document.getElementById(buttonId);
      const progressText = btn?.querySelector(".btn-progress");

      if (btn && progressText) {
        btn.style.setProperty("--progress", `${percent}%`);
        progressText.textContent = `${percent}%`;

        if (percent > 0 && percent < 100) {
          btn.classList.add("uploading");
          progressText.classList.remove("hidden");
        } else {
          btn.classList.remove("uploading");
          if (percent === 0) {
            progressText.classList.add("hidden");
          }
        }
      }
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
let currentButtonId = null; // Track which button is being updated

async function poll() {
  if (pollAttempts >= CONFIG.MAX_POLL_ATTEMPTS) {
    UI.updateBadge("ERROR");
    UI.showMessage(
      "❌ Update timeout. Please check device status.",
      "panel-msg",
      "error"
    );
    pollAttempts = 0;
    if (currentButtonId) {
      UI.updateProgress(0, currentButtonId);
    }
    return;
  }

  try {
    const response = await fetchWithAuth("/ota-progress");
    const data = await response.json();
    const progress = data.progress || 0;

    UI.updateProgress(progress, currentButtonId);
    lastProgress = progress;

    if (progress < 100) {
      UI.updateBadge("UPDATING");
      pollAttempts++;
      setTimeout(poll, CONFIG.POLL_INTERVAL);
    } else {
      // Progress = 100, update complete!
      UI.updateBadge("SUCCESS");
      document.getElementById("status-msg").classList.remove("hidden");

      // ESP32 will restart, so we expect connection errors
      // Wait a bit longer before reload to let ESP32 restart
      setTimeout(() => location.reload(), CONFIG.RELOAD_DELAY);
    }
  } catch (err) {
    console.log("Poll error:", err.message);

    // If we got progress = 100 before, this error is expected (ESP32 restarting)
    if (lastProgress === 100) {
      console.log("Update complete, ESP32 is restarting...");
      UI.updateBadge("SUCCESS");
      document.getElementById("status-msg").classList.remove("hidden");
      setTimeout(() => location.reload(), CONFIG.RELOAD_DELAY);
      return;
    }

    // Otherwise, retry polling
    pollAttempts++;

    // If too many consecutive errors, might be a real problem
    if (pollAttempts > 10) {
      UI.updateBadge("ERROR");
      UI.showMessage(
        "❌ Connection lost. Device may be restarting. Please refresh manually.",
        "panel-msg",
        "warning"
      );
      if (currentButtonId) {
        UI.updateProgress(0, currentButtonId);
      }
      return;
    }

    setTimeout(poll, CONFIG.POLL_INTERVAL);
  }
}

// ==================== EVENT HANDLERS ====================
async function handleLogin(event) {
  event.preventDefault();

  const username = document.getElementById("user").value;
  const password = document.getElementById("pass").value;

  UI.clearMessage("login-msg");
  UI.setLoading("login-btn", true);

  try {
    // Create auth header and test it
    const credentials = Auth.login(username, password);

    // Test authentication by fetching status
    const response = await fetchWithAuth("/status");

    if (!response.ok) {
      throw new Error("Authentication failed");
    }

    // Success - show panel
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

  // Validate file
  const validation = Validator.isValidFirmware(file);
  if (!validation.valid) {
    UI.showMessage(validation.error, "upload-msg", "error");
    return;
  }

  UI.setLoading("upload-btn", true);
  UI.updateProgress(0, "upload-btn");
  UI.updateBadge("UPDATING");

  try {
    const formData = new FormData();
    formData.append("update", file);

    const response = await fetchWithAuth("/update", {
      method: "POST",
      body: formData,
    });

    if (!response.ok) {
      throw new Error("Upload failed");
    }

    UI.showMessage("✅ Upload started successfully!", "upload-msg", "success");

    // Start polling
    currentButtonId = "upload-btn"; // Set current button
    pollAttempts = 0;
    lastProgress = 0;
    setTimeout(poll, CONFIG.POLL_INTERVAL);
  } catch (err) {
    console.error("Upload error:", err);
    UI.showMessage(
      "❌ Upload failed. Please try again.",
      "upload-msg",
      "error"
    );
    UI.updateBadge("ERROR");
    UI.updateProgress(0, "upload-btn");
  } finally {
    UI.setLoading("upload-btn", false);
  }
}

async function handleUpdateFromURL() {
  const urlInput = document.getElementById("url");
  const url = urlInput.value.trim();

  UI.clearMessage("url-msg");

  // Validate URL
  const validation = Validator.isValidURL(url);
  if (!validation.valid) {
    UI.showMessage(validation.error, "url-msg", "error");
    return;
  }

  UI.setLoading("url-btn", true);
  UI.updateProgress(0, "url-btn");
  UI.updateBadge("UPDATING");

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

    // Start polling
    currentButtonId = "url-btn"; // Set current button
    pollAttempts = 0;
    lastProgress = 0;
    setTimeout(poll, CONFIG.POLL_INTERVAL);
  } catch (err) {
    console.error("URL update error:", err);
    UI.showMessage(
      "❌ Update from URL failed. Please check the URL and try again.",
      "url-msg",
      "error"
    );
    UI.updateBadge("ERROR");
    UI.updateProgress(0, "url-btn");
  } finally {
    UI.setLoading("url-btn", false);
  }
}

// ==================== UTILITY FUNCTIONS ====================
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

// ==================== INITIALIZATION ====================
document.addEventListener("DOMContentLoaded", () => {
  // Load OTA version on login screen
  loadOTAVersion();

  // Check if already authenticated
  if (Auth.isAuthenticated()) {
    // Try to verify authentication is still valid
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
