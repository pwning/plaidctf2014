all: kappa

kappa: kappa.c
	gcc -m32 -std=c99 -w $^ -o $@
	strip $@

.PHONY: clean
clean:
	rm kappa

