CC=cl65

make:
	$(CC) -O -o SKI.PRG -t cx16 main.c wait.c utils.c config.c course.c sprites.c

run:
	x16emur43/x16emu -prg SKI.PRG -run

ldtk:
	node ldtk-convert.js

gimp:
	node gimp-convert.js tilemap

zip:
	zip ski.zip SKI.PRG C0L0.BIN C0L1.BIN C0FLAGS.BIN SKITILES.BIN SKITILES.PAL manifest.json

all:
	make ldtk
	make gimp
	make
