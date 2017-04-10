EXECUTABLE_NAME = ans-coder
CFLAGS = -Wall -pedantic -O3

build:
	gcc $(CFLAGS) src/*.c -o $(EXECUTABLE_NAME)

build_debug:
	gcc $(CFLAGS) -g src/*.c -o $(EXECUTABLE_NAME)

default:
	build