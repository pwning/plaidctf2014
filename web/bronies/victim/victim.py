#!/usr/bin/python
import sys
import time
import hashlib
import hmac
import struct
from selenium import webdriver
from selenium.webdriver.common.keys import Keys
import beanstalkc
import multiprocessing

NPROCS=16
LOGIN_URL = 'http://portal.essolutions.largestctf.com/'
XSS_SITE = 'http://54.196.225.30/'
HMAC_KEY = '34a23eb4a5dc54e73c459cb1a1ecad22'

def frob(s):
    return ''.join(chr(ord(x)^42) for x in s)

def generate_otp(username, password):
    tok = (username + '\n:' + frob(password + '\n') + ':' +
           struct.pack('<I', int(time.time()) / 600))
    return struct.unpack('<I', hashlib.sha256(tok).digest()[:4])[0] % 3133337

def get_token(mid):
    return hmac.new(HMAC_KEY, str(mid), hashlib.sha256).hexdigest()

username = 'ebleford'
password = "this is a really long string you'll never guess"

def robust(f):
    def g(*args, **kwargs):
        try:
            f(*args, **kwargs)
        except:
            pass
    return g

@robust
def work(mid):
    driver = webdriver.PhantomJS(
       service_args=['--ignore-ssl-errors=true', '--ssl-protocol=any'],
       service_log_path='/dev/null')
    try:
        driver.set_page_load_timeout(2 * 60)
        driver.get(LOGIN_URL)
        driver.find_element_by_id('username').send_keys(username)
        driver.find_element_by_id('password').send_keys(password)

        otp = generate_otp(username, password)
        driver.find_element_by_id('otp').send_keys(str(otp))
        driver.find_element_by_id('otp').send_keys(Keys.RETURN)

        url = XSS_SITE + 'index.php?page=msgs&mid=' + str(int(mid))
        url += '&prevent_teams_from_stealing_your_xss_payload='
        url += get_token(mid)
        driver.get(url)
        time.sleep(5)
    except:
        pass
    finally:
        driver.quit()


def worker():
    beanstalk = beanstalkc.Connection(host='127.0.0.1', port=11300)
    while True:
        job = beanstalk.reserve()
        mid = int(job.body)
        print 'Got job:', mid
        work(mid)
        job.delete()


def runpool():
    for i in xrange(NPROCS - 1):
        multiprocessing.Process(target=worker).start()
    worker()

if __name__ == '__main__':
    runpool()
