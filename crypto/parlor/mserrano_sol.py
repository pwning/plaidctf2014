import socket
from random import randint
from subprocess import PIPE,Popen


S11 = 7
S12 = 12
S13 = 17
S14 = 22
S21 = 5
S22 = 9
S23 = 14
S24 = 20
S31 = 4
S32 = 11
S33 = 16
S34 = 23
S41 = 6
S42 = 10
S43 = 15
S44 = 21

PADDING = "\x80" + 63*"\0"   # do not overlook first byte again :-)

# F, G, H and I are basic MD5 functions
def F(x, y, z): return (((x) & (y)) | ((~x) & (z)))

def G(x, y, z): return (((x) & (z)) | ((y) & (~z)))

def H(x, y, z): return ((x) ^ (y) ^ (z))

def I(x, y, z): return((y) ^ ((x) | (~z)))

# ROTATE_LEFT rotates x left n bits.
def ROTATE_LEFT(x, n):
  x = x & 0xffffffffL   # make shift unsigned
  return (((x) << (n)) | ((x) >> (32-(n)))) & 0xffffffffL

# FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
# Rotation is separate from addition to prevent recomputation.
def FF(a, b, c, d, x, s, ac):
  a = a + F ((b), (c), (d)) + (x) + (ac)
  a = ROTATE_LEFT ((a), (s))
  a = a + b
  return a # must assign this to a

def GG(a, b, c, d, x, s, ac):
  a = a + G ((b), (c), (d)) + (x) + (ac)
  a = ROTATE_LEFT ((a), (s))
  a = a + b
  return a # must assign this to a

def HH(a, b, c, d, x, s, ac):
  a = a + H ((b), (c), (d)) + (x) + (ac)
  a = ROTATE_LEFT ((a), (s))
  a = a + b
  return a # must assign this to a

def II(a, b, c, d, x, s, ac):
  a = a + I ((b), (c), (d)) + (x) + (ac)
  a = ROTATE_LEFT ((a), (s))
  a = a + b
  return a # must assign this to a


