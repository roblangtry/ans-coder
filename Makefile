EXECUTABLE_NAME = ans-coder
CFLAGS = -Wall -pedantic -O3

build:
	gcc $(CFLAGS) src/*.c -lm -o $(EXECUTABLE_NAME)

build_debug:
	gcc $(CFLAGS) -g src/*.c -lm -o $(EXECUTABLE_NAME)

build_prelude_test: clean_prelude_test prelude_test

clean_prelude_test:
	rm PRELUDE_TEST

prelude_test:
	gcc $(CFLAGS) src/writer.c src/reader.c src/prelude_code.c src/prelude_test.c -lm -o PRELUDE_TEST

default:
	build