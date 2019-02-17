const outputCtx = document.querySelector('#output').getContext('2d');
const nameInput = document.querySelector('#name');

const displayWidth = 296;
const displayHeight = 128;

function readCanvas() {
  const { data } = outputCtx.getImageData(0, 0, displayWidth, displayHeight);
  const bitmap = new Uint8Array((displayWidth * displayHeight) / 8);
  for (let y = 0; y < displayHeight; y++) {
    for (let x = 0; x < displayWidth; x++) {
      const point = data[(y * displayWidth + x) * 4];
      // TODO better algorithm for calculating pixel values?
      if (point > 0) {
        const bitmapOffs = Math.floor(y / 8) * displayWidth + x;
        const bit = 7 - (y % 8);
        bitmap[bitmapOffs] |= 1 << bit;
      }
    }
  }
  return bitmap;
}

outputCtx.fillStyle = 'white';
outputCtx.fillRect(0, 0, displayWidth, displayHeight);

let lastImage = readCanvas();
let displayCharacteristic = null;

let displayUpdating = false;
async function updateDisplay() {
  if (displayUpdating) {
    return;
  }
  displayUpdating = true;
  try {
    let updated = false;
    do {
      updated = false;
      const currentImage = readCanvas();
      for (let i = 0; i < currentImage.length; i++) {
        if (lastImage[i] != currentImage[i]) {
          const data = [i & 0xff, (i >> 8) | 0x80, ...currentImage.slice(i, i + 18)];
          await displayCharacteristic.writeValue(new Uint8Array(data));
          for (let j = 0; j < 18; j++) {
            lastImage[i + j] = currentImage[i + j];
          }
          updated = true;
          break;
        }
      }
    } while (updated);
  } finally {
    displayUpdating = false;
  }
}

function drawName() {
  outputCtx.fillStyle = 'white';
  outputCtx.fillRect(0, 0, displayWidth, displayHeight);
  outputCtx.fillStyle = 'black';
  outputCtx.font = '48px cursive';
  outputCtx.fillText(nameInput.value, 10, 40);
  if (displayCharacteristic) {
    updateDisplay();
  }
}

async function connect() {
  const device = await navigator.bluetooth.requestDevice({
    filters: [{ services: [0xfeef] }]
  });
  await device.gatt.connect();
  const svc = await device.gatt.getPrimaryService(0xfeef);
  displayCharacteristic = await svc.getCharacteristic(0xfeee);
  console.log('Connected :-)');

  // Clear screen to white:
  await displayCharacteristic.writeValue(new Uint8Array([0xff]));
  for (let i = 0; i < lastImage.length; i++) {
    lastImage[i] = 0xff;
  }

  // Draw current photo
  updateDisplay();
}

drawName();
