#!/bin/sh
IP=i.dont.know:8001
test -z "$1" && echo You could give me an ip:port to make a proper cert... or not care. || IP="$1"
openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -days 100 -nodes -subj '/C=US/O=PlaidCTF/OU=halphow2js/CN='"$IP"
