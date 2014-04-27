all: ezhp

ezhp: ezhp.c
	gcc -m32 -z execstack -std=c99 -w $^ -o $@
	strip $@

.PHONY: clean
clean:
	rm ezhp

