.PHONY: all run clean
all: sass
clean:
	rm sass
sass: sass.c
	cc -std=c99 -m32 -pie -fPIE -fno-stack-protector -O2 -o $@ $^
run: sass
	./sass
