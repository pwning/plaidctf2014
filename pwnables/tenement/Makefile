all: tenement

tenement: tenement.c
	gcc -z execstack -m32 -Wall -lseccomp -ljansson -Os $^ -o $@
	strip $@

exploit: tenement
	python ./exploit.py | ./tenement config.json

netsploit:
	python ./exploit.py | nc localhost 9999

seccomp_test:
	python ./seccomp_test.py | ./tenement config.json

clean:
	rm tenement

install: tenement
	sudo ./setup.sh