class md5:
  def __init__(self, initial=None):
    self.count = 0L
    self.state = (0x67452301L,
            0xefcdab89L,
            0x98badcfeL,
            0x10325476L,)
    self.buffer = ""
    if initial:
      self.update(initial)
      

  def update(self, input):
    """
    MD5 block update operation. Continues an MD5 message-digest
    operation, processing another message block, and updating the
    context.
    """
    inputLen = len(input)
    index = int(self.count >> 3) & 0x3F

    # Update number of bits 
    self.count = self.count + (inputLen << 3)
    
    partLen = 64 - index

    # Transform as many times as possible.
    if inputLen >= partLen:
      self.buffer = self.buffer[:index] + input[:partLen]
      self.transform(self.buffer)
      i = partLen
      while i + 63 < inputLen:
        self.transform(input[i:i+64])
        i = i + 64
      index = 0
    else:
      i = 0

    # Buffer remaining input
    self.buffer = self.buffer[:index] + input[i:inputLen]

    
  def final(self):
    """
    MD5 finalization. Ends an MD5 message-digest operation, 
    writing the message digest and zeroizing the context.
    """
    # Save number of bits
    bits = Encode((self.count & 0xffffffffL, self.count>>32), 8)
    
    # Pad out to 56 mod 64
    index = int((self.count >> 3) & 0x3f)

    if index < 56:
      padLen = (56 - index)
    else:
      padLen = (120 - index)
    
    # Append padding
    self.update(PADDING[:padLen])

    # Append bits
    self.update(bits)
    
    # Store state in digest
    digest = Encode(self.state, 16)

    # Zeroize sensitive information
    self.__dict__.clear()
    return digest
  
  digest = final  # alias

  def transform(self, block):
    """ MD5 basic transformation. Transforms state based on block """
    a, b, c, d = state = self.state

    x = Decode(block, 64)

    # Round 1
    a = FF (a, b, c, d, x[ 0], S11, 0xd76aa478)#; /* 1 */
    d = FF (d, a, b, c, x[ 1], S12, 0xe8c7b756)#; /* 2 */
    c = FF (c, d, a, b, x[ 2], S13, 0x242070db)#; /* 3 */
    b = FF (b, c, d, a, x[ 3], S14, 0xc1bdceee)#; /* 4 */
    a = FF (a, b, c, d, x[ 4], S11, 0xf57c0faf)#; /* 5 */
    d = FF (d, a, b, c, x[ 5], S12, 0x4787c62a)#; /* 6 */
    c = FF (c, d, a, b, x[ 6], S13, 0xa8304613)#; /* 7 */
    b = FF (b, c, d, a, x[ 7], S14, 0xfd469501)#; /* 8 */
    a = FF (a, b, c, d, x[ 8], S11, 0x698098d8)#; /* 9 */
    d = FF (d, a, b, c, x[ 9], S12, 0x8b44f7af)#; /* 10 */
    c = FF (c, d, a, b, x[10], S13, 0xffff5bb1)#; /* 11 */
    b = FF (b, c, d, a, x[11], S14, 0x895cd7be)#; /* 12 */
    a = FF (a, b, c, d, x[12], S11, 0x6b901122)#; /* 13 */
    d = FF (d, a, b, c, x[13], S12, 0xfd987193)#; /* 14 */
    c = FF (c, d, a, b, x[14], S13, 0xa679438e)#; /* 15 */
    b = FF (b, c, d, a, x[15], S14, 0x49b40821)#; /* 16 */

    # Round 2 
    a = GG (a, b, c, d, x[ 1], S21, 0xf61e2562)#; /* 17 */
    d = GG (d, a, b, c, x[ 6], S22, 0xc040b340)#; /* 18 */
    c = GG (c, d, a, b, x[11], S23, 0x265e5a51)#; /* 19 */
    b = GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa)#; /* 20 */
    a = GG (a, b, c, d, x[ 5], S21, 0xd62f105d)#; /* 21 */
    d = GG (d, a, b, c, x[10], S22,  0x2441453)#; /* 22 */
    c = GG (c, d, a, b, x[15], S23, 0xd8a1e681)#; /* 23 */
    b = GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8)#; /* 24 */
    a = GG (a, b, c, d, x[ 9], S21, 0x21e1cde6)#; /* 25 */
    d = GG (d, a, b, c, x[14], S22, 0xc33707d6)#; /* 26 */
    c = GG (c, d, a, b, x[ 3], S23, 0xf4d50d87)#; /* 27 */
    b = GG (b, c, d, a, x[ 8], S24, 0x455a14ed)#; /* 28 */
    a = GG (a, b, c, d, x[13], S21, 0xa9e3e905)#; /* 29 */
    d = GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8)#; /* 30 */
    c = GG (c, d, a, b, x[ 7], S23, 0x676f02d9)#; /* 31 */
    b = GG (b, c, d, a, x[12], S24, 0x8d2a4c8a)#; /* 32 */

    # Round 3
    a = HH (a, b, c, d, x[ 5], S31, 0xfffa3942)#; /* 33 */
    d = HH (d, a, b, c, x[ 8], S32, 0x8771f681)#; /* 34 */
    c = HH (c, d, a, b, x[11], S33, 0x6d9d6122)#; /* 35 */
    b = HH (b, c, d, a, x[14], S34, 0xfde5380c)#; /* 36 */
    a = HH (a, b, c, d, x[ 1], S31, 0xa4beea44)#; /* 37 */
    d = HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9)#; /* 38 */
    c = HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60)#; /* 39 */
    b = HH (b, c, d, a, x[10], S34, 0xbebfbc70)#; /* 40 */
    a = HH (a, b, c, d, x[13], S31, 0x289b7ec6)#; /* 41 */
    d = HH (d, a, b, c, x[ 0], S32, 0xeaa127fa)#; /* 42 */
    c = HH (c, d, a, b, x[ 3], S33, 0xd4ef3085)#; /* 43 */
    b = HH (b, c, d, a, x[ 6], S34,  0x4881d05)#; /* 44 */
    a = HH (a, b, c, d, x[ 9], S31, 0xd9d4d039)#; /* 45 */
    d = HH (d, a, b, c, x[12], S32, 0xe6db99e5)#; /* 46 */
    c = HH (c, d, a, b, x[15], S33, 0x1fa27cf8)#; /* 47 */
    b = HH (b, c, d, a, x[ 2], S34, 0xc4ac5665)#; /* 48 */

    # Round 4 
    a = II (a, b, c, d, x[ 0], S41, 0xf4292244)#; /* 49 */
    d = II (d, a, b, c, x[ 7], S42, 0x432aff97)#; /* 50 */
    c = II (c, d, a, b, x[14], S43, 0xab9423a7)#; /* 51 */
    b = II (b, c, d, a, x[ 5], S44, 0xfc93a039)#; /* 52 */
    a = II (a, b, c, d, x[12], S41, 0x655b59c3)#; /* 53 */
    d = II (d, a, b, c, x[ 3], S42, 0x8f0ccc92)#; /* 54 */
    c = II (c, d, a, b, x[10], S43, 0xffeff47d)#; /* 55 */
    b = II (b, c, d, a, x[ 1], S44, 0x85845dd1)#; /* 56 */
    a = II (a, b, c, d, x[ 8], S41, 0x6fa87e4f)#; /* 57 */
    d = II (d, a, b, c, x[15], S42, 0xfe2ce6e0)#; /* 58 */
    c = II (c, d, a, b, x[ 6], S43, 0xa3014314)#; /* 59 */
    b = II (b, c, d, a, x[13], S44, 0x4e0811a1)#; /* 60 */
    a = II (a, b, c, d, x[ 4], S41, 0xf7537e82)#; /* 61 */
    d = II (d, a, b, c, x[11], S42, 0xbd3af235)#; /* 62 */
    c = II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb)#; /* 63 */
    b = II (b, c, d, a, x[ 9], S44, 0xeb86d391)#; /* 64 */

    self.state = (0xffffffffL & (state[0] + a),
            0xffffffffL & (state[1] + b),
            0xffffffffL & (state[2] + c),
            0xffffffffL & (state[3] + d),)

    # Zeroize sensitive information.
    del x

