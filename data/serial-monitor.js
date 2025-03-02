// ModernWifi Serial Monitor - Terminal JavaScript

// DOM Elements
const terminal = document.getElementById('terminal');
const terminalContent = document.getElementById('terminal-content');
const terminalInput = document.getElementById('terminal-input');
const clearBtn = document.getElementById('clear-btn');
const saveBtn = document.getElementById('save-btn');
const autoscrollBtn = document.getElementById('autoscroll-btn');
const timestampBtn = document.getElementById('timestamp-btn');
const matrixBtn = document.getElementById('matrix-btn');
const baudRateSelector = document.getElementById('baud-rate');
const connectionIndicator = document.getElementById('connection-indicator');
const connectionText = document.getElementById('connection-text');
const commandHistoryDropdown = document.getElementById('command-history-dropdown');
const bootProgress = document.getElementById('boot-progress');
const bootStatus = document.getElementById('boot-status');
const matrixCanvas = document.getElementById('matrix-canvas');

// State
let isConnected = false;
let autoscroll = true;
let showTimestamps = false;
let matrixMode = false;
let commandHistory = [];
let historyIndex = -1;
let currentBaudRate = 115200;
let serialBuffer = '';
let lastDataTime = Date.now();
let reconnectAttempts = 0;
let maxReconnectAttempts = 5;
let reconnectInterval = null;
let matrixRain = null;

// Initialize
document.addEventListener('DOMContentLoaded', () => {
  initializeTerminal();
  setupEventListeners();
  startBootSequence();
  initializeMatrixRain();
});

// Initialize terminal
function initializeTerminal() {
  // Load saved settings from localStorage
  autoscroll = localStorage.getItem('autoscroll') !== 'false';
  showTimestamps = localStorage.getItem('showTimestamps') === 'true';
  matrixMode = localStorage.getItem('matrixMode') === 'true';
  currentBaudRate = parseInt(localStorage.getItem('baudRate')) || 115200;
  
  // Apply settings to UI
  autoscrollBtn.innerHTML = `<i class="fas fa-arrow-down"></i> Autoscroll: ${autoscroll ? 'ON' : 'OFF'}`;
  timestampBtn.innerHTML = `<i class="fas fa-clock"></i> Timestamps: ${showTimestamps ? 'ON' : 'OFF'}`;
  baudRateSelector.value = currentBaudRate.toString();
  
  // Load command history
  try {
    const savedHistory = localStorage.getItem('commandHistory');
    if (savedHistory) {
      commandHistory = JSON.parse(savedHistory);
    }
  } catch (error) {
    console.error('Error loading command history:', error);
    commandHistory = [];
  }
  
  // Apply matrix mode if enabled
  if (matrixMode) {
    toggleMatrixMode(true);
  }
}

// Setup event listeners
function setupEventListeners() {
  // Clear button
  clearBtn.addEventListener('click', clearTerminal);
  
  // Save button
  saveBtn.addEventListener('click', saveTerminalLog);
  
  // Autoscroll button
  autoscrollBtn.addEventListener('click', toggleAutoscroll);
  
  // Timestamp button
  timestampBtn.addEventListener('click', toggleTimestamps);
  
  // Matrix mode button
  matrixBtn.addEventListener('click', () => toggleMatrixMode());
  
  // Baud rate selector
  baudRateSelector.addEventListener('change', changeBaudRate);
  
  // Terminal input
  terminalInput.addEventListener('keydown', handleInputKeydown);
  
  // Connect to WebSocket when page loads
  setTimeout(connectWebSocket, 1500); // Delay to allow boot sequence to show
}

// Boot sequence animation
function startBootSequence() {
  const progressBar = document.getElementById('boot-progress');
  let progress = 0;
  
  const interval = setInterval(() => {
    progress += 5;
    progressBar.style.width = `${progress}%`;
    
    if (progress >= 100) {
      clearInterval(interval);
      bootStatus.textContent = 'Ready';
      bootStatus.classList.add('terminal-success');
      
      // Add some boot messages
      setTimeout(() => {
        addTerminalLine('Serial monitor initialized successfully', 'success');
        addTerminalLine('Type "help" for available commands', 'info');
      }, 500);
    } else if (progress === 30) {
      bootStatus.textContent = 'Initializing buffers...';
    } else if (progress === 60) {
      bootStatus.textContent = 'Configuring baud rate...';
    } else if (progress === 90) {
      bootStatus.textContent = 'Starting connection...';
    }
  }, 50);
}

