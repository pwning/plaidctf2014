from hashlib import sha512,sha1
import random

KEY = [0, 0]

M = 12
N = M * 2
K = N
numrounds = 2 ** 24 # Protip: would not bruteforce this if I were you.

def genTable(seed="Function shamelessly stolen from bagre"):
  fSub = {}
  i = 0
  prng = sha512()
  prng.update(seed)
  seed = prng.digest()
  cSeed = ""
  for x in xrange(2048):
    cSeed+=prng.digest()
    prng.update(str(x)+prng.digest())
  fCharSub = [0]*(2**M)
  gCharSub = [0]*(2**M)
  unused = range(2**M)
  for x in xrange(0,2**(M+1),2):
    curInd = (ord(cSeed[x]) + (ord(cSeed[x + 1]) << 8)) % len(unused)
    toDo = unused[curInd]
    del unused[curInd]
    fSub[x / 2] = toDo
  return fSub

f = genTable()
f2 = genTable("Good thing I didn't also steal the seed!")

def gen_key():
  k0 = random.randint(0,2**(K/2)-1)
  k1 = random.randint(0,2**(K/2)-1)
  return [k0, k1]

def F(s, k):
  return f[s ^ k]

def F2(s, k):
  return f2[s^k]

def get_key(key, n):
  return key[n & 1]

def encrypt_block(plaintext, key):
  txt = plaintext
  l, r = (txt >> M) & ((1 << M) - 1), txt & ((1 << M) - 1)
  for x in xrange(numrounds):
    if x % 2 == 0:
      l1 = r
      r1 = l ^ F(r, key[0])
      l, r = l1, r1
    else:
      l1 = l
      r1 = l ^ F2(r, key[1])
      l, r = l1, r1
  return l << M | r

def extract(s):
  c = 0
  for x in s:
    c = (c << 8) | ord(x)
  return c

def intract(n):
  s = []
  while n > 0:
    s.append(chr(n & 0xff))
    n = n >> 8
  return ''.join(s[::-1])

def get_blocks(txt):
  n = N / 8
  if len(txt) % n != 0:
    txt += '\x00' * (n - len(txt) % n)
  block_strs = [txt[i*n:i*n+n] for i in range(len(txt) / n)]
  return [extract(s) for s in block_strs]

def unblocks(l):
  z = [intract(x) for x in l]
  s = ''.join(z)
  s = s.strip('\x00')
  return s

def encrypt(plaintext):
  blocks = get_blocks(plaintext)
  out = [encrypt_block(block, KEY) for block in blocks]
  return unblocks(out)