# Some helper functions to decode and encode binary data
import struct, string

def Encode(input, len):
  k = len >> 2
  res = apply(struct.pack, ("%iI" % k,) + tuple(input[:k]))
  return string.join(res, "")

def Decode(input, len):
  k = len >> 2
  res = struct.unpack("%iI" % k, input[:len])
  return list(res)





# =================
# = Spoofing Code =
# =================
def spoof_digest(originalDigest, originalLen, spoofMessage=""):
  # first decode digest back into state tuples
  state = Decode(originalDigest, 16)
  
  # generate a seed md5 object
  spoof = md5()
  
  # seed the count variable for calculation of index, padLen, and bits
  spoof.count += originalLen << 3
  
  # calculate some variables to generate the original padding
  index = int((spoof.count >> 3) & 0x3f)
  padLen = (56 - index) if index < 56 else (120 - index)
  bits = Encode((spoof.count & 0xffffffffL, spoof.count>>32), 8)

  # construct the original padding
  padding = PADDING[:padLen]

  # augment the count with the new padding and trailing bits
  spoof.count += len(padding) << 3
  spoof.count += len(bits) << 3
  spoof.state = state
  
  # run an update
  spoof.update(spoofMessage)
  
  # We now have a digest of the original secret + message + some_padding
  return (spoof.digest(), padding + bits)

import sys
HOST = "parlor"
PORT = 4321
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
f = s.makefile('rw', bufsize=0)
def echoline():
  s = f.readline()[:-1]
  print s
  return s
for _ in xrange(17):
  echoline()
def read_menu():
  for _ in xrange(9):
    echoline()
def send(st):
  print st
  f.write(st + '\n')
read_menu()
send('1')
send('124')
echoline()
def extract_lower_bits(nonce):
  read_menu()
  send('3')
  echoline()
  f.write(nonce)
  echoline()
  x = echoline()
  x = x.split(' ')
  x = x[4][:-1]
  z = hex(int(x)).strip("0x").strip("L").rjust(31,'0')
  return z
base = "A" * 48
append = "B" * 64
_, padbits = spoof_digest('\x00' * 32, len(base)+16, append)
l1 = extract_lower_bits(base)
print "Base", l1
l2 = extract_lower_bits(base + padbits + append)
print "new", l2
# Okay, what we want to do now is determine the higher-order 28 bits of the hash
print ["./hash_extender/poop", ("%032x"%int(l1,16)), l2[-8:]]
process = Popen(["./pctf2014/problems/crypto/parlor/hash_extender/poop", ("%032x"%int(l1,16)), l2[-8:]] , stdin=PIPE, stdout=PIPE)
(output, _) = process.communicate()
print output
'''
for x in xrange(2**4):
  m = hex(x).strip("0x").strip("L").rjust(1, '0')
  m = m + l1
  m = m.decode('hex')
  print "trying", m.encode('hex')
  # Okay, try now.
  spf, pd = spoof_digest(m, len(base)+16, append)
  print spf.encode('hex')[1:]
  print l2
  if spf.encode('hex')[1:] == l2:
    print "FOUND ONE"
    print m.encode('hex'), spf.encode('hex')
    break
print spf.encode('hex'), m.encode('hex')
sys.exit(0)
'''
m = (output.strip()[:-1]+l1[6:]).decode("hex")
# Okay, now I have the base hash for my base.
# Now I just bruteforce until the last bit is zero..
used = []
alphabet = "abcdefghijklmnopqrstuvwxyz"
alphabet = alphabet + alphabet.upper()
def get_new_thing():
  global used
  z = ''.join(chr(randint(0,255)) for _ in xrange(64))
  if z not in used:
    used.append(z)
    return z
  return get_new_thing()
read_menu()
send('1')
send('1')
echoline()
money = 998
while money < 1000000000:
  read_menu()
  send('2')
  send(str(money))
  echoline()
  z = get_new_thing()
  res, _ = spoof_digest(m, len(base)+16, append)
  while (int(res.encode('hex'),16)%4 != 0):
    z = get_new_thing()
    res, _ = spoof_digest(m, len(base)+16, z)
    print (int(res.encode('hex'),16)%4)
  # Now I know we win.
  read_menu()
  send('3')
  echoline()
  f.write(base + padbits + z)
  echoline()
  echoline()
  money = money * 2
read_menu()
f.write('6\n')
f.close()
s.close()

