// ModernWifi Manager - Modern, consistent UI + robust API use

// DOM Elements
const connectionStatus = document.getElementById('connection-status');
const connectionStatusDetail = document.getElementById('connection-status-detail');
const connectionBadge = document.getElementById('connection-badge');
const ipAddress = document.getElementById('ip-address');
const signalStrength = document.getElementById('signal-strength');
const signalBars = document.getElementById('signal-bars');
const networkList = document.getElementById('network-list');
const wifiForm = document.getElementById('wifi-form');
const ssidInput = document.getElementById('ssid');
const passwordInput = document.getElementById('password');
const togglePasswordBtn = document.getElementById('toggle-password');
const scanBtn = document.getElementById('scan-btn');
const resetBtn = document.getElementById('reset-btn');
const customParams = document.getElementById('custom-params');
const stepperNav = document.getElementById('stepper-nav');
const stepPanes = {
  1: document.getElementById('step-1'),
  2: document.getElementById('step-2'),
  3: document.getElementById('step-3')
};
const prev2 = document.getElementById('prev-step-2');
const next1 = document.getElementById('next-step-1');
const next2 = document.getElementById('next-step-2');
const prev3 = document.getElementById('prev-step-3');
const themeToggle = document.getElementById('theme-toggle');
const headerTitle = document.getElementById('header-title');
const headerSubtitle = document.getElementById('header-subtitle');
const footerVersion = document.getElementById('footer-version');
const footerText = document.getElementById('footer-text');
const favicon = document.getElementById('favicon');
const logoImage = document.getElementById('logo-image');
const copyrightText = document.getElementById('copyright-text');

// State
let networks = [];
let selectedNetwork = null;
let currentStep = 1;
let branding = null;
let currentTheme = 'light'; // Default theme

// Utilities
const sleep = (ms) => new Promise(r => setTimeout(r, ms));
const withTimeout = async (promise, ms = 5000) => {
  const ctrl = new AbortController();
  const id = setTimeout(() => ctrl.abort(), ms);
  try {
    const res = await promise(ctrl.signal);
    return res;
  } finally {
    clearTimeout(id);
  }
};

function showToast(message, type = 'info') {
  const container = document.getElementById('toast-container');
  if (!container) return;
  const el = document.createElement('div');
  el.className = `toast ${type}`;
  el.innerHTML = `<span class="icon">${type === 'success' ? '✅' : type === 'error' ? '❌' : 'ℹ️'}</span><span>${message}</span>`;
  container.appendChild(el);
  setTimeout(() => { el.style.opacity = '0'; el.style.transform = 'translateY(-6px)'; }, 3200);
  setTimeout(() => container.removeChild(el), 3800);
}

// Initialize
document.addEventListener('DOMContentLoaded', () => {
  loadBrandingConfig();
  initializeTheme();
  fetchStatus();
  fetchNetworks();
  fetchCustomParams();
  setupEventListeners();
  goToStep(1);
});

// Load branding configuration
async function loadBrandingConfig() {
  try {
    const response = await fetch('/branding.json');
    branding = await response.json();
    
    // Apply branding to UI elements
    if (branding) {
      // Update page title and favicon
      if (branding.brand) {
        document.title = branding.brand.name || 'WiFi Manager';
        if (favicon && branding.brand.favicon) {
          favicon.href = branding.brand.favicon;
        }
        
        // Update header title and logo
        if (headerTitle) {
          headerTitle.textContent = branding.brand.name || 'WiFi Manager';
        }
        
        // Show logo if available
        if (logoImage && branding.brand.logo) {
          logoImage.src = branding.brand.logo;
          logoImage.classList.remove('hidden');
        }
      }
      
      // Update portal text
      if (branding.portal) {
        if (headerSubtitle) {
          headerSubtitle.textContent = branding.portal.subtitle || 'WiFi Configuration Portal';
        }
        
        // Update footer text
        const footerElements = document.querySelectorAll('footer p');
        if (footerElements.length >= 2) {
          footerElements[0].textContent = branding.brand.name + ' v' + branding.brand.version;
          footerElements[1].textContent = branding.portal.footer_text || 'ESP32 WiFi Configuration Portal';
        }
      }
      
      // Apply theme colors
      if (branding.theme) {
        document.documentElement.style.setProperty('--primary-color', branding.theme.primary_color);
        document.documentElement.style.setProperty('--secondary-color', branding.theme.secondary_color);
        document.documentElement.style.setProperty('--success-color', branding.theme.success_color);
        document.documentElement.style.setProperty('--danger-color', branding.theme.danger_color);
        document.documentElement.style.setProperty('--warning-color', branding.theme.warning_color);
        document.documentElement.style.setProperty('--info-color', branding.theme.info_color);
      }
    }
  } catch (error) {
    console.error('Error loading branding configuration:', error);
  }
}

