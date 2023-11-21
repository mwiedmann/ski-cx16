CC=cl65

make:
	$(CC) -O -o SKI.PRG -t cx16 main.c wait.c utils.c config.c course.c sprites.c scores.c

run:
	x16emur46/x16emu -prg SKI.PRG -run

ldtk:
	node ldtk-convert.js

gimp:
	node gimp-convert.js tilemap

score:
	node makescores.js

zip:
	zip ski.zip SKI.PRG \
	C0L0.BIN C0L1.BIN C0FLAGS.BIN \
	C1L0.BIN C1L1.BIN C1FLAGS.BIN \
	C15L0.BIN C15L1.BIN C15FLAGS.BIN \
	SCORE01.BIN SCORE02.BIN SCORE03.BIN SCORE04.BIN \
	SCORE11.BIN SCORE12.BIN SCORE13.BIN SCORE14.BIN \
	TILEMAP.BIN TILEMAP.PAL manifest.json

all:
	make ldtk
	make gimp
	make score
	make
	make zip
