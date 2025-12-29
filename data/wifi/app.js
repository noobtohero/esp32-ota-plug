// Helper to get API Base URL (handle Captive Portal domains)
function getApiBase() {
  const host = window.location.hostname;
  // If accessing via weird domain (Captive Portal), force API to Gateway IP
  if (
    !/^(?:[0-9]{1,3}\.){3}[0-9]{1,3}$/.test(host) &&
    !host.endsWith(".local")
  ) {
    return "http://192.168.4.1";
  }
  return ""; // Relative path
}

function scanWifi() {
  const btn = document.getElementById("scan-btn");
  const spinner = document.getElementById("spinner");
  const list = document.getElementById("networks-list");

  btn.disabled = true;
  spinner.classList.remove("hidden");
  list.classList.add("hidden");

  // 1. Start async scan
  fetch(getApiBase() + "/api/wifi/scan")
    .then(() => {
      // 2. Poll for results
      pollScanResult();
    })
    .catch((e) => {
      alert("Scan failed: " + e);
      btn.disabled = false;
      spinner.classList.add("hidden");
    });
}

function pollScanResult() {
  const btn = document.getElementById("scan-btn");
  const spinner = document.getElementById("spinner");
  const list = document.getElementById("networks-list");

  fetch(getApiBase() + "/api/wifi/scanresult")
    .then((r) => r.json())
    .then((data) => {
      // Still scanning?
      if (data.status === "scanning") {
        setTimeout(pollScanResult, 1000); // Poll again in 1s
        return;
      }
      // Failed?
      if (data.status === "failed") {
        alert("Scan failed");
        btn.disabled = false;
        spinner.classList.add("hidden");
        return;
      }
      // Got results (Array)
      list.innerHTML = "";
      if (data.length === 0) {
        list.innerHTML =
          '<div class="wifi-item" style="justify-content:center;">No networks found</div>';
      } else {
        data.sort((a, b) => b.rssi - a.rssi);
        data.forEach((net) => {
          const div = document.createElement("div");
          div.className = "wifi-item";
          div.innerHTML = `
            <span class="ssid-name">${net.ssid}</span>
            <span class="signal-icon">
              ${getSignalIcon(net.rssi)}
              ${net.auth ? "🔒" : ""}
            </span>
          `;
          div.onclick = () => selectWifi(net.ssid);
          list.appendChild(div);
        });
      }
      list.classList.remove("hidden");
      btn.disabled = false;
      spinner.classList.add("hidden");
    })
    .catch((e) => {
      alert("Poll failed: " + e);
      btn.disabled = false;
      spinner.classList.add("hidden");
    });
}

// SVG Icons
const RSSI_ICONS = {
  4: '<svg viewBox="0 0 24 24" width="18" height="18" fill="currentColor"><path d="M12 21L24 9a16 16 0 00-24 0l12 12z"/></svg>',
  3: '<svg viewBox="0 0 24 24" width="18" height="18" fill="currentColor" opacity="0.8"><path d="M12 19l9-9a12 12 0 00-18 0l9 9z"/></svg>',
  2: '<svg viewBox="0 0 24 24" width="18" height="18" fill="currentColor" opacity="0.6"><path d="M12 16l6-6a8 8 0 00-12 0l6 6z"/></svg>',
  1: '<svg viewBox="0 0 24 24" width="18" height="18" fill="currentColor" opacity="0.4"><path d="M12 13l3-3a4 4 0 00-6 0l3 3z"/></svg>',
};

function getSignalIcon(rssi) {
  let bars = 1;
  if (rssi > -60) bars = 4;
  else if (rssi > -70) bars = 3;
  else if (rssi > -80) bars = 2;

  return `<span style="color:${bars > 2 ? "#00e676" : "#ffca28"}">${
    RSSI_ICONS[bars]
  } ${rssi}dBm</span>`;
}

function selectWifi(ssid) {
  document.getElementById("ssid").value = ssid;
  const passInput = document.getElementById("pass");
  passInput.value = "";
  passInput.focus();
}

function connectWifi(e) {
  e.preventDefault();
  const ssid = document.getElementById("ssid").value;
  const pass = document.getElementById("pass").value;
  const btn = document.querySelector(".primary-btn");

  if (!ssid) return alert("SSID is required");

  btn.innerHTML = `<div class="spinner" style="width:14px;height:14px;border-width:2px;display:inline-block;vertical-align:middle;margin:0 5px;"></div> Connecting...`;
  btn.disabled = true;

  const data = new URLSearchParams();
  data.append("ssid", ssid);
  data.append("pass", pass);

  fetch(getApiBase() + "/api/wifi/connect", { method: "POST", body: data })
    .then((r) => r.text())
    .then((msg) => {
      btn.innerHTML = "✅ Saved!";
      btn.style.background = "#00e676";
      alert(msg); // Still alert for final confirmation
    })
    .catch((err) => {
      alert("Error: " + err);
      btn.innerHTML = "Save & Connect";
      btn.disabled = false;
    });
}

function togglePass() {
  const x = document.getElementById("pass");
  const btn = document.getElementById("eye-icon");
  if (x.type === "password") {
    x.type = "text";
    btn.textContent = "🙈";
  } else {
    x.type = "password";
    btn.textContent = "👁️";
  }
}

function resetWifi() {
  if (
    !confirm(
      "⚠️ Factory Reset WiFi Settings?\n\nThis will forget the current network and restart into AP Mode."
    )
  )
    return;

  const btn = document.querySelector(".danger-btn");
  btn.disabled = true;
  btn.innerText = "Resetting...";

  fetch(getApiBase() + "/api/wifi/reset", { method: "POST" })
    .then((r) => r.text())
    .then((msg) => {
      alert(msg);
      setTimeout(() => location.reload(), 5000);
    })
    .catch((e) => {
      alert("Error: " + e);
      btn.disabled = false;
      btn.innerText = "⚠️ Factory Reset WiFi";
    });
}
