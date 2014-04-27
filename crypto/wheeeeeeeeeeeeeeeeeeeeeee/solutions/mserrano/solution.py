#!/usr/bin/python
import socket
import math
from hashlib import sha512
import array,binascii
import collections
from hashlib import sha512,sha1
import base64 as b64
import random,sys,struct
import SocketServer,os
from time import time
import itertools
import string
from subprocess import Popen, PIPE

HOST = '54.82.75.29'
PORT = 8193
G_enc = '85b3e329c9825ce21133ea8afc232f9eb575fbfe9479900c89b682b28e6d8c73c9ff0042b27766d5e2de33ea8a95037ae50048701ec5225a9360d9163ba61f4747d828a1c420b0692b426f'.decode('hex')

M = 12
N = M * 2
K = N
numrounds = 2 ** 24

def genTable(seed="Function shamelessly stolen from bagre"):
  fSub = {}
  fInvSub = {}
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
    fInvSub[toDo] = x / 2
  return fSub, fInvSub

f, fInv = genTable()
f2, fInv2 = genTable("Good thing I didn't also steal the seed!")

def F(s, k):
  return f[s ^ k]

def F2(s, k):
  return f2[s^k]

def FInv(s, k):
  return fInv[s] ^ k
def F2Inv(s, k):
  return fInv2[s] ^k

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

def decrypt_block(ciphertext, key):
  txt = ciphertext
  l, r = (txt >> M) & ((1 << M) - 1), txt & ((1 << M) - 1)
  for x in xrange(numrounds):
    y = numrounds - x - 1
    if y % 2 == 0:
      r0 = l
      l0 = r ^ F(l, get_key(key, y))
      l, r = l0, r0
    else:
      l0 = l
      r0 = F2Inv(l ^ r, get_key(key, y))
      l, r = l0, r0
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

# Whee ECB
def encrypt(plaintext, key):
  blocks = get_blocks(plaintext)
  out = [encrypt_block(block, key) for block in blocks]
  return unblocks(out)
def decrypt(ciphertext, key):
  blocks = get_blocks(ciphertext)
  ars = [str(len(blocks))] + [str(block) for block in blocks]
  process = Popen(["./pctf2014/problems/crypto/wheeeeeeeeeeeeeeeeeeeeeee/solutions/mserrano/csol", str(key[0]), str(key[1])], stdin=PIPE, stdout=PIPE)
  (output, _) = process.communicate("\n".join(ars) + "\n")
  decrypted = output[:-1].decode('hex')
  return decrypted

def find_proof(proof):
  for i in itertools.product(string.lowercase+string.uppercase, repeat=5):
    sh = sha1()
    real_proof = proof + "".join(i)
    sh.update(real_proof)
    if sh.digest()[-3:] == "\xff\xff\xff":
      return real_proof

def encrypt_pool(plaintexts):
  global G_enc
  count = len(plaintexts)
  size = (count * N) / 4
  if size > 2048:
    print "danger..."
  data = unblocks(plaintexts)
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  s.connect((HOST, PORT))
  f = s.makefile('rw', bufsize=0)
  f.readline()[:-1]
  ln = f.readline()[:-1]
  proof = ln.split(' ')[6][:-1]
  proof = find_proof(proof)
  print "Found proof."
  f.write(proof + '\n')
  f.readline()[:-1]
  G_enc = f.readline()[:-1]
  print "Got new G_enc:", G_enc
  f.readline()[:-1]
  f.write(data.encode('hex') + '\n')
  received = f.readline()[:-1]
  received = received.decode('hex')
  ciphertexts = get_blocks(received)
  f.close()
  s.close()
  return zip(plaintexts, ciphertexts)

def do_slide_attack():
  # Okay.
  # Slide attack time.
  # Our block size is N.
  print "Block size", N
  print "Feistel, so we need 2", (N / 4)+1, "bit pools."
  print "Compared to a", K, "bit bruteforce."
  print "Our total runtime will be O(2^" + str(N/2+2 + K/2) + ") compared to O(2^" + str(K + math.log(numrounds, 2)) + ") for bruteforce."

  # Okay, so first things first. We need an n/2 bit prefix.
  L = N / 2
  print "So, we need a", L, "bit prefix/postfix."
  prefix = (1 << L) - 1
  print "Our prefix will be", hex(prefix)
  print "But it doesn't actually matter what it is."

  def gen_substr():
    return random.randint(0, 2**L - 1)
  def gen_pool(pre):
    if pre:
      return [(prefix << L) | gen_substr() for _ in xrange(2 ** (L/2+1))]
    return [(gen_substr() << L) | prefix for _ in xrange(2 ** (L / 2+1))]
  pre_pool = gen_pool(True)
  post_pool = gen_pool(False)
  print "Okay, we have our plaintext pools."
  print "Now go get ciphertexts."
  print "Expecting to encrypt", len(pre_pool)*2, "blocks."
  st = time()
  pre_pool = encrypt_pool(pre_pool)
  print "Got first pool.", (time() - st)
  post_pool = encrypt_pool(post_pool)
  print "Got ciphertexts."
  print post_pool
  print "Time taken to get ciphertexts", (time() - st)
  print "Okay, now we need to be clever."

  mask = (1 << M) - 1
  def is_slid_pair((a,c0), (b, c1)):
    # assume a = two rounds(b)
    # then: lA = rB. rA =
    l0, r0 = (a >> M) & mask, a & mask
    l2, r2 = (b >> M) & mask, b & mask
    for k0 in xrange(0, 2**(K/2)-1):
      l1 = r0
      r1 = l0 ^ F(r0, k0)
      # Try k0
      mystery = l2 ^ r2
      # Supposedly we now have f[r1 ^ k1].
      k1 = F2Inv(mystery, r1)
      # l0, r0 = (c0 >> L) & mask, c0 & mask
      l0p, r0p = (c0 >> L) & mask, c0 & mask
      l1, r1 = r0p, r0p ^ F2(l0p ^ F(r0p,k0), k1)
      if (l1 << L) | r1 == c1:
        print "Found a candidate..."
        print "flag would be", (k0, k1)
        if encrypt_block(a,(k0,k1)) == c0:
          return [(k0, k1)]
    return []
  print "starting..."
  st = time()
  z = 0
  for a in post_pool:
    print "Outer iteration", z, time() - st
    z = z + 1
    for b in pre_pool:
      k = is_slid_pair(a,b)
      if len(k) > 0:
        print "Found a slid pair!"
        ky = k[0]
        print "Resulting key", ky
        print decrypt(G_enc.decode('hex'), ky)
        return True
  return False

do_slide_attack()
