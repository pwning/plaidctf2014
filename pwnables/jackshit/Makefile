all: jackshit testing

NS := $(shell test `uname -r | cut -d '.' -f 1,2 | tr -d '.'` -ge 313 || echo "-DNEEDS_SOCKETCALL=1")

jackshit: main.c
	gcc -m32 -Os $^ -o $@ -lseccomp -fno-stack-protector ${NS}
	strip $@

testing: main.c
	gcc -m32 -Os $^ -o $@ -lseccomp -fno-stack-protector -DLOCAL_TESTING=1 ${NS}
	strip $@

clean:
	rm -f jackshit testing
