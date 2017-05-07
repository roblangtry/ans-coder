EXECUTABLE_NAME = ans-coder
CFLAGS = -Wall -pedantic -O3

build:
	gcc $(CFLAGS) src/*.c -lm -o $(EXECUTABLE_NAME)

build_debug:
	gcc $(CFLAGS) -g src/*.c -lm -o $(EXECUTABLE_NAME)

default:
	build