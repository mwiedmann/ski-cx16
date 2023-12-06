const fs = require("fs");

const shortHi = (s) => s>>8
const shortLo = (s) => s&255

const startScore = 14400
const timeBump = 60*15
const names = ['AAA', 'BBB', 'CCC', 'DDD', 'EEE', 'FFF', 'GGG', 'HHH', 'III', 'JJJ']

const vals = names.map((name, idx) => ({score: startScore + (timeBump * idx), name }))

const charBump = 128

const bytes = vals.map(v => ([shortLo(v.score), shortHi(v.score), v.name.charCodeAt(0)+charBump, v.name.charCodeAt(1)+charBump, v.name.charCodeAt(2)+charBump, 0])).flat();

let output = new Uint8Array([0,0,...bytes]);

fs.writeFileSync(`build/SCORE.BIN`, output, "binary");