// Initialize theme based on preferences
function initializeTheme() {
  // Check if theme preference is stored in localStorage
  const savedTheme = localStorage.getItem('theme');
  
  if (savedTheme) {
    // Use saved preference
    currentTheme = savedTheme;
  } else if (branding && branding.ui && branding.ui.default_theme) {
    // Use default from branding config
    currentTheme = branding.ui.default_theme;
  } else {
    // Default to auto
    currentTheme = 'auto';
  }
  
  applyTheme(currentTheme);
}

// Apply the selected theme
function applyTheme(theme) {
  // Update current theme
  currentTheme = theme;
  
  // Save preference to localStorage
  localStorage.setItem('theme', theme);
  
  // Apply theme
  if (theme === 'dark') {
    document.body.classList.add('dark-mode');
    if (themeToggle) themeToggle.innerHTML = '<i class="fas fa-moon"></i>';
  } else if (theme === 'light') {
    document.body.classList.remove('dark-mode');
    if (themeToggle) themeToggle.innerHTML = '<i class="fas fa-sun"></i>';
  } else if (theme === 'auto') {
    // Check system preference
    if (window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches) {
      document.body.classList.add('dark-mode');
      if (themeToggle) themeToggle.innerHTML = '<i class="fas fa-adjust"></i>';
    } else {
      document.body.classList.remove('dark-mode');
      if (themeToggle) themeToggle.innerHTML = '<i class="fas fa-adjust"></i>';
    }
    
    // Listen for system theme changes
    window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', e => {
      if (currentTheme === 'auto') {
        if (e.matches) {
          document.body.classList.add('dark-mode');
        } else {
          document.body.classList.remove('dark-mode');
        }
      }
    });
  }
}

// Setup Event Listeners
function setupEventListeners() {
  // Scan button
  if (scanBtn) scanBtn.addEventListener('click', fetchNetworks);
  
  // Reset button
  if (resetBtn) resetBtn.addEventListener('click', resetSettings);
  
  // Form submission
  wifiForm.addEventListener('submit', (e) => {
    e.preventDefault();
    connectToNetwork();
  });
  
  // Toggle password visibility
  if (togglePasswordBtn) {
    togglePasswordBtn.addEventListener('click', () => {
      const type = passwordInput.getAttribute('type') === 'password' ? 'text' : 'password';
      passwordInput.setAttribute('type', type);
      togglePasswordBtn.innerHTML = type === 'password' ? '<i class="fas fa-eye"></i>' : '<i class="fas fa-eye-slash"></i>';
    });
  }
  
  // Theme toggle button
  if (themeToggle) {
    themeToggle.addEventListener('click', () => {
      // Cycle through themes: light -> dark -> auto -> light
      if (currentTheme === 'light') {
        applyTheme('dark');
      } else if (currentTheme === 'dark') {
        applyTheme('auto');
      } else {
        applyTheme('light');
      }
    });
  }

  // Stepper navigation
  if (stepperNav) {
    stepperNav.querySelectorAll('.step-pill').forEach(btn => {
      btn.addEventListener('click', () => goToStep(parseInt(btn.dataset.step, 10)));
    });
  }
  if (next1) next1.addEventListener('click', () => goToStep(2));
  if (prev2) prev2.addEventListener('click', () => goToStep(1));
  if (next2) next2.addEventListener('click', () => {
    if (!ssidInput.value.trim()) {
      showToast('Please enter SSID', 'error');
      ssidInput.focus();
      return;
    }
    goToStep(3);
  });
  if (prev3) prev3.addEventListener('click', () => goToStep(2));
}

