CC=cl65
EMU=../x16emur46/x16emu

make:
	$(CC) --cpu 65C02 -Or -Cl -C cx16.cfg -o ./build/SKI.PRG -t cx16 \
	src/main.c src/wait.c src/utils.c src/config.c src/course.c src/sprites.c src/scores.c src/sound.c src/joy.c src/zsmkit.lib

run:
	cd build && \
	$(EMU) -prg SKI.PRG -run

ldtk:
	node tools/ldtk-convert.js

gimp:
	node tools/gimp-convert.js tilemap

score:
	node tools/makescores.js

zip:
	cd build && \
	rm ski.zip && \
	zip ski.zip *

all:
	make ldtk
	make gimp
	make score
	make
	make zip
