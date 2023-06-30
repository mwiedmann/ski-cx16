/*
Read the .ldtk file and turn it into a series of .bin files (one per level)
The bin file is formatted for the C data structs so they can be malloc'd and loaded in
*/
const fs = require("fs");

const rawText = fs.readFileSync("ski.ldtk");
const d = JSON.parse(rawText);

const tileWidth = 40
const tileMax = 64

const createLevelCode = (levelNum, level) => {

  const addDummyBytes = (bytes) => {
    for (let i=40; i<64; i++) {
        bytes.push(0);
        bytes.push(0);
    }
  }

  // { "px": [48,32], "src": [80,80], "f": 0, "t": 65, "d": [83] },
  const addTiles = (gridTiles, override0) => {
    const bytes = []
    let col = 0;
    gridTiles.forEach((g) => {
        bytes.push(g.t === override0 ? 0 : g.t + 1); // Add 1 because we have a blank tile before the loaded tiles

        const flip = ((g.f === 2 || g.f === 3 ? 1 : 0)<<3) + ((g.f === 1 || g.f === 3 ? 1 : 0)<<2)
        bytes.push(flip); 
        col++
    
        if (col == tileWidth) {
            addDummyBytes(bytes)
            col=0
        }
    });

    return bytes
  };

  const addFlags = (gridTiles) => {
    const bytes = []
    gridTiles.forEach((g) => {
        bytes.push(g.t + 1) // Add 1 because we have a blank tile before the loaded tiles
        bytes.push(parseInt(g.px[0] / 16))
        bytes.push(parseInt(g.px[1] / 16))
    });

    return bytes
  };

  level.layerInstances.forEach((li) => {
    let bytes = undefined
    switch (li.__identifier) {
      case "Layer_0":
        bytes = addTiles(li.gridTiles);
        break;
      case "Layer_1":
        bytes = addTiles(li.gridTiles, 0);
        break;
      case  "Flags":
        bytes = addFlags(li.gridTiles)
    }

    if (bytes) {
        let outputFilename
        let output

        if (li.__identifier === "Flags") {
            outputFilename = `C${levelNum}FLAGS.BIN`
            output = new Uint8Array([li.gridTiles.length,...bytes])
        } else {
            const layerNum = li.__identifier.split('_')[1]
            outputFilename = `C${levelNum}L${layerNum}.BIN`
            output = new Uint8Array(bytes)
        }
        fs.writeFileSync(outputFilename, output, "binary");
    }
  });
};

d.levels.forEach((l) => {
  const levelNum = l.identifier.split("_")[1];
  createLevelCode(levelNum, l);
});