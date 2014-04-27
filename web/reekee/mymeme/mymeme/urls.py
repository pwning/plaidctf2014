from django.conf.urls import patterns, include, url

urlpatterns = patterns('mymeme.views',
    (r'login', 'logmein'),
    (r'logout', 'logmeout'),
    (r'register', 'register'),
    (r'make', 'makememe'),
    (r'view/(?P<meme>\d+)', 'viewmeme'),
    (r'view', 'viewmeme'),
    (r'index', 'index'),
    (r'^/?$', 'index')
)
