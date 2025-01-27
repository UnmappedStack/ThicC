all: build run

build:
	cc src/* -o thicc -I include -g

run:
	./thicc
