CC=cl65

make:
	$(CC) -O -o SKI.PRG -t cx16 main.c wait.c utils.c config.c course.c sprites.c scores.c

run:
	x16emur43/x16emu -prg SKI.PRG -run

ldtk:
	node ldtk-convert.js

gimp:
	node gimp-convert.js tilemap

score:
	node makescores.js

zip:
	zip ski.zip SKI.PRG C0L0.BIN C0L1.BIN C0FLAGS.BIN C1L0.BIN C1L1.BIN C1FLAGS.BIN C15L0.BIN C15L1.BIN C15FLAGS.BIN TILEMAP.BIN TILEMAP.PAL manifest.json

all:
	make ldtk
	make gimp
	make
	make zip
