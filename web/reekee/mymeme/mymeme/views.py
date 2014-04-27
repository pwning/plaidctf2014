from django.http import HttpResponse
from django.contrib.auth import authenticate, login, logout
from django.contrib.auth.models import User
from django.contrib.auth.decorators import login_required
#from django.contrib.auth.forms import 
from django.shortcuts import redirect,render
from django import forms
import os,urllib2,imghdr
from PIL import Image, ImageFont, ImageDraw

BACK = """</br><script>
function back()
  {
  window.history.back()
  }
</script>

<body>
<button onclick="back()">Go Back</button>
</body>"""

def user_exists(username):
  if User.objects.filter(username=username).count():
    return True
  return False

def get_next_file(username):
  bp = "/tmp/memes/"+username+"/"
  if len(os.listdir(bp)) > 9:
    return bp+min(os.listdir(bp), key=lambda x:os.path.getctime(bp+x))
  else:
    return bp+str(len(os.listdir(bp)))

def add_text(fn,fmt,text):
  i = Image.open(fn)
  d = ImageDraw.Draw(i)
  d.text((0,0),text,(255,255,255),font=ImageFont.truetype("font.ttf", 30))
  i.save(fn,format=fmt)

def logmein(request):
  if request.method == 'POST':
    username = request.POST['username']
    password = request.POST['password']
    user = authenticate(username=username, password=password)
    if user is not None:
      if user.is_active:
        login(request, user)
        return redirect('/index')
    return HttpResponse("Error: login failed"+BACK)
  return render(request,"login.html")

def logmeout(request):
  logout(request)
  return redirect('/index')

def register(request):
  if request.method == 'POST':
    username = request.POST['username']
    password = request.POST['password']
    if user_exists(username):
      return HttpResponse("Error: user exists"+BACK)
    if (".." in username) or ("/" in username):
      return HttpResponse("Error: invalid username"+BACK)
    try:
      os.mkdir("/tmp/memes/"+username)
    except:
      return HttpResponse("Error: failed to create user"+BACK)
    User.objects.create_user(username,password=password)
    user = authenticate(username=username, password=password)
    login(request,user)
    return redirect('/index')
  return render(request,"register.html")

@login_required(login_url='/login')
def makememe(request):
  username = str(request.user)
  if request.method == 'POST':
    url = request.POST['url']
    text = request.POST['text']
    try:
      if "http://" in url:
        image = urllib2.urlopen(url)
      else:
        image = urllib2.urlopen("http://"+url)
    except:
      return HttpResponse("Error: couldn't get to that URL"+BACK)
    if int(image.headers["Content-Length"]) > 1024*1024:
      return HttpResponse("File too large")
    fn = get_next_file(username)
    print fn
    open(fn,"w").write(image.read())
    add_text(fn,imghdr.what(fn),text)
  return render(request,"make.html",{'files':os.listdir("/tmp/memes/"+username)})

@login_required(login_url='/login')
def viewmeme(request,meme=None):
  print meme
  username = str(request.user)
  if meme is not None:
    filename = "/tmp/memes/"+username+"/"+str(meme)
    ctype = str(imghdr.what(filename))
    return HttpResponse(open(filename).read(),content_type="image/"+ctype)
  else:
    return render(request,"view.html",{'files':sorted(os.listdir("/tmp/memes/"+username), key=lambda x:os.path.getctime(bp+x) )})
  return HttpResponse("view"+username)

def index(request):
  print [request.session[a] for a in request.session.keys()]
  return render(request,"index.html",{'auth':request.user.is_authenticated()})