// Fetch current connection status
async function fetchStatus() {
  try {
    const response = await fetch('/status_json');
    const data = await response.json();
    
    connectionStatus.textContent = data.status;
    connectionStatusDetail.textContent = data.status;
    ipAddress.textContent = data.ip;
    if (data.ssid) {
      const ssidEl = document.getElementById('ssid-current');
      if (ssidEl) ssidEl.textContent = data.ssid;
    }
    
    // Update UI based on connection status
    if (data.status === 'Connected') {
      // Update connection badge
      connectionBadge.classList.remove('bg-blue-200', 'bg-red-200', 'bg-yellow-200');
      connectionBadge.classList.add('bg-green-200');
      connectionStatus.classList.remove('text-blue-800', 'text-red-800', 'text-yellow-800');
      connectionStatus.classList.add('text-green-800');
      
      // Update signal strength if available from status JSON
      if (typeof data.rssi === 'number') {
        signalStrength.textContent = data.rssi + ' dBm';
        signalBars.innerHTML = generateSignalBars(data.rssi);
      }
    } else if (data.status === 'Connecting...') {
      connectionBadge.classList.remove('bg-blue-200', 'bg-red-200', 'bg-green-200');
      connectionBadge.classList.add('bg-yellow-200');
      connectionStatus.classList.remove('text-blue-800', 'text-red-800', 'text-green-800');
      connectionStatus.classList.add('text-yellow-800');
    } else {
      connectionBadge.classList.remove('bg-green-200', 'bg-yellow-200', 'bg-red-200');
      connectionBadge.classList.add('bg-blue-200');
      connectionStatus.classList.remove('text-green-800', 'text-yellow-800', 'text-red-800');
      connectionStatus.classList.add('text-blue-800');
    }
  } catch (error) {
    console.error('Error fetching status:', error);
    connectionStatus.textContent = 'Error';
    connectionStatusDetail.textContent = 'Error';
    connectionBadge.classList.remove('bg-blue-200', 'bg-green-200', 'bg-yellow-200');
    connectionBadge.classList.add('bg-red-200');
    connectionStatus.classList.remove('text-blue-800', 'text-green-800', 'text-yellow-800');
    connectionStatus.classList.add('text-red-800');
  }
}

// Fetch available networks
async function fetchNetworks() {
  try {
    networkList.innerHTML = '<div class="loading"><i class="fas fa-spinner fa-spin mr-2"></i>Scanning networks...</div>';
    scanBtn.disabled = true;
    const response = await fetch('/scan');
    networks = await response.json();
    displayNetworks(networks);
  } catch (error) {
    console.error('Error scanning networks:', error);
    networkList.innerHTML = '<div class="error">Error scanning networks</div>';
  } finally {
    scanBtn.disabled = false;
  }
}

// Display networks in the UI
function displayNetworks(networks) {
  if (networks.length === 0) {
    networkList.innerHTML = '<div class="no-networks">No networks found</div>';
    return;
  }
  
  // Sort networks by signal strength
  networks.sort((a, b) => b.rssi - a.rssi);
  
  let html = '';
  networks.forEach(network => {
    const signalStrength = getSignalStrengthFromRSSI(network.rssi);
    const isSecure = network.encryptionType !== 0;
    
    html += `
      <div class="network-item" data-ssid="${network.ssid}">
        <div class="network-name">
          ${network.ssid}
          ${isSecure ? '<span class="network-secure">🔒</span>' : ''}
        </div>
        <div class="signal-strength">
          ${signalStrength}dBm
          <div class="signal-bars">
            ${generateSignalBars(network.rssi)}
          </div>
        </div>
      </div>
    `;
  });
  
  networkList.innerHTML = html;
  
  // Add click event to network items
  document.querySelectorAll('.network-item').forEach(item => {
    item.addEventListener('click', () => {
      const ssid = item.getAttribute('data-ssid');
      selectNetwork(ssid);
    });
  });
}

