from os import environ
from struct import unpack

sizeoff = 0x1a0c0
ptrsoff = 0x1a0e0
rodiff = 0x400000

pt = open(environ['HOME']+"/pctf2014/problems/reversing/pt/pt").read()

def readint(s):
	return unpack('<I', s)[0]

def readlong(s):
	return unpack('<Q', s)[0]

childbufs = []
for i in range(7):
	siz = readint(pt[sizeoff+i*4:sizeoff+i*4+4])
	off = readlong(pt[ptrsoff+i*8:ptrsoff+i*8+8]) - rodiff
	childbufs.append([readint(pt[off+4*j:off+4*j+4]) for j in range(siz)])

def stepidx_(buf, idx, c):
	return (buf[(idx << 8) + idx + ord(c) + 2])

def checkidx_(buf, idx):
	return (buf[idx + (idx << 8) + 1])

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

q = __import__('graphAlgos').dijkstra(neighs, (tuple([0]*7),), (tuple(endstates),), dist = lambda x,y: 1)

s = ''
for i in range(1,len(q)):
	for c in map(chr, range(32,127)):
		if stepall(q[i-1], c) == q[i]:
			s += c
			break
print s
