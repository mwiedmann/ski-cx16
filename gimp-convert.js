const fs = require("fs");

const filebase = process.argv[2];
const imageName = `${filebase}.data`
const palName = `${filebase}.data.pal`
const palOutputFilename = `${filebase.toUpperCase()}.PAL`
const imageOutputFilename = `${filebase.toUpperCase()}.BIN`

console.log(
  `imageName: ${imageName} palName ${palName}`
);

const palData = fs.readFileSync(palName);
const imageData = fs.readFileSync(imageName);

const finalPal = []

const adjustColor = (c) => c>>4

// The raw data is just a long array of R,G and B bytes
// Convert them to G+B and R (4 bytes each)
for (let i = 0; i < palData.length; i+=3) {
    finalPal.push((adjustColor(palData[i+1])<<4) + adjustColor(palData[i+2]))
    finalPal.push(adjustColor(palData[i]))
}

const flattenedTiles = [];
const frameWidth = 16
const frameHeight = 16
const xTiles = 12
const yTiles = 12

let ty, tx, y, x, start, pixelIdx;

for (ty = 0; ty < yTiles; ty++) {
  for (tx = 0; tx < xTiles; tx++) {
    for (y = 0; y < frameHeight; y++) {
      start =
        ty * xTiles * frameWidth * frameHeight +
        tx * frameWidth +
        y * xTiles * frameWidth;
      for (x = 0; x < frameWidth; x++) {
        pixelIdx = start + x;
        flattenedTiles.push(imageData[pixelIdx]);
      }
    }
  }
}

let output = new Uint8Array(finalPal);
fs.writeFileSync(palOutputFilename, output, "binary");

const emptyTile = Array(256).fill(0)
output = new Uint8Array([...emptyTile, ...flattenedTiles]);
fs.writeFileSync(imageOutputFilename, output, "binary");

console.log(`Generated files ${palOutputFilename} ${imageOutputFilename}`);
