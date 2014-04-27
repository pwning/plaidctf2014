from random import randint
alphabet = "abcdefghijklmnopqrstuvwxyz"
plaintext = filter(lambda z: z in alphabet, open('plaintext').read().lower())
d = {}
chosen = []
for c in alphabet:
  z = randint(0, 25)
  while alphabet[z] in chosen:
    z = randint(0, 25)
  d[c] = alphabet[z]
  chosen.append(alphabet[z])

print ''.join(d[c] for c in plaintext)

