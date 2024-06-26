/************/
/* ELEMENTS */
/************/

const systemBattery = document.getElementById('systemBattery');
const systemSocket = document.getElementById('systemSocket');

const terminalContainer = document.getElementById('terminalContainer');
const terminalConsole = document.getElementById('terminalConsole');
const terminalInput = document.getElementById('terminalInput');

const cmdResetWifi = document.getElementById('cmdResetWifi');
const cmdResetModule = document.getElementById('cmdResetModule');
const cmdRestart = document.getElementById('cmdRestart');
const cmdHelp = document.getElementById('cmdHelp');

const wifiScanner = document.getElementById('wifiScanner');
const wifiScannerLoader = document.getElementById('wifiScannerLoader');
const wifiNetwork = document.getElementById('wifiNetwork');
const wifiNetworkLoader = document.getElementById('wifiNetworkLoader');

const touchKeys = document.getElementById('touchKeys');
const touchCommon = document.getElementById('touchCommon');

const sensorAnemometerVelocity = document.getElementById('sensorAnemometerVelocity');
const sensorHydreonStatus = document.getElementById('sensorHydreonStatus');
const sensorHydreonIntensity = document.getElementById('sensorHydreonIntensity');

const input01 = document.getElementById('input01');
const input02 = document.getElementById('input02');
const input03 = document.getElementById('input03');
const input04 = document.getElementById('input04');
const input05 = document.getElementById('input05');
const input06 = document.getElementById('input06');
const input07 = document.getElementById('input07');
const input08 = document.getElementById('input08');
const input09 = document.getElementById('input09');
const input10 = document.getElementById('input10');
const input11 = document.getElementById('input11');
const input12 = document.getElementById('input12');

const output01 = document.getElementById('output01');
const output02 = document.getElementById('output02');
const output03 = document.getElementById('output03');
const output04 = document.getElementById('output04');
const output05 = document.getElementById('output05');
const output06 = document.getElementById('output06');
const output07 = document.getElementById('output07');
const output08 = document.getElementById('output08');
const output09 = document.getElementById('output09');
const output10 = document.getElementById('output10');
const output11 = document.getElementById('output11');
const output12 = document.getElementById('output12');
const output13 = document.getElementById('output13');
const output14 = document.getElementById('output14');
const output15 = document.getElementById('output15');
const output16 = document.getElementById('output16');

/***************/
/** UTILITIES **/
/***************/

/**
 * ESPAsyncWebServer template modulo workaround
 * @param {*} dividend value to divide
 * @param {*} divisor value for division
 * @returns remaining value
 */
function modulo(dividend, divisor) {
  // Division by zero
  if (divisor === 0) return NaN;

  // If the dividend is zero, the result is also zero
  if (dividend === 0) return 0;

  // Handling negative numbers
  var sign = (dividend > 0 && divisor > 0) || (dividend < 0 && divisor < 0) ? 1 : -1;

  // Make both dividend and divisor positive to simplify calculation
  dividend = Math.abs(dividend);
  divisor = Math.abs(divisor);

  // Subtract divisor from dividend until dividend becomes less than divisor
  while (dividend >= divisor) dividend -= divisor;

  // Apply sign to the result
  return sign * dividend;
}

/************/
/** SOCKET **/
/************/

let socket;
let socketMonitor;

function refreshSocketMonitoring() {
  systemSocket.innerHTML = 'Connected';
  clearInterval(socketMonitor);
  // socket timeout monitor
  socketMonitor = setInterval(() => {
    systemSocket.innerHTML = 'Timeout';
    transmitToSocket('');
  }, '%ping%' * 3);
}

function reconnectSocket() {
  systemSocket.innerHTML = 'Reconnect';
  printTerminal('Socket Reconnect');
  connectSocket();
}

function connectSocket() {
  socket = new WebSocket(`ws://%address%%socket%`);
  socket.addEventListener('open', onOpenSocket);
  socket.addEventListener('close', onCloseSocket);
  socket.addEventListener('message', onMessageSocket);
}

function onCloseSocket() {
  systemSocket.innerHTML = 'Disconnected';
  printTerminal('Socket Closed');
  clearInterval(socketMonitor);
  setTimeout(reconnectSocket, 2000);
}