// Connect to WebSocket for serial data
function connectWebSocket() {
  // Create WebSocket connection
  const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
  const ws = new WebSocket(`${protocol}//${window.location.host}/serial_ws`);
  
  ws.onopen = () => {
    setConnectionStatus(true);
    addTerminalLine('Connected to serial port', 'success');
    
    // Send current baud rate to server
    ws.send(JSON.stringify({ type: 'baudRate', value: currentBaudRate }));
    
    // Reset reconnect attempts
    reconnectAttempts = 0;
    if (reconnectInterval) {
      clearInterval(reconnectInterval);
      reconnectInterval = null;
    }
  };
  
  ws.onmessage = (event) => {
    // Process incoming serial data
    const data = event.data;
    processSerialData(data);
    lastDataTime = Date.now();
  };
  
  ws.onclose = () => {
    setConnectionStatus(false);
    addTerminalLine('Disconnected from serial port', 'error');
    
    // Try to reconnect
    if (reconnectAttempts < maxReconnectAttempts && !reconnectInterval) {
      reconnectInterval = setInterval(() => {
        reconnectAttempts++;
        addTerminalLine(`Reconnect attempt ${reconnectAttempts}/${maxReconnectAttempts}...`, 'warning');
        connectWebSocket();
        
        if (reconnectAttempts >= maxReconnectAttempts) {
          clearInterval(reconnectInterval);
          reconnectInterval = null;
          addTerminalLine('Max reconnect attempts reached. Please refresh the page to try again.', 'error');
        }
      }, 5000);
    }
  };
  
  ws.onerror = (error) => {
    console.error('WebSocket error:', error);
    addTerminalLine('Error connecting to serial port', 'error');
  };
  
  // Store WebSocket reference
  window.serialWs = ws;
}

// Process incoming serial data
function processSerialData(data) {
  // Add data to buffer
  serialBuffer += data;
  
  // Process complete lines
  const lines = serialBuffer.split('\n');
  serialBuffer = lines.pop(); // Keep the last incomplete line in the buffer
  
  // Add complete lines to terminal
  lines.forEach(line => {
    if (line.trim()) {
      addTerminalLine(line);
    }
  });
}

// Add a line to the terminal
function addTerminalLine(text, type = '') {
  const line = document.createElement('div');
  line.className = 'terminal-line';
  
  // Add timestamp if enabled
  if (showTimestamps) {
    const timestamp = document.createElement('span');
    timestamp.className = 'terminal-timestamp';
    timestamp.textContent = new Date().toLocaleTimeString();
    line.appendChild(timestamp);
  }
  
  // Add text content with appropriate styling
  const content = document.createElement('span');
  if (type) {
    content.className = `terminal-${type}`;
  }
  
  // Apply syntax highlighting for code-like content
  if (text.includes('{') && text.includes('}') || 
      text.includes('function') || 
      text.includes('var ') || 
      text.includes('const ') || 
      text.includes('let ') || 
      text.includes('if ') || 
      text.includes('for ')) {
    content.innerHTML = hljs.highlight(text, {language: 'javascript'}).value;
  } else if (text.startsWith('#include') || text.includes('void setup()') || text.includes('int ')) {
    content.innerHTML = hljs.highlight(text, {language: 'cpp'}).value;
  } else {
    content.textContent = text;
  }
  
  line.appendChild(content);
  terminalContent.appendChild(line);
  
  // Auto-scroll if enabled
  if (autoscroll) {
    terminal.scrollTop = terminal.scrollHeight;
  }
  
  // Limit the number of lines to prevent memory issues
  const maxLines = 1000;
  while (terminalContent.childElementCount > maxLines) {
    terminalContent.removeChild(terminalContent.firstChild);
  }
}

// Clear the terminal
function clearTerminal() {
  // Keep only the boot sequence
  const bootSequence = document.querySelector('.boot-sequence');
  terminalContent.innerHTML = '';
  if (bootSequence) {
    terminalContent.appendChild(bootSequence.cloneNode(true));
  }
  addTerminalLine('Terminal cleared', 'info');
}

// Save terminal log to file
function saveTerminalLog() {
  // Get all text content from terminal
  const lines = Array.from(terminalContent.querySelectorAll('.terminal-line'))
    .map(line => {
      const timestamp = line.querySelector('.terminal-timestamp');
      const content = line.textContent.replace(timestamp ? timestamp.textContent : '', '').trim();
      return (timestamp ? `[${timestamp.textContent}] ` : '') + content;
    })
    .join('\n');
  
  // Create blob and download link
  const blob = new Blob([lines], { type: 'text/plain' });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = `serial_log_${new Date().toISOString().replace(/[:.]/g, '-')}.txt`;
  a.click();
  
  // Clean up
  setTimeout(() => URL.revokeObjectURL(url), 100);
  addTerminalLine('Log saved to file', 'success');
}

