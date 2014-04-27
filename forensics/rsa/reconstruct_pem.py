#!/usr/bin/env python
import gmpy
import math
from itertools import product


#get the base64 out, replace corrupted bytes with base64 value
def fixup(b64, pad):
  ret = ''.join(b64.split("\n")[1:-2]).replace(" ",pad)
  ret = ret.ljust(((len(ret)-1)/4+1)*4,'=')
  return ret

def s2bin(s):
  return bin(int(s.encode("hex"),16))[2:]

'''
#get known offsets from a real pem file
# FIXME: this is dangerous, some places shift around if the leading byte is 0. Maybe make
#  a few of these or something and try multiple times... since this one may be wrong...
dat = s2bin(''.join(open("poop").read().split("\n")[1:-2]).decode("base64"))
rsak = RSA.importKey(open("poop").read())
d = {}
#ugly, but meh, I'm lazy
d['n'] = bin(rsak.n)[2:].strip("L")
d['e'] = bin(rsak.e)[2:].strip("L")
d['q'] = bin(rsak.q)[2:].strip("L")
d['p'] = bin(rsak.p)[2:].strip("L")
d['d'] = bin(rsak.d)[2:].strip("L")
d['c'] = bin(libnum.invmod(rsak.q,rsak.p))[2:].strip("L")
d['dq'] = bin(rsak.d % (rsak.q - 1))[2:].strip("L")
d['dp'] = bin(rsak.d % (rsak.p - 1))[2:].strip("L")


inp = open("corrupted2").read()

s1 = s2bin(fixup(inp,'A').decode("base64"))
s2 = s2bin(fixup(inp,'/').decode("base64"))
'''

inp = open("corrupted.pem").read().split("\n")
cn = ''.join([x[4:].replace(":","") for x in inp[2:11]])
cd = ''.join([x[4:].replace(":","") for x in inp[13:22]])
cp = ''.join([x[4:].replace(":","") for x in inp[23:28]])
cq = ''.join([x[4:].replace(":","") for x in inp[29:34]])
cpd = ''.join([x[4:].replace(":","") for x in inp[35:40]])
cqd = ''.join([x[4:].replace(":","") for x in inp[41:46]])


'''
known = [a if (a == b) else '2' for (a,b) in zip(s1,s2)]

#figure out where in our pem file things live
mask = [" "]*(len(s1)+100)
for v in ['n','e','p','q','d','c','dq','dp']:
  for i in range(dat.index(d[v]),dat.index(d[v])+len(d[v])):
    mask[i] = v

#representation of what we know about our missing key bits...
print ''.join([(colored(v[-1],'green' if (b != '2') else 'red')) if (v != ' ') else ' ' for (v,b) in zip(mask,known)])


pd = int(''.join([str(int(b)&1) if (v == 'd') else '' for (v,b) in zip(mask,known)]),2)|1 #these are odd...
kpd = int(''.join([str(1-(int(b)>>1)) if (v == 'd') else '' for (v,b) in zip(mask,known)]),2)|1

pq = int(''.join([str(int(b)&1) if (v == 'q') else '' for (v,b) in zip(mask,known)]),2)|1
kpq = int(''.join([str(1-(int(b)>>1)) if (v == 'q') else '' for (v,b) in zip(mask,known)]),2)|1

pp = int(''.join([str(int(b)&1) if (v == 'p') else '' for (v,b) in zip(mask,known)]),2)|1
kpp = int(''.join([str(1-(int(b)>>1)) if (v == 'p') else '' for (v,b) in zip(mask,known)]),2)|1

pdp = int(''.join([str(int(b)&1) if (v == 'dp') else '' for (v,b) in zip(mask,known)]),2)
kpdp = int(''.join([str(1-(int(b)>>1)) if (v == 'dp') else '' for (v,b) in zip(mask,known)]),2)|1

pdq = int(''.join([str(int(b)&1) if (v == 'dq') else '' for (v,b) in zip(mask,known)]),2)
kpdq = int(''.join([str(1-(int(b)>>1)) if (v == 'dq') else '' for (v,b) in zip(mask,known)]),2)|1
'''
pd = int(cd.replace(" ","0"),16)|1
kpd = int(cd.replace(" ","0"),16)^int(cd.replace(" ","F"),16)^int("F"*len(cd),16)|1

pp = int(cp.replace(" ","0"),16)|1
kpp = int(cp.replace(" ","0"),16)^int(cp.replace(" ","F"),16)^int("F"*len(cp),16)|1

pq = int(cq.replace(" ","0"),16)|1
kpq = int(cq.replace(" ","0"),16)^int(cq.replace(" ","F"),16)^int("F"*len(cq),16)|1

pdp = int(cpd.replace(" ","0"),16)|1
kpdp = int(cpd.replace(" ","0"),16)^int(cpd.replace(" ","F"),16)^int("F"*len(cpd),16)|1

pdq = int(cqd.replace(" ","0"),16)|1
kpdq = int(cqd.replace(" ","0"),16)^int(cqd.replace(" ","F"),16)^int("F"*len(cqd),16)|1



# we are given N and e..
N = int("dbfabdb1495d3276e7626b84796e9fc20fa13c1744f10c8c3f3e3c2c6040c2e7f313dfa3d1fe10d1ae577cfeab7452aa53102eef7be0099c022560e57a5c30d50940642d1b097dd2109ae02f2dcff8198cd5a395fcac4266107848b9dd63c387d2538e50415343042033ea09c084155e652b0f062340d5d4717a402a9d806a6b",16)
e = int("010001",16)


