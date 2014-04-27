all: lazy

lazy: lazy.c
	gcc -m32 -O3 -std=c99 -w $^ -o $@
	strip $@

.PHONY: clean
clean:
	rm lazy