// Toggle autoscroll
function toggleAutoscroll() {
  autoscroll = !autoscroll;
  autoscrollBtn.innerHTML = `<i class="fas fa-arrow-down"></i> Autoscroll: ${autoscroll ? 'ON' : 'OFF'}`;
  localStorage.setItem('autoscroll', autoscroll);
  
  if (autoscroll) {
    terminal.scrollTop = terminal.scrollHeight;
  }
}

// Toggle timestamps
function toggleTimestamps() {
  showTimestamps = !showTimestamps;
  timestampBtn.innerHTML = `<i class="fas fa-clock"></i> Timestamps: ${showTimestamps ? 'ON' : 'OFF'}`;
  localStorage.setItem('showTimestamps', showTimestamps);
  
  // Add a message to show the change
  addTerminalLine(`Timestamps ${showTimestamps ? 'enabled' : 'disabled'}`, 'info');
}

// Change baud rate
function changeBaudRate() {
  const newBaudRate = parseInt(baudRateSelector.value);
  if (newBaudRate !== currentBaudRate) {
    currentBaudRate = newBaudRate;
    localStorage.setItem('baudRate', currentBaudRate);
    
    // Send baud rate change to server if connected
    if (window.serialWs && window.serialWs.readyState === WebSocket.OPEN) {
      window.serialWs.send(JSON.stringify({ type: 'baudRate', value: currentBaudRate }));
      addTerminalLine(`Baud rate changed to ${currentBaudRate}`, 'info');
    }
  }
}

// Handle input keydown events
function handleInputKeydown(event) {
  // Enter key - send command
  if (event.key === 'Enter') {
    const command = terminalInput.value.trim();
    if (command) {
      // Add command to history
      if (commandHistory.length === 0 || commandHistory[commandHistory.length - 1] !== command) {
        commandHistory.push(command);
        // Limit history size
        if (commandHistory.length > 50) {
          commandHistory.shift();
        }
        localStorage.setItem('commandHistory', JSON.stringify(commandHistory));
      }
      
      // Reset history index
      historyIndex = -1;
      
      // Display command in terminal
      addTerminalLine(`> ${command}`, 'prompt');
      
      // Process command
      processCommand(command);
      
      // Clear input
      terminalInput.value = '';
    }
  }
  // Up arrow - previous command in history
  else if (event.key === 'ArrowUp') {
    event.preventDefault();
    if (commandHistory.length > 0) {
      if (historyIndex < commandHistory.length - 1) {
        historyIndex++;
      }
      terminalInput.value = commandHistory[commandHistory.length - 1 - historyIndex];
    }
  }
  // Down arrow - next command in history
  else if (event.key === 'ArrowDown') {
    event.preventDefault();
    if (historyIndex > 0) {
      historyIndex--;
      terminalInput.value = commandHistory[commandHistory.length - 1 - historyIndex];
    } else if (historyIndex === 0) {
      historyIndex = -1;
      terminalInput.value = '';
    }
  }
  // Tab key - show command history dropdown
  else if (event.key === 'Tab') {
    event.preventDefault();
    toggleCommandHistoryDropdown();
  }
  // Escape key - hide command history dropdown
  else if (event.key === 'Escape') {
    hideCommandHistoryDropdown();
  }
}

// Process commands
function processCommand(command) {
  // Send command to server
  if (window.serialWs && window.serialWs.readyState === WebSocket.OPEN) {
    window.serialWs.send(JSON.stringify({ type: 'command', value: command }));
  }
  
  // Handle built-in commands
  if (command.toLowerCase() === 'help') {
    addTerminalLine('Available commands:', 'info');
    addTerminalLine('  help - Show this help message', 'info');
    addTerminalLine('  clear - Clear the terminal', 'info');
    addTerminalLine('  reset - Reset the connection', 'info');
    addTerminalLine('  baud <rate> - Change baud rate (e.g. baud 115200)', 'info');
    addTerminalLine('  matrix - Toggle matrix effect', 'info');
  } else if (command.toLowerCase() === 'clear') {
    clearTerminal();
  } else if (command.toLowerCase() === 'reset') {
    if (window.serialWs) {
      window.serialWs.close();
      setTimeout(connectWebSocket, 1000);
    }
    addTerminalLine('Resetting connection...', 'warning');
  } else if (command.toLowerCase().startsWith('baud ')) {
    const rate = parseInt(command.split(' ')[1]);
    if (!isNaN(rate)) {
      baudRateSelector.value = rate.toString();
      changeBaudRate();
    } else {
      addTerminalLine('Invalid baud rate', 'error');
    }
  } else if (command.toLowerCase() === 'matrix') {
    toggleMatrixMode();
  }
}

