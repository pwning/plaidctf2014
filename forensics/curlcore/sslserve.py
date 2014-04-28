#!/usr/bin/python
import BaseHTTPServer, SimpleHTTPServer
import ssl

from time import sleep

class MyHandler(BaseHTTPServer.BaseHTTPRequestHandler):
	def do_GET(self):
		if self.path != "/flag.html":
			self.send_response(404)
			return
		self.send_response(200)
		sleep(10)
		self.send_header("Content-type", "text/html")
		self.end_headers()
		self.wfile.write('''\
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>curlcore</title>
</head>
<body>
<h1>CONGRATULATIONS!</h1>
<p>
  Your flag is: congratz_you_beat_openssl_as_a_whitebox
</p>
</body>
</html>
''')

httpd = BaseHTTPServer.HTTPServer(('10.211.55.2', 443), MyHandler)
httpd.socket = ssl.wrap_socket (httpd.socket, certfile='pem.pem', server_side=True)
httpd.serve_forever()
