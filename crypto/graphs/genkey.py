#!/usr/bin/env python

import os,random,math,pickle,zlib

'''
Encrypts a message using super secure, NP-hard graph stuff.

Encryption works like this: take a graph, split your message up into
N numbers, such that the sum of all the numbers is equal to your message.
Assign each vertex to one of these N numbers. 

Then ciphertext[v] is the sum of v's N, and all of the N's from v's neighbors
'''

class Graphcrypt:
  def __init__(self,fil=None,pubkey=None,privkey=None,keylen=None):
    if fil is not None:
      c = ''.join(open(fil).read().split("\n")[1:-1]).decode("base64")
      key = pickle.loads(zlib.decompress(c))
      if 'pub' in key:
        self.pubkey = key['pub']
        self.keylen = len(self.pubkey)
      elif 'priv' in key:
        self.pubkey = key['priv'].pubkey
        self.privkey = key['priv'].privkey
        self.keylen = key['priv'].keylen
    elif (pubkey == None) and (privkey == None):
      if keylen is None:
        self.keylen = 1024
      self.new()
    else:
      if (pubkey is None):
        raise Exception('u need a pub key, dawg')
      self.pubkey = pubkey
      self.keylen = len(self.pubkey)
      self.privkey = privkey

  def new(self):
    random.seed(os.urandom(self.keylen>>3)) #get some tasty randomness

    #use this to generate a random graph
    vertices = range(self.keylen)
    privkey = random.sample(vertices,self.keylen>>4)

    tocover = set(vertices).difference(set(privkey))

    G = [0]*self.keylen
    for v in vertices:
      G[v] = []

    while len(tocover) > 0:
      src = random.choice(privkey)
      dst = random.choice(list(tocover))
      G[src].append(dst)
      G[dst].append(src)
      tocover = tocover.difference(set([dst]))

    others = list(set(vertices).difference(set(privkey)))
    for o in others:
      for n in others:
        if random.getrandbits(5) == 0:
          if o not in G[n]:
            G[n].append(o)
            G[o].append(n)
    self.privkey = privkey
    self.pubkey = G

  def encrypt(self,message):
    t = message/self.keylen
    vals = [int(random.gauss(t,t)) for _ in xrange(self.keylen-1)]
    missing = message - reduce(lambda a,b:a+b, vals)
    vals.append(missing)
    assert(reduce(lambda a,b:a+b, vals) == message)

    ctext = [0] * self.keylen
    for v in xrange(self.keylen):
      ctext[v] = vals[v]
      for n in self.pubkey[v]:
        ctext[v] += vals[n]
    return zlib.compress(pickle.dumps(ctext),9).encode("base64")

  def decrypt(self,ct):
    if self.privkey == None:
      raise Exception('lol u dont hav a private key how u decrypt')
    ct = pickle.loads(zlib.decompress(ct.decode("base64")))
    return reduce(lambda a,b:a+b,[ct[i] for i in self.privkey])

  def exportpub(self,fil):
    s = "-----BEGIN GRAPHCRYPT PUBLIC KEY-----\n"
    s += zlib.compress(pickle.dumps({'pub':self.pubkey}),9).encode("base64")
    s += "-----END GRAPHCRYPT PUBLIC KEY-----\n"
    open(fil,'w').write(s)

  def exportpriv(self,fil):
    s = "-----BEGIN GRAPHCRYPT PRIVATE KEY-----\n"
    s += zlib.compress(pickle.dumps({'priv':self}),9).encode("base64")
    s += "-----END GRAPHCRYPT PRIVATE KEY-----\n"
    open(fil,'w').write(s)