emu: emulator.h ../src/* ../src/assets/* ../src/ui/* ../lib/gigagl/gigagl.c
	gcc -o emu \
		../src/main.c \
		../src/assets/*.c \
		../src/calibrate.c \
		../lib/gigagl/gigagl.c ../src/ui/*.c \
		-I../include/ -I../lib/gigagl/ -I. \
		-lraylib -lm -lpthread \
		-DEMULATOR -ggdb -O0

run: emu
	./emu