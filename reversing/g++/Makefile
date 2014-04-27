KEY:=guess_a_key
.PHONY: all key.h

all: solveme.cpp key.h
	g++ -std=c++0x solveme.cpp -ftemplate-depth-1000000 -o solveme

key.h: 
	echo '${KEY}' | perl -p -e 's/(.)/sprintf("K(%d,%d)\n",$$i++,ord($$1))/ge' > key.h
