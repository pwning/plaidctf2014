from struct import pack

def make_state(st):
  acc, dc = st
  s = pack('I', acc)
  for x in xrange(256):
    if chr(x) in dc:
      # wheee
      s += pack('I', dc[chr(x)])
    else:
      s += pack('I', 1)
  return s

def make_dfa(states):
  s = pack('I', len(states))
  for st in states:
    s += make_state(st)
  return s

def out_dfa(states, filename):
  f = open(filename, 'w')
  f.write(repr(make_dfa(states)).replace('"', '\\"').replace("'", '"') + ',')
  f.close()

# okay.
# {my_synchronization_skills_suck}
# first, wrap string in { }
def bounded_by_braces():
  s0 = { '{': 2 }
  s1 = { }
  s2 = dict((chr(c), 2) for c in xrange(256))
  s2['}'] = 3
  s3 = { }
  print 4 * 257 + 1
  out_dfa([(0, s0), (0, s1), (0, s2), (1, s3)], "braces.txt")
def length_32():
  s0 = dict((chr(c), 2) for c in xrange(256)) 
  s1 = { }
  s = []
  for x in xrange(31):
    s.append(dict((chr(c),x+3) for c in xrange(256)))
  for x in xrange(len(s)):
    s[x] = (0, s[x])
  s = [(0,s0), (0,s1)] + s + [(1, dict((chr(c), 1) for c in xrange(256)))]
  print len(s) * 257 + 1
  out_dfa(s, "length32.txt")
def underscore_count():
  s0 = dict((chr(c), 0) for c in xrange(256))
  s0['_'] = 2
  s1 = {}
  s2 = dict((chr(c), 2) for c in xrange(256))
  s2['_'] = 3
  s3 = dict((chr(c), 3) for c in xrange(256))
  s3['_'] = 4
  s4 = dict((chr(c), 4) for c in xrange(256))
  print 5 * 257 + 1
  out_dfa([(0, s0), (0,s1), (0, s2), (0,s3), (1, s4)], "underscores.txt")
def my():
  s0 = dict((chr(c), 2) for c in xrange(256))
  s0['_'] = 1
  s1 = { }
  s2 = { 'm': 3 }
  s3 = { 'y': 4 }
  s4 = dict((chr(c), 4) for c in xrange(256))
  print 5 * 257 + 1
  out_dfa([(0, s0), (0, s1), (0, s2), (0, s3), (1, s4)], "my.txt")
def synchronization():
  s0 = dict((chr(c), 0) for c in xrange(256))
  s0['_'] = 2
  s1 = { }
  st = "synchronization"
  s = []
  i = 2
  for c in st:
    tm = { c: i + 1 }
    s.append(tm)
    i = i + 1
  sn = dict((chr(c), i) for c in xrange(256))
  s = [(0, s0), (0, s1)] + [(0, x) for x in s] + [(1, sn)]
  print len(s) * 257 + 1
  out_dfa(s, "synchronization.txt")
def skills():
  s0 = dict((chr(c), 0) for c in xrange(256))
  s0['_'] = 2
  s1 = { }
  st = "skills"
  s2 = dict((chr(c), 2) for c in xrange(256))
  s2['_'] = 3
  s = []
  i = 3
  for c in st:
    tm = { c: i + 1 }
    s.append(tm)
    i = i + 1
  sn = dict((chr(c), i) for c in xrange(256))
  s = [(0, s0), (0, s1), (0, s2)] + [(0, x) for x in s] + [(1, sn)]
  print len(s) * 257 + 1
  out_dfa(s, "skills.txt")
def suck():
  s0 = dict((chr(c), 0) for c in xrange(256))
  s0['_'] = 2
  s1 = { }
  st = "suck"
  s2 = dict((chr(c), 2) for c in xrange(256))
  s2['_'] = 3
  s3 = dict((chr(c), 3) for c in xrange(256))
  s3['_'] = 4
  s = []
  i = 4
  for c in st:
    tm = { c: i + 1 }
    s.append(tm)
    i = i + 1
  sn = dict((chr(c), i) for c in xrange(256))
  s = [(0, s0), (0, s1), (0, s2), (0, s3)] + [(0, x) for x in s] + [(1, sn)]
  print len(s) * 257 + 1
  out_dfa(s, "suck.txt")

bounded_by_braces()
length_32()
underscore_count()
my()
synchronization()
skills()
suck()