// Set connection status
function setConnectionStatus(connected) {
  isConnected = connected;
  
  if (connected) {
    connectionIndicator.classList.remove('status-disconnected');
    connectionIndicator.classList.add('status-connected');
    connectionText.textContent = 'Connected';
  } else {
    connectionIndicator.classList.remove('status-connected');
    connectionIndicator.classList.add('status-disconnected');
    connectionText.textContent = 'Disconnected';
  }
}

// Toggle command history dropdown
function toggleCommandHistoryDropdown() {
  if (commandHistory.length === 0) return;
  
  const dropdown = document.getElementById('command-history-dropdown');
  
  if (dropdown.style.display === 'block') {
    hideCommandHistoryDropdown();
    return;
  }
  
  // Populate dropdown
  dropdown.innerHTML = '';
  for (let i = commandHistory.length - 1; i >= 0; i--) {
    const item = document.createElement('div');
    item.className = 'command-history-item';
    item.textContent = commandHistory[i];
    item.addEventListener('click', () => {
      terminalInput.value = commandHistory[i];
      hideCommandHistoryDropdown();
      terminalInput.focus();
    });
    dropdown.appendChild(item);
  }
  
  dropdown.style.display = 'block';
}

// Hide command history dropdown
function hideCommandHistoryDropdown() {
  const dropdown = document.getElementById('command-history-dropdown');
  dropdown.style.display = 'none';
}

// Initialize Matrix rain effect
function initializeMatrixRain() {
  const canvas = document.getElementById('matrix-canvas');
  const ctx = canvas.getContext('2d');
  
  // Set canvas dimensions
  canvas.width = terminal.clientWidth;
  canvas.height = terminal.clientHeight;
  
  // Matrix characters
  const chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789$+-*/=%"\'\_&|<>[]{}(),.;:?!@#';
  const fontSize = 14;
  const columns = Math.floor(canvas.width / fontSize);
  
  // Array to track the y position of each column
  const drops = [];
  for (let i = 0; i < columns; i++) {
    drops[i] = Math.floor(Math.random() * -100); // Random starting position above the canvas
  }
  
  // Drawing function
  function draw() {
    // Semi-transparent black to create fade effect
    ctx.fillStyle = 'rgba(0, 0, 0, 0.05)';
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    
    ctx.fillStyle = '#0f0'; // Matrix green
    ctx.font = `${fontSize}px monospace`;
    
    // Loop through each column
    for (let i = 0; i < drops.length; i++) {
      // Get random character
      const char = chars[Math.floor(Math.random() * chars.length)];
      
      // Draw the character
      ctx.fillText(char, i * fontSize, drops[i] * fontSize);
      
      // Move the drop down
      if (drops[i] * fontSize > canvas.height && Math.random() > 0.975) {
        drops[i] = 0;
      }
      drops[i]++;
    }
  }
  
  // Store the animation reference
  matrixRain = {
    interval: null,
    start: function() {
      this.interval = setInterval(draw, 33); // ~30fps
    },
    stop: function() {
      if (this.interval) {
        clearInterval(this.interval);
        this.interval = null;
        
        // Clear the canvas
        ctx.clearRect(0, 0, canvas.width, canvas.height);
      }
    }
  };
  
  // Handle window resize
  window.addEventListener('resize', () => {
    if (matrixMode) {
      matrixRain.stop();
      canvas.width = terminal.clientWidth;
      canvas.height = terminal.clientHeight;
      matrixRain.start();
    }
  });
}

// Toggle matrix mode
function toggleMatrixMode(force) {
  matrixMode = force !== undefined ? force : !matrixMode;
  localStorage.setItem('matrixMode', matrixMode);
  
  if (matrixMode) {
    matrixBtn.innerHTML = '<i class="fas fa-code"></i> Normal Mode';
    matrixRain.start();
    addTerminalLine('Matrix mode activated', 'success');
  } else {
    matrixBtn.innerHTML = '<i class="fas fa-code"></i> Matrix Mode';
    matrixRain.stop();
    addTerminalLine('Matrix mode deactivated', 'info');
  }
}