#include "hash_extender_engine.h"
#include <ctype.h>
#include <err.h>
#include <getopt.h>

#include "buffer.h"
#include "formats.h"
#include "util.h"
#include <arpa/inet.h>

static unsigned int hex2int(char ch){
  if(ch>='0' && ch<='9') return ch-'0';
  if(ch>='a' && ch<='f') return ch-'a'+10;
  if(ch>='A' && ch<='F') return ch-'A'+10;
  return 0;
}

int main(int argc, char** argv) {
  uint8_t *b = malloc(64);
  memset(b,'B',64);
  uint8_t *ns = malloc(16);

  uint8_t *hash = calloc(16,1);
  unsigned int match = htonl(strtol(argv[2],0,16));
  unsigned int i;
  for (i = 0; i < 16; i++) {
    hash[i] = (hex2int(argv[1][2*i])<<4) + hex2int(argv[1][1+(2*i)]);
  }
  for (i = 0; i < 0x10000000; i++) {
    *(unsigned int*)&hash[0] &= 0x0f000000;
    *(unsigned int*)&hash[0] |= ((i & 0x0f000000)<<4) | (i & 0x00ffffff);
    hash_gen_signature_evil("md5",16,48,hash,b,64,ns);
    /* printf("%s\n",ns); */
    if (!memcmp(ns+12,(char*)&match,4)) {
      printf("%08x\n",htonl(((i & 0x0f000000)<<4) | (i & 0x00ffffff)));
      return 1;
    }
  }
  return 0;
}
