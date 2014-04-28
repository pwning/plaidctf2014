#!/usr/bin/python -u
'''
You may wish to refer to solutions to the pCTF 2013 "pyjail" problem if
you choose to attempt this problem, BUT IT WON'T HELP HAHAHA.
'''

from imp import acquire_lock
from threading import Thread
from sys import modules, stdin, stdout

# No more importing!
x = Thread(target = acquire_lock, args = ())
x.start()
x.join()
del x
del acquire_lock
del Thread

# No more modules!
for k, v in modules.iteritems():
	if v == None: continue
	if k == '__main__': continue
	v.__dict__.clear()

del k, v

__main__ = modules['__main__']
modules.clear()
del modules

# No more anything!
del __builtins__, __doc__, __file__, __name__, __package__

print >> stdout, "Get a shell. The flag is NOT in ./key, ./flag, etc."
while 1:
	exec 'print >> stdout, ' + stdin.readline() in {'stdout':stdout}