// Select a network from the list
function selectNetwork(ssid) {
  selectedNetwork = networks.find(n => n.ssid === ssid);
  ssidInput.value = ssid;
  passwordInput.focus();
  // Move to credentials step on selection for smoother flow
  goToStep(2);
  
  // Highlight selected network
  document.querySelectorAll('.network-item').forEach(item => {
    if (item.getAttribute('data-ssid') === ssid) {
      item.classList.add('selected');
    } else {
      item.classList.remove('selected');
    }
  });
}

// Connect to the selected network
async function connectToNetwork() {
  const ssid = ssidInput.value.trim();
  const password = passwordInput.value;
  
  if (!ssid) {
    alert('Please select a network or enter an SSID');
    return;
  }
  
  try {
    // Prepare form data (URL-encoded/multipart) expected by server
    const formData = new FormData();
    formData.append('ssid', ssid);
    formData.append('password', password);
    
    // Add any custom parameters
    const existingForm = new FormData(wifiForm);
    for (const [key, value] of existingForm.entries()) {
      if (key !== 'ssid' && key !== 'password') {
        formData.append(key, value);
      }
    }
    
    // Save custom parameters first (non-blocking)
    try { await fetch('/update_params', { method: 'POST', body: existingForm }); } catch (e) { /* ignore */ }

    // Show connecting status
    connectionStatus.textContent = 'Connecting...';
    connectionStatus.style.color = 'var(--warning-color)';
    
    // Send connection request
    const response = await fetch('/connect', {
      method: 'POST',
      body: formData
    });
    
    const result = await response.json();
    if (result.result === 'Connected') {
      showToast('Connected to ' + ssid, 'success');
      // poll to reflect new IP and RSSI
      for (let i=0;i<5;i++){ await sleep(800); await fetchStatus(); }
    } else {
      showToast('Failed to connect to ' + ssid, 'error');
      connectionStatus.textContent = 'Connection Failed';
      connectionStatus.style.color = 'var(--danger-color)';
    }
  } catch (error) {
    console.error('Error connecting to network:', error);
    showToast('Error connecting to network', 'error');
    connectionStatus.textContent = 'Error';
    connectionStatus.style.color = 'var(--danger-color)';
  }
}

// Reset WiFi settings
async function resetSettings() {
  if (confirm('Are you sure you want to reset all WiFi settings?')) {
    try {
      await fetch('/reset');
      showToast('Settings reset successfully', 'success');
      setTimeout(() => window.location.reload(), 800);
    } catch (error) {
      console.error('Error resetting settings:', error);
      showToast('Error resetting settings', 'error');
    }
  }
}

