from hashlib import sha512,sha1
import random,sys,struct
import SocketServer
import base64 as b64
import os
import hmac
from time import time
import math
from hashlib import sha512
import array,binascii
import collections
from hashlib import sha512,sha1
import base64 as b64
import random,sys,struct
import SocketServer,os
from time import time
from subprocess import Popen, PIPE
import threading

KEY = [2633, 3247]# [0, 0] # Not the real key, obviously...
FLAGE = "Gotta love it when you can SLIDE. The flage is id_almost_rather_be_sledding"
ENC_FLAGE = "85b3e329c9825ce21133ea8afc232f9eb575fbfe9479900c89b682b28e6d8c73c9ff0042b27766d5e2de33ea8a95037ae50048701ec5225a9360d9163ba61f4747d828a1c420b0692b426f"

M = 12
N = M * 2
K = N
numrounds = 2 ** 24 # Protip: would not bruteforce this if I were you.

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
def encrypt(plaintext):
  blocks = get_blocks(plaintext)
  ars = [str(len(blocks))] + [str(block) for block in blocks]
  process = Popen(["./cimpl", str(KEY[0]), str(KEY[1])], stdin=PIPE, stdout=PIPE)
  (output, _) = process.communicate("\n".join(ars) + "\n")
  output = map(int, output.split('\n')[:-1])
  return unblocks(output)
def decrypt(ciphertext):
  blocks = get_blocks(ciphertext)
  out = [decrypt_block(block, KEY) for block in blocks]
  return unblocks(out)

class HandleCryptoService(SocketServer.BaseRequestHandler):
  def doPOW(self):
    req=self.request
    proof = b64.b64encode(os.urandom(12))
    req.sendall("We would first like some proof of work.\n")
    req.sendall("Send us a string starting with %s, of length %s, such that its sha1 sum ends in ffffff\n"% (proof, str(len(proof)+5)))
    test = req.recv(21)
    req.recv(1)
    ha = sha1()
    ha.update(test)
    if (test[0:16] != proof or ha.digest()[-3:] != "\xff\xff\xff"):
      req.sendall("Invalid proof of work!\n")
      req.close()
      return False
    return True

  def handle(self):
    req = self.request
    if (not (self.doPOW())):
      req.close()
      return
    req.sendall("WHHEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n")
    req.sendall(ENC_FLAGE + '\n')
    req.sendall("Send your encryption string: \n")
    data = req.recv(2049)[:-1].decode('hex')
    resp = encrypt(data)
    req.sendall(resp.encode('hex') + '\n')
    req.close()

class ThreadedServer(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
  pass

def checkKey():
  global ENC_FLAGE
  print "checking key..."
  valid = True
  process = Popen(["./check", str(KEY[0]), str(KEY[1])], stdin=PIPE, stdout=PIPE)
  blocks = get_blocks(FLAGE)
  try:
    (output, _) = process.communicate("\n".join([str(len(blocks))] + [str(x) for x in blocks]))
  except:
    valid = False
  if valid:
    ENC_FLAGE = output.strip('\n')
  return valid

def updateKey():
  global KEY
  print "Changing key..."
  print "Old key:", KEY
  KEY = [random.randint(0, (1 << (K >> 1)) - 1), random.randint(0, (1 << (K >> 1)) -1)]
  while not checkKey():
    KEY = [random.randint(0, (1 << (K >> 1)) - 1), random.randint(0, (1 << (K >> 1)) -1)]
  print "New key:", KEY
  print "New ENC_FLAGE:", ENC_FLAGE
  threading.Timer(10 * 60, updateKey, []).start()

if __name__ == "__main__":
  HOST, PORT = "", 0 
  threading.Timer(1, updateKey, []).start()
  ThreadedServer.allow_reuse_address = True
  server = ThreadedServer((HOST, PORT), HandleCryptoService)
  server.allow_reuse_address = True
  server.serve_forever()

