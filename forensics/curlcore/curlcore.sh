#/bin/sh

sudo rm /tmp/capture 2>/dev/null
sudo dumpcap -i eth0 -w /tmp/capture &
DUMPCAPPID=$!

sleep 1
OUTPUT="`/usr/bin/env -i /bin/dash -c 'ulimit -c unlimited; curl -k https://curlcore.local.plaidctf.com/flag.html & PID=$!; sleep 5; printf "generate-core-file\ninfo proc mappings\ndetach\n" | sudo gdb attach $PID; wait'`"
sleep 1

sudo kill -INT $DUMPCAPPID
wait

sudo chown `whoami` /tmp/capture

echo "$OUTPUT"

sudo mv "`echo "$OUTPUT" | grep -o 'Saved corefile .*$' | cut -c 16-`" /tmp/corefile
sudo chown `whoami` /tmp/corefile


echo "$OUTPUT" | awk '/Mapped address spaces/,/(gdb)/' | grep -v '(gdb)' > /tmp/coremaps

rm /tmp/curlcore.tgz 2>/dev/null
tar czf /tmp/curlcore.tgz `grep -o ' /.*$' /tmp/coremaps | sort -us | tr '\n' ' '` /tmp/corefile /tmp/coremaps /tmp/capture "$0"
