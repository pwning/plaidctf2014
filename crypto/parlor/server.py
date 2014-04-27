#!/usr/bin/env python

from hashlib import md5
import SocketServer,threading,os

msg = """/------------------------------------------------------------------------------\\
| Welcome to the betting parlor!                                               |
|                                                                              |
| We implement State of the Art cryptography to give you the fairest and most  |
| exciting betting experience!                                                 |
|                                                                              |
| Here's how it works: we both pick a nonce, you tell us odds, and you give us |
| some money.                                                                  |
| If md5(our number + your number) % odds == 0, you win bet amount*odds.       |
| Otherwise, we get your money! We're even so nice, we gave you $1000 to start.|
|                                                                              |
| If you don't trust us, we will generate a new nonce, and reveal the old nonce|
| to you, so you can verify all of our results!                                |
|                                                                              |
| (Oh, and if you win a billion dollars, we'll give you a flag.)               |
\______________________________________________________________________________/
"""

options = """
====================
  1) set your odds
  2) set your bet
  3) play a round
  4) get balance
  5) reveal nonce
  6) quit
====================

"""

def getn(s):
  try:
    return int(s.recv(1024))
  except:
    return 0

class threadedserver(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
  pass

class incoming(SocketServer.BaseRequestHandler):
  def handle(self):
    cur_thread = threading.current_thread()
    self.nonce = os.urandom(16)
    self.user_nonces = []
    self.monies = 1000
    self.bet = 1
    self.odds = 1

    self.request.send(msg)

    while True:
      if self.monies >= 1000000000:
        self.request.send("Holy shit you have a lot of money. Here's a key: XXXXXXXXXXXXXXX\n")
      self.request.send(options)
      m = getn(self.request)
      if m == 1:
        self.request.send("Please pick odds (as a power of 2 between 1 and 100): ")
        self.odds = min(max(getn(self.request),1),100)
        self.request.send("Odds set to 2^%d, good luck!"%self.odds)
      elif m == 2:
        self.request.send("Please pick your bet amount (between 0 and %d): "%self.monies)
        self.bet = min(max(0,getn(self.request)),self.monies)
        self.request.send("Alright, bet amount set to $%d, good luck!"%self.bet)
      elif m == 3:
        self.request.send("Okay, send us a nonce for this round!\n")
        self.user_nonces.append(self.request.recv(1024))
        if self.user_nonces[-1] in self.user_nonces[:-1]:
          self.request.send("What part of NONCE don't you understand, asshole?\n")
          return 0
        self.bet = min(self.bet,self.monies)
        self.monies -= self.bet
        self.request.send("Betting $%d at odds of 2^%d"%(self.bet,self.odds))
        result = int(md5(self.nonce + self.user_nonces[-1]).hexdigest(),16)&((2<<(self.odds-1))-1)
        if result == 0:
          self.request.send("\nWow! You won, congratulations!")
          self.monies += self.bet*(2<<(self.odds-1))
        else:
          self.request.send("\nToo bad, we generated %d, not 0... better luck next time!"%result)
      elif m == 4:
        self.request.send("Your current balance is $%d"%self.monies)
      elif m == 5:
        self.request.send("What? You think we're cheaters? Fine, the nonce has been "+
          self.nonce.encode("hex")+"\n")
        self.request.send("Who is the cheater now, huh?")
        self.nonce = os.urandom(16)
        self.user_nonces = []
      elif m == 6:
        return 0


SocketServer.TCPServer.allow_reuse_address = True
server = threadedserver(("0.0.0.0", 4321), incoming)
server.timeout = 4
server_thread = threading.Thread(target=server.serve_forever)
server_thread.daemon = False
server_thread.start()

