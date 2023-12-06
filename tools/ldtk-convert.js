/*
Read the .ldtk file and turn it into a series of .bin files (one per level)
The bin file is formatted for the C data structs so they can be malloc'd and loaded in
*/
const fs = require("fs");

const rawText = fs.readFileSync("gfx/ski.ldtk");
const d = JSON.parse(rawText);

const tileWidth = 40;
const tileMax = 64;

const createLevelCode = (levelNum, level) => {
  const addDummyBytes = (bytes) => {
    for (let i = 40; i < 64; i++) {
      bytes.push(0);
      bytes.push(0);
    }
  };

  // { "px": [48,32], "src": [80,80], "f": 0, "t": 65, "d": [83] },
  const addTiles = (gridTiles, override0) => {
    const bytes = [];
    let col = 0;
    gridTiles.forEach((g) => {
      bytes.push(g.t === override0 ? 0 : g.t + 1); // Add 1 because we have a blank tile before the loaded tiles

      const flip =
        ((g.f === 2 || g.f === 3 ? 1 : 0) << 3) +
        ((g.f === 1 || g.f === 3 ? 1 : 0) << 2);
      bytes.push(flip);
      col++;

      if (col == tileWidth) {
        addDummyBytes(bytes);
        col = 0;
      }
    });

    return bytes;
  };

  const addFlags = (gridTiles) => {
    const bytes = [];
    gridTiles.forEach((g) => {
      bytes.push(parseInt(g.px[1] / 16)); // row
      bytes.push(g.t + 1); // tile1 - Add 1 because we have a blank tile before the loaded tiles
      bytes.push(parseInt(g.px[0] / 16)); // col1
      bytes.push(0); // tile2
      bytes.push(0); // col2
    });

    return bytes;
  };

  const addGates = (gridTiles) => {
    const bytes = [];
    let length = 0;

    for (let i = 0; i < gridTiles.length; i++) {
      length++;
      const g = gridTiles[i];
      bytes.push(parseInt(g.px[1] / 16)); // row
      bytes.push(g.t + 1); // tile1 - Add 1 because we have a blank tile before the loaded tiles
      bytes.push(parseInt(g.px[0] / 16)); // col1
      // For gate flags, there should alwyas be a 2nd flag for the gate
      if (g.t == 8 || g.t == 9) {
        const g2 = gridTiles[i + 1];
        bytes.push(g2.t + 1); // tile2
        bytes.push(parseInt(g2.px[0] / 16)); // col2
        i++;
      } else {
        // This is not a gate flag, just add 0
        bytes.push(0); // tile2
        bytes.push(0); // col2
      }
    }

    return { bytes, length };
  };

  level.layerInstances.forEach((li) => {
    let bytes = undefined;
    let results

    switch (li.__identifier) {
      case "Layer_0":
        bytes = addTiles(li.gridTiles);
        break;
      case "Layer_1":
        bytes = addTiles(li.gridTiles, 0);
        break;
      case "Flags":
        // "Levels" 16-17 don't have flags/gates
        if (levelNum >= 16) {
          return
        }
        bytes = addFlags(li.gridTiles);
        break;
      case "Gates":
        // "Levels" 16-17 don't have flags/gates
        if (levelNum >= 16) {
          return
        }
        results = addGates(li.gridTiles);
        break;
    }

    if (bytes || results.bytes) {
      let outputFilename;
      let output;

      if (li.__identifier === "Flags") {
        outputFilename = `C${levelNum}FLAGS.BIN`;
        output = new Uint8Array([li.gridTiles.length, ...bytes]);
      } else if (li.__identifier === "Gates") {
        outputFilename = `C${levelNum}GATES.BIN`;
        output = new Uint8Array([results.length, ...results.bytes]);
      } else {
        const layerNum = li.__identifier.split("_")[1];
        outputFilename = `C${levelNum}L${layerNum}.BIN`;
        output = new Uint8Array(bytes);
      }
      fs.writeFileSync(`build/${outputFilename}`, output, "binary");
    }
  });
};

d.levels.forEach((l) => {
  const levelNum = l.identifier.split("_")[1];
  createLevelCode(levelNum, l);
});
