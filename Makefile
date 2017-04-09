EXECUTABLE_NAME = ans-coder
CFLAGS = -Wall -pedantic -O3

build:
	gcc $(CFLAGS) src/*.c -o $(EXECUTABLE_NAME)

default:
	build