# start adding in stuff from http://eprint.iacr.org/2008/510.pdf
def hamming_weight(x):
  return bin(x).count("1")

def dtwiddle(k):
  return ((k*(N+1)+1)/e)


kp_candidates = []
for kp in xrange(e):
  kp_candidates.append(hamming_weight((dtwiddle(kp)^pd)&pd))

k = kp_candidates.index(min(kp_candidates))
del kp_candidates

#we now know the first half of d!
dtwid = dtwiddle(k)
pd = (pd & ((1<<512)-1)) | (dtwid & (((1<<512)-1)<<512) )
kpd |= (((1<<512)-1)<<512)

def kptest(kp):
  return ((kp*kp)%e - ((k*(N-1)+1)%e)*kp - k) %e == 0

def tau(t):
  for i in xrange(t.bit_length()):
    if (t&(1<<i)):
      return i
  return 0

kpps = []
for kpv in xrange(e):
  if kptest(kpv):
    kpps.append(kpv)

#if these don't work, we need to try the other way!
kq = kpps[0]
kp = kpps[1]

dpps = [gmpy.invert(e,2<<(1+tau(kpps[j]))) for j in xrange(2)]

def gb(v,i):
  return (v>>i)&1

def bp(x,i):
  return x&((1<<i)-1)

def sb(x,i,v):
  return (x & ~(1<<i))|(v << i)

#fix e*pdp[q] % (2**(1+tau(kp[q]))) == 1
# FIXME we're lazy and e = 0x10001, so just treat e=1
pdp = (pdp & (~(2**(1+tau(kp)) - 1))) | 1
pdq = (pdq & (~(2**(1+tau(kq)) - 1))) | 1

def calc_slice(vals,i):
  pp,pq,pd,pdp,pdq = vals
  c1 = gb(N-bp(pp,i)*bp(pq,i),i)
  c2 = gb(k*(N+1)+1 - k*(bp(pp,i)+bp(pq,i)) - e*bp(pd,i),i+tau(k))
  c3 = gb(kp*(bp(pp,i)-1)+1 - e*bp(pdp,i),i+tau(kp))
  c4 = gb(kq*(bp(pq,i)-1)+1 - e*bp(pdq,i),i+tau(kq))
  feasible = []
  for (pi,qi,di,dpi,dqi) in product([0,1],repeat=5):
    if (c1 == (pi^qi)) and (c2 == (di^pi^qi)) and (c3 == (dpi^pi)) and (c4 == (dqi^qi)):
      feasible.append((pi,qi,di,dpi,dqi))

  return feasible

def force_slice(bits,vals,i):
  pp,pq,pd,pdp,pdq = vals
  pp = sb(pp,i,bits[0])
  pq = sb(pq,i,bits[1])
  pd = sb(pd,tau(k)+i,bits[2])
  pdp = sb(pdp,tau(kp)+i,bits[3])
  pdq = sb(pdq,tau(kq)+i,bits[4])
  return (pp,pq,pd,pdp,pdq)

def pick_slice(vals,i):
  slices = calc_slice(vals,i)
  pp,pq,pd,pdp,pdq = vals
  best_slices = []
  for s in slices:
    score = 0
    #fixme shouldn't count to score if it is an "unknown" bit
    if gb(pp,i) != s[0]:
      score += gb(kpp,i)
    if gb(pq,i) != s[1]:
      score += gb(kpq,i)
    if gb(pd,i+tau(k)) != s[2]:
      score += gb(kpd,i+tau(k))
    if gb(pdp,i+tau(kp)) != s[3]:
      score += gb(kpdp,i+tau(kp))
    if gb(pdq,i+tau(kq)) != s[4]:
      score += gb(kpdq,i+tau(kq))
    if score == 0:
      best_slices.append(s)
#    else:
#      print s, score
#      best_slices.append(s)
  return best_slices

def actual_slice(i):
  return (gb(pp,i),gb(pq,i),gb(pd,tau(k)+i),gb(pdp,tau(kp)+i),gb(pdq,tau(kq)+i))

def solve(stop=512):
  global pp,pq,pd,pdp,pdq
  c = 1
  backtrack = []
  i = 1
  while i < stop:
    if (i%100 == 0):
      print i, len(backtrack)
#    print i
    vals = (pp,pq,pd,pdp,pdq)
    gg = pick_slice(vals,i)
    choose = 0
    if (len(gg) == 0):
#      print "oops going back (%d)"%(len(backtrack))
      saved,vals = backtrack.pop()
      i = saved
      gg = pick_slice(vals,i)
      choose = 1

    if (choose == 0):
      c*= len(gg)
      if len(gg) > 1:
        backtrack.append((i,vals))
#        print i, len(backtrack)
    pp,pq,pd,pdp,pdq = force_slice(gg[choose],vals,i)
    i += 1
  return c

solve(512)
print pp*pq == N

msg = open("encrypted").read()
msg = msg.encode('hex')
msg = int(msg, 16)
msg = pow(msg, pd, N)
msg = hex(msg).strip('L').split('00')[-1]
print msg.decode('hex')

