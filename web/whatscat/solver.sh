#!/bin/bash

USERNAME=${RANDOM}_tester

curl --silent -g "http://[2001:470:8:f7d::1]/index.php?page=login" --data "name=${USERNAME}&pass=${USERNAME}&email=poop&register=1" > /dev/null

curl --silent --interface 2001:470:8:f81::2 -g "http://[2001:470:8:f7d::1]/index.php?page=login" --data "name=${USERNAME}&reset=1" > /dev/null

curl --silent -g "http://[2001:470:8:f7d::1]/index.php?page=login" --data "name=${USERNAME}&reset=1" | grep email