// Fetch custom parameters
async function fetchCustomParams() {
  try {
    const response = await fetch('/params_json');
    const params = await response.json();
    
    if (params.length > 0) {
      let html = '<h3 class="text-slate-200 font-medium mb-1">Additional Settings</h3>';
      
      params.forEach(param => {
        // Get parameter type if available, default to text
        const paramType = param.type || 'text';
        
        html += `<div class="form-group">`;
        
        // Add label for most input types (except when it should come after)
        if (paramType !== 'checkbox' && paramType !== 'radio') {
          html += `<label for="${param.id}">${param.label}:</label>`;
        }
        
        // Generate the appropriate input based on type
        switch(paramType) {
          case 'password':
            html += `<input type="password" id="${param.id}" name="${param.id}" value="${param.value}" ${param.attributes || ''}>`;
            break;
            
          case 'number':
            html += `<input type="number" id="${param.id}" name="${param.id}" value="${param.value}" ${param.attributes || ''}>`;
            break;
            
          case 'checkbox':
            html += `<input type="checkbox" id="${param.id}" name="${param.id}" ${param.value === 'true' ? 'checked' : ''} ${param.attributes || ''}>`;
            html += `<label for="${param.id}">${param.label}</label>`;
            break;
            
          case 'range':
            html += `<input type="range" id="${param.id}" name="${param.id}" value="${param.value}" ${param.attributes || ''}>`;
            break;
            
          case 'select':
            html += `<select id="${param.id}" name="${param.id}" ${param.attributes || ''}>${param.options || ''}</select>`;
            break;
            
          case 'email':
            html += `<input type="email" id="${param.id}" name="${param.id}" value="${param.value}" ${param.attributes || ''}>`;
            break;
            
          case 'url':
            html += `<input type="url" id="${param.id}" name="${param.id}" value="${param.value}" ${param.attributes || ''}>`;
            break;
            
          case 'search':
            html += `<input type="search" id="${param.id}" name="${param.id}" value="${param.value}" ${param.attributes || ''}>`;
            break;
            
          case 'tel':
            html += `<input type="tel" id="${param.id}" name="${param.id}" value="${param.value}" ${param.attributes || ''}>`;
            break;
            
          case 'date':
            html += `<input type="date" id="${param.id}" name="${param.id}" value="${param.value}" ${param.attributes || ''}>`;
            break;
            
          case 'time':
            html += `<input type="time" id="${param.id}" name="${param.id}" value="${param.value}" ${param.attributes || ''}>`;
            break;
            
          case 'datetime-local':
            html += `<input type="datetime-local" id="${param.id}" name="${param.id}" value="${param.value}" ${param.attributes || ''}>`;
            break;
            
          case 'month':
            html += `<input type="month" id="${param.id}" name="${param.id}" value="${param.value}" ${param.attributes || ''}>`;
            break;
            
          case 'week':
            html += `<input type="week" id="${param.id}" name="${param.id}" value="${param.value}" ${param.attributes || ''}>`;
            break;
            
          case 'color':
            html += `<input type="color" id="${param.id}" name="${param.id}" value="${param.value}" ${param.attributes || ''}>`;
            break;
            
          case 'file':
            html += `<input type="file" id="${param.id}" name="${param.id}" ${param.attributes || ''}>`;
            break;
            
          case 'hidden':
            html += `<input type="hidden" id="${param.id}" name="${param.id}" value="${param.value}" ${param.attributes || ''}>`;
            break;
            
          case 'textarea':
            html += `<textarea id="${param.id}" name="${param.id}" ${param.attributes || ''}>${param.value}</textarea>`;
            break;
            
          default: // text
            html += `<input type="text" id="${param.id}" name="${param.id}" value="${param.value}" ${param.attributes || ''}>`;
            break;
        }
        
        html += `</div>`;
      });
      
      customParams.innerHTML = html;
    }
  } catch (error) {
    console.error('Error fetching custom parameters:', error);
  }
}

// Helper function to generate signal strength bars
function generateSignalBars(rssi) {
  // RSSI typically ranges from -100 (weak) to -30 (strong)
  let strength = 0;
  
  if (rssi >= -55) strength = 4;
  else if (rssi >= -65) strength = 3;
  else if (rssi >= -75) strength = 2;
  else if (rssi >= -85) strength = 1;
  else strength = 0;
  
  let bars = '';
  for (let i = 0; i < 5; i++) {
    if (i < strength) {
      bars += `<span class="signal-${i}"></span>`;
    } else {
      bars += `<span class="signal-${i}" style="opacity: 0.3"></span>`;
    }
  }
  
  return bars;
}

// Helper function to get signal strength text from RSSI
function getSignalStrengthFromRSSI(rssi) {
  return rssi;
}

// Periodically update status
setInterval(fetchStatus, 10000); // Update every 10 seconds

// Stepper helpers
function goToStep(step) {
  currentStep = Math.max(1, Math.min(3, step));
  Object.entries(stepPanes).forEach(([k, el]) => {
    if (!el) return;
    if (parseInt(k, 10) === currentStep) el.classList.remove('hidden');
    else el.classList.add('hidden');
  });
  if (stepperNav) {
    stepperNav.querySelectorAll('.step-pill').forEach(btn => {
      btn.classList.toggle('active', parseInt(btn.dataset.step,10) === currentStep);
    });
  }
}
