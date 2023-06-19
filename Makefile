.PHONY: build run clean

all: build

build: bmp

run: build
	-Wall ./bmp

clean:
	rm -f ./bmp

bmp: bmp.c
	gcc -g -o bmp bmp.c