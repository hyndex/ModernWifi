// ModernWifi Manager - Main JavaScript

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

// State
let networks = [];
let selectedNetwork = null;

// Initialize
document.addEventListener('DOMContentLoaded', () => {
  fetchStatus();
  fetchNetworks();
  fetchCustomParams();
  setupEventListeners();
});

// Setup Event Listeners
function setupEventListeners() {
  // Scan button
  scanBtn.addEventListener('click', fetchNetworks);
  
  // Reset button
  resetBtn.addEventListener('click', resetSettings);
  
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
}

// Fetch current connection status
async function fetchStatus() {
  try {
    const response = await fetch('/status_json');
    const data = await response.json();
    
    connectionStatus.textContent = data.status;
    connectionStatusDetail.textContent = data.status;
    ipAddress.textContent = data.ip;
    
    // Update UI based on connection status
    if (data.status === 'Connected') {
      // Update connection badge
      connectionBadge.classList.remove('bg-blue-200', 'bg-red-200', 'bg-yellow-200');
      connectionBadge.classList.add('bg-green-200');
      connectionStatus.classList.remove('text-blue-800', 'text-red-800', 'text-yellow-800');
      connectionStatus.classList.add('text-green-800');
      
      // Update signal strength if available
      if (WiFi && WiFi.RSSI) {
        const rssi = WiFi.RSSI();
        signalStrength.textContent = rssi + ' dBm';
        signalBars.innerHTML = generateSignalBars(rssi);
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
    networkList.innerHTML = '<div class="loading">Scanning networks...</div>';
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
    // Collect custom parameters
    const formData = new FormData(wifiForm);
    const data = {
      ssid: ssid,
      password: password
    };
    
    // Add any custom parameters
    for (const [key, value] of formData.entries()) {
      if (key !== 'ssid' && key !== 'password') {
        data[key] = value;
      }
    }
    
    // Show connecting status
    connectionStatus.textContent = 'Connecting...';
    connectionStatus.style.color = 'var(--warning-color)';
    
    // Send connection request
    const response = await fetch('/connect', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify(data)
    });
    
    const result = await response.json();
    
    if (result.result === 'Connected') {
      // Connection successful
      alert('Successfully connected to ' + ssid);
      // Refresh status after a short delay
      setTimeout(fetchStatus, 2000);
    } else {
      // Connection failed
      alert('Failed to connect to ' + ssid);
      connectionStatus.textContent = 'Connection Failed';
      connectionStatus.style.color = 'var(--danger-color)';
    }
  } catch (error) {
    console.error('Error connecting to network:', error);
    alert('Error connecting to network');
    connectionStatus.textContent = 'Error';
    connectionStatus.style.color = 'var(--danger-color)';
  }
}

// Reset WiFi settings
async function resetSettings() {
  if (confirm('Are you sure you want to reset all WiFi settings?')) {
    try {
      await fetch('/reset');
      alert('Settings reset successfully');
      // Refresh the page
      window.location.reload();
    } catch (error) {
      console.error('Error resetting settings:', error);
      alert('Error resetting settings');
    }
  }
}

// Fetch custom parameters
async function fetchCustomParams() {
  try {
    const response = await fetch('/params_json');
    const params = await response.json();
    
    if (params.length > 0) {
      let html = '<h3>Additional Settings</h3>';
      
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

// Periodically update status
setInterval(fetchStatus, 10000); // Update every 10 seconds