function onOpenSocket() {
  systemSocket.innerHTML = 'Connected';
  printTerminal('Socket Open');
  refreshSocketMonitoring();
}

function onMessageSocket(event) {
  socketReceive(JSON.parse(event.data));
}

function socketReceive(message) {
  const data = message.data;
  const id = message.id;

  switch (id) {
    case 'update':
      data.forEach((e) => socketReceive(e));
      break;

    case 'ping':
      refreshSocketMonitoring(data);
      break;

    case 'terminal':
      terminalUpdate(data);
      break;

    case 'wifi':
      wifiUpdate(data);
      break;

    case 'system':
      systemUpdate(data);
      break;

    case 'sensor':
      sensorUpdate(data);
      break;

    case 'output':
      outputUpdate(data);
      break;
  }
}

/**************/
/** TRANSMIT **/
/**************/

function transmitToSocket(id, data) {
  if (socket !== undefined) {
    if (socket.readyState === WebSocket.OPEN) {
      socket.send(JSON.stringify({ id, data }));
    }
  }
}

function transmitButtonEvent(event) {
  const buttonState = (event) => {
    switch (event.type) {
      case 'mousedown':
        if (/Android|iPhone/i.test(navigator.userAgent)) break;
      case 'touchstart':
      case 'click':
        return true;
      case 'mouseup':
      case 'mouseout':
        if (/Android|iPhone/i.test(navigator.userAgent)) break;
      case 'touchend':
        return false;
    }
  };

  transmitToSocket('button', {
    name: event.target.name,
    state: buttonState(event),
  });
}

/************/
/** SYSTEM **/
/************/

function systemUpdate(data) {
  const voltage = data.batteryVoltage;
  if (voltage && systemBattery.innerHTML != voltage) {
    systemBattery.innerHTML = voltage;
  }
}

/**************/
/** TERMINAL **/
/**************/

terminalInput.addEventListener('keyup', terminalEvent);

function terminalUpdate(data) {
  data.text && printToTerminal(data.text);
  data.text && printToConsole(data.text);
}

function printTerminal(text) {
  printToTerminal('>>', text);
  printToConsole('>>', text);
}

function printToConsole(...data) {
  console.log(...data);
}

function printToTerminal(...data) {
  data.forEach((p, i, a) => (a[i] += ' '));
  terminalConsole.value && terminalConsole.append('\n');
  terminalConsole.append(...data);
  terminalConsole.scrollTop = terminalConsole.scrollHeight;
}

function terminalEvent(event) {
  switch (event.key) {
    case 'Enter':
      const text = event.target.value;
      transmitToSocket('terminal', { text });
      printTerminal(text);
      event.target.value = '';
      break;
  }
}

const terminalObserver = new MutationObserver(function () {
  if (terminalContainer.style.display != 'none') {
    terminalConsole.scrollTop = terminalConsole.scrollHeight;
  }
});

terminalObserver.observe(terminalContainer, {
  attributes: true,
  childList: true,
});

/*********/
/** CMD **/
/*********/

cmdResetWifi.addEventListener('click', transmitButtonEvent);
cmdResetModule.addEventListener('click', transmitButtonEvent);
cmdRestart.addEventListener('click', transmitButtonEvent);
cmdHelp.addEventListener('click', transmitButtonEvent);

/**********/
/** WIFI **/
/**********/

wifiScanner.addEventListener('click', transmitButtonEvent);
wifiScanner.addEventListener('click', wifiScannerShowLoader);

function wifiScannerShowLoader() {
  // display loader element
  wifiScannerLoader.style.display = 'block';
  wifiScannerLoader.style.opacity = 1;
  wifiScanner.disabled = true;
  wifiNetwork.style.display = 'none';

  // clear network container
  while (wifiNetwork.firstChild) {
    wifiNetwork.removeChild(wifiNetwork.lastChild);
  }

  // scroll to bottom on mobile
  if (/Android|iPhone/i.test(navigator.userAgent)) {
    window.scrollTo(0, document.body.scrollHeight);
    wifiNetwork.setAttribute('offset', window.scrollY);
  }

  // animation and timeout handler
  const loaderTimeout = 10000;
  const animationIntervall = 100;
  const animationTime = 500;
  const startDate = new Date();
  const startTime = startDate.getTime();
  const scannerLoaderInterval = setInterval(() => {
    const intervalDate = new Date();
    const intervalTime = intervalDate.getTime();
    const elapsedTime = intervalTime - startTime;

    // simple loading animiation
    if (modulo(elapsedTime, animationTime * 2) >= animationTime) {
      wifiScannerLoader.style.opacity = modulo(elapsedTime, animationTime) * 0.001 + 0.5;
    } else {
      wifiScannerLoader.style.opacity = 1 - modulo(elapsedTime, animationTime) * 0.001;
    }

    // reload page on timeout
    if (elapsedTime > loaderTimeout) {
      location.reload();
    }

    // clear intervall if network data received
    if (wifiNetwork.childElementCount) {
      clearInterval(scannerLoaderInterval);
    }
  }, animationIntervall);
}

