from os import environ
from struct import unpack

def readint(s):
	return unpack('<I', s)[0]

def readlong(s):
	return unpack('<Q', s)[0]

sizeoff = 0x1a0c0
ptrsoff = 0x1a0e0
rodiff = 0x400000

pt = open(environ['HOME']+"/pctf2014/problems/reversing/pt/pt").read()

childbufs = []
for i in range(7):
	siz = readint(pt[sizeoff+i*4:sizeoff+i*4+4])
	off = readlong(pt[ptrsoff+i*8:ptrsoff+i*8+8]) - rodiff
	childbufs.append([readint(pt[off+4*j:off+4*j+4]) for j in range(siz)])

def stepidx_(buf, idx, c):
	return (buf[(idx << 8) + idx + ord(c) + 2])

def checkidx_(buf, idx):
	return (buf[idx + (idx << 8) + 1])

def stepidx(buf, idx, c):
	return BV2Int(buf[(idx * 2**8) + idx + BV2Int(c) + 2])

def checkidx(buf, idx):
	return BV2Int(buf[idx + (idx * 2**8) + 1])


endstates = []
for buf in childbufs:
	try:
		for i in range(999):
			if checkidx_(buf, i):
				endstates.append(i)
		assert False
	except IndexError:
		pass

endstates[1] = 33 #bug!
assert len(endstates) == 7

import graphAlgos


def stepall(idxs, c):
	try:
		return tuple(stepidx_(buf, i, c) for i,buf in zip(idxs, childbufs))
	except IndexError:
		return None

def neighs(v):
	ret = []
	for c in map(chr, range(32,127)):
		x = stepall(v, c)
		if x:
			ret.append(x)
	return ret

q = graphAlgos.dijkstra(neighs, (tuple([0]*7),), (tuple(endstates),), dist = lambda x,y: 1)
# q = graphAlgos.bfs(neighs, (tuple([0]*7),), lambda path: path if path[0] == endstates else None)
# q = graphAlgos.beamSearch(neighs, (tuple([0]*7),), (tuple(endstates),), 999, lambda x: 1)
print q

s = ''
for i in range(1,len(q)):
	for c in map(chr, range(32,127)):
		if stepall(q[i-1], c) == q[i]:
			s += c
			break
print s

exit(0)

from z3 import *
from z3util import *

solv = Solver()

for i in range(7):
	st = FakeString('child'+str(i), BitVecSort(32))
	solv.add(len(st) == len(childbufs[i]))
	for j in range(len(childbufs[i])):
		solv.add(st[j] == childbufs[i][j])
	childbufs[i] = st

flag = FakeString('flag')

solv.add(flag.isascii())

# solv.add(len(flag) <= 32)
# solv.add(0 < len(flag))
# for buf in childbufs:
# 	f = Function(buf.name+'steps', IntSort(), IntSort())
# 	solv.add(f(-1) == 0)
# 	x = Int('x')
# 	solv.add(ForAllXInRange(0, len(flag), lambda x: f(x) == stepidx(buf, f(x-1), flag[x])))
# 	solv.add(checkidx(buf, f(len(flag))) != 0)

length = 1
solv.add(len(flag) == length)
for buf in childbufs:
	prevstep = 0
	for i in range(length):
		prevstep = stepidx(buf, prevstep, flag[i])
	thingy = checkidx(buf, prevstep)
	solv.add(thingy != 0)


for c in childbufs:
	solv.add(c.constraint)
solv.add(flag.constraint)

print "Solving"
print solv.check()
print solv.model()
print flag.stringInModel(solv.model())
