from pulp import *
import os,random,math,pickle,zlib

c = ''.join(open("pubkey").read().split("\n")[1:-1]).decode("base64")
# c is a graph.
g = pickle.loads(zlib.decompress(c))
# g is now a graph
g = g['pub']
N = len(g)
vs = []
us = []
prob = LpProblem("myProblem", LpMinimize)
for x in xrange(N):
  vs.append(LpVariable("v"+str(x), 0, 1))
for x in xrange(N):
  # we know its neighbors.
  l = g[x]
  prob += (lpSum([vs[z] for z in l] + [vs[x]]) >= 1)
prob += lpSum(vs)
print "Problem set up. Solving..."
status = prob.solve(GLPK(msg=0))
print "Solved."
lst = []
for x in xrange(N):
  if value(vs[x]) > 0:
    lst.append(x)
print lst
ct = open("ciphertext").read()
ct = pickle.loads(zlib.decompress(ct.decode("base64")))
msg = reduce(lambda a,b:a+b, [ct[i] for i in lst])
print hex(int(msg)).strip('L').strip('0x').decode('hex')