function wifiUpdate(data) {
  // hide wifi loader
  wifiScanner.disabled = false;
  wifiScannerLoader.style.display = 'none';

  // clear network container
  while (wifiNetwork.firstChild) {
    wifiNetwork.removeChild(wifiNetwork.lastChild);
  }

  // new network data available
  if (data.network.length) {
    // password field
    const input = document.createElement('input');
    input.classList.add('wifi-input');
    input.style.display = 'none';
    input.type = 'password';
    input.placeholder = 'PASSWORD';

    // submit field
    const submit = document.createElement('button');
    submit.classList.add('wifi-input');
    submit.style.display = 'none';
    submit.innerHTML = 'SUBMIT';

    // update wifi network event
    const updateWifiNetwork = () => {
      wifiNetworkLoader.style.display = 'block';
      wifiNetworkLoader.style.opacity = 1;

      // animation and timeout handler
      const loaderTimeout = 10000;
      const animationIntervall = 100;
      const animationTime = 500;
      const startDate = new Date();
      const startTime = startDate.getTime();
      setInterval(() => {
        const intervalDate = new Date();
        const intervalTime = intervalDate.getTime();
        const elapsedTime = intervalTime - startTime;

        // simple loading animiation
        if (modulo(elapsedTime, animationTime * 2) >= animationTime) {
          wifiNetworkLoader.style.opacity = modulo(elapsedTime, animationTime) * 0.001 + 0.5;
        } else {
          wifiNetworkLoader.style.opacity = 1 - modulo(elapsedTime, animationTime) * 0.001;
        }

        // reload page on timeout
        if (elapsedTime > loaderTimeout) {
          location.reload();
        }
      }, animationIntervall);

      // get selected network ssid
      const ssid = [...wifiNetwork.childNodes].find((e) => {
        if (e.classList.contains('wifi-selected')) return e;
      });

      // transmit to socket
      transmitToSocket('wifi', {
        ssid: ssid && ssid.innerHTML,
        password: input.value,
      });
    };

    // add update network event to elements
    submit.addEventListener('click', updateWifiNetwork);
    input.addEventListener('keyup', (e) => e.key === 'Enter' && updateWifiNetwork());

    // select wifi network event
    const selectWifiNetwork = (event) => {
      wifiNetwork.childNodes.forEach((e) => e.classList.remove('wifi-selected'));
      event.target.classList.add('wifi-selected');
      input.style.display = 'block';
      submit.style.display = 'block';

      // scroll to bottom on mobile
      if (/Android|iPhone/i.test(navigator.userAgent)) {
        window.scrollTo(0, document.body.scrollHeight);
      }
    };

    // add network data
    data.network.forEach((value) => {
      // add network ssid
      const ssid = document.createElement('button');
      ssid.innerHTML = value.SSID;
      ssid.name = 'ssid';
      ssid.classList.add('wifi-variable');
      ssid.addEventListener('click', selectWifiNetwork);

      // add network rssi
      const rssi = document.createElement('div');
      rssi.innerHTML = value.RSSI;
      rssi.name = 'rssi';
      rssi.classList.add('wifi-value');

      wifiNetwork.appendChild(ssid);
      wifiNetwork.appendChild(rssi);
    });

    // add elements to network container
    wifiNetwork.appendChild(input);
    wifiNetwork.appendChild(submit);
    wifiNetwork.style.display = 'grid';

    // scroll to bottom on mobile
    if (/Android|iPhone/i.test(navigator.userAgent)) {
      const offset = wifiNetwork.getAttribute('offset');
      if (offset && offset == window.scrollY) {
        window.scrollTo(0, document.body.scrollHeight);
      }
    }
  }
}

/***********/
/** TOUCH **/
/***********/

function touchChangeCommon(event) {
  const text = event.innerHTML;
  const setting = event.nextElementSibling.getAttribute('name');
  const value = prompt('change ' + text);
  value && transmitToSocket('touch', { id: 'common', setting, value });
}

function touchChangeKey(event) {
  const text = event.innerHTML;
  const key = event.parentElement.getAttribute('value');
  const setting = event.nextElementSibling.getAttribute('name');
  const value = prompt('change key ' + key + ' ' + text);
  value && transmitToSocket('touch', { id: 'key', setting, value, key });
}

function touchUpdate(data) {
  const touchCommonChildren = touchCommon.children;
  // get touch common values
  for (const child of touchCommonChildren) {
    try {
      const name = child.getAttribute('name');
      const value = data.common[name];
      // update touch common value
      if (value && value != child.innerHTML) child.innerHTML = value;
    } catch (e) {
      continue;
    }
  }

  // get touch key elements
  const touchKeyChildren = touchKeys.children;
  for (let i = 0; i < touchKeyChildren.length; i++) {
    const keyId = touchKeyChildren[i].getAttribute('value');
    const keyChildren = touchKeyChildren[i].children;
    // get touch key values
    for (const child of keyChildren) {
      try {
        const name = child.getAttribute('name');
        const value = data.key[keyId][name];
        // update touch key value
        if (value && value != child.innerHTML) child.innerHTML = value;
      } catch (e) {
        continue;
      }
    }
  }
}

/************/
/** SENSOR **/
/************/

function sensorUpdate(data) {
  if (data.anemometerVelocity != undefined) {
    sensorAnemometerVelocity.innerHTML = data.anemometerVelocity;
  }

  if (data.hydreonStatus != undefined) {
    sensorHydreonStatus.innerHTML = data.hydreonStatus;
  }
}

/*********/
/* INPUT */
/*********/

input01.addEventListener('mousedown', transmitButtonEvent);
input01.addEventListener('mouseup', transmitButtonEvent);
input01.addEventListener('mouseout', transmitButtonEvent);
input01.addEventListener('touchstart', transmitButtonEvent);
input01.addEventListener('touchend', transmitButtonEvent);

input02.addEventListener('mousedown', transmitButtonEvent);
input02.addEventListener('mouseup', transmitButtonEvent);
input02.addEventListener('mouseout', transmitButtonEvent);
input02.addEventListener('touchstart', transmitButtonEvent);
input02.addEventListener('touchend', transmitButtonEvent);

input03.addEventListener('mousedown', transmitButtonEvent);
input03.addEventListener('mouseup', transmitButtonEvent);
input03.addEventListener('mouseout', transmitButtonEvent);
input03.addEventListener('touchstart', transmitButtonEvent);
input03.addEventListener('touchend', transmitButtonEvent);

input04.addEventListener('mousedown', transmitButtonEvent);
input04.addEventListener('mouseup', transmitButtonEvent);
input04.addEventListener('mouseout', transmitButtonEvent);
input04.addEventListener('touchstart', transmitButtonEvent);
input04.addEventListener('touchend', transmitButtonEvent);

input05.addEventListener('mousedown', transmitButtonEvent);
input05.addEventListener('mouseup', transmitButtonEvent);
input05.addEventListener('mouseout', transmitButtonEvent);
input05.addEventListener('touchstart', transmitButtonEvent);
input05.addEventListener('touchend', transmitButtonEvent);

input06.addEventListener('mousedown', transmitButtonEvent);
input06.addEventListener('mouseup', transmitButtonEvent);
input06.addEventListener('mouseout', transmitButtonEvent);
input06.addEventListener('touchstart', transmitButtonEvent);
input06.addEventListener('touchend', transmitButtonEvent);

input07.addEventListener('mousedown', transmitButtonEvent);
input07.addEventListener('mouseup', transmitButtonEvent);
input07.addEventListener('mouseout', transmitButtonEvent);
input07.addEventListener('touchstart', transmitButtonEvent);
input07.addEventListener('touchend', transmitButtonEvent);

input08.addEventListener('mousedown', transmitButtonEvent);
input08.addEventListener('mouseup', transmitButtonEvent);
input08.addEventListener('mouseout', transmitButtonEvent);
input08.addEventListener('touchstart', transmitButtonEvent);
input08.addEventListener('touchend', transmitButtonEvent);

input09.addEventListener('mousedown', transmitButtonEvent);
input09.addEventListener('mouseup', transmitButtonEvent);
input09.addEventListener('mouseout', transmitButtonEvent);
input09.addEventListener('touchstart', transmitButtonEvent);
input09.addEventListener('touchend', transmitButtonEvent);

input10.addEventListener('mousedown', transmitButtonEvent);
input10.addEventListener('mouseup', transmitButtonEvent);
input10.addEventListener('mouseout', transmitButtonEvent);
input10.addEventListener('touchstart', transmitButtonEvent);
input10.addEventListener('touchend', transmitButtonEvent);

input11.addEventListener('mousedown', transmitButtonEvent);
input11.addEventListener('mouseup', transmitButtonEvent);
input11.addEventListener('mouseout', transmitButtonEvent);
input11.addEventListener('touchstart', transmitButtonEvent);
input11.addEventListener('touchend', transmitButtonEvent);

input12.addEventListener('mousedown', transmitButtonEvent);
input12.addEventListener('mouseup', transmitButtonEvent);
input12.addEventListener('mouseout', transmitButtonEvent);
input12.addEventListener('touchstart', transmitButtonEvent);
input12.addEventListener('touchend', transmitButtonEvent);

/**********/
/* OUTPUT */
/**********/

function outputUpdate(data) {
  if (data.output01 != undefined && data.output01) output01.classList.add('output-active');
  if (data.output01 != undefined && !data.output01) output01.classList.remove('output-active');
  if (data.output02 != undefined && data.output02) output02.classList.add('output-active');
  if (data.output02 != undefined && !data.output02) output02.classList.remove('output-active');
  if (data.output03 != undefined && data.output03) output03.classList.add('output-active');
  if (data.output03 != undefined && !data.output03) output03.classList.remove('output-active');
  if (data.output04 != undefined && data.output04) output04.classList.add('output-active');
  if (data.output04 != undefined && !data.output04) output04.classList.remove('output-active');
  if (data.output05 != undefined && data.output05) output05.classList.add('output-active');
  if (data.output05 != undefined && !data.output05) output05.classList.remove('output-active');
  if (data.output06 != undefined && data.output06) output06.classList.add('output-active');
  if (data.output06 != undefined && !data.output06) output06.classList.remove('output-active');
  if (data.output07 != undefined && data.output07) output07.classList.add('output-active');
  if (data.output07 != undefined && !data.output07) output07.classList.remove('output-active');
  if (data.output08 != undefined && data.output08) output08.classList.add('output-active');
  if (data.output08 != undefined && !data.output08) output08.classList.remove('output-active');
  if (data.output09 != undefined && data.output09) output09.classList.add('output-active');
  if (data.output09 != undefined && !data.output09) output09.classList.remove('output-active');
  if (data.output10 != undefined && data.output10) output10.classList.add('output-active');
  if (data.output10 != undefined && !data.output10) output10.classList.remove('output-active');
  if (data.output11 != undefined && data.output11) output11.classList.add('output-active');
  if (data.output11 != undefined && !data.output11) output11.classList.remove('output-active');
  if (data.output12 != undefined && data.output12) output12.classList.add('output-active');
  if (data.output12 != undefined && !data.output12) output12.classList.remove('output-active');
  if (data.output13 != undefined && data.output13) output13.classList.add('output-active');
  if (data.output13 != undefined && !data.output13) output13.classList.remove('output-active');
  if (data.output14 != undefined && data.output14) output14.classList.add('output-active');
  if (data.output14 != undefined && !data.output14) output14.classList.remove('output-active');
  if (data.output15 != undefined && data.output15) output15.classList.add('output-active');
  if (data.output15 != undefined && !data.output15) output15.classList.remove('output-active');
  if (data.output16 != undefined && data.output16) output16.classList.add('output-active');
  if (data.output16 != undefined && !data.output16) output16.classList.remove('output-active');
}

/**********/
/** MAIN **/
/**********/

try {
  connectSocket();
} catch (e) {
  // for dev environment
}
