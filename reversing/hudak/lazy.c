// fuck freeing memory
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct thunk {
  int computed;
  void *value;
  void *((*compute)(struct thunk *));
  struct thunk **arguments;
};
typedef struct thunk thunk;

thunk **make_args(thunk *t) {
  thunk **b = malloc(sizeof(thunk *));
  *b = t;
  return b;
}

thunk *make_thunk(void *(*f)(thunk *)) {
  thunk *t = malloc(sizeof(thunk));
  t->computed = 0; t->value = NULL;
  t->compute = f; t->arguments = NULL;
  return t;
}

void *id(thunk *t) {
  return t->value;
}

int string_length(thunk *t) {
  if (t->computed) return t->value;
  char *s;
  thunk *a1 = t->arguments[0];
  s = (char *)a1->compute(a1);
  int l = 0;
  while (s[l] != -1) l++;
  t->computed = 1;
  t->value = l;
  return t->value;
}

char *rotate(thunk *t) {
  if (t->computed) return t->value;
  char *s; int k, l;
  thunk *a1 = t->arguments[0];
  thunk *a2 = t->arguments[1];
  thunk *a3 = t->arguments[2];
  s = (char *)(a1->compute(a1));
  k = (int)(a2->compute(a2));
  l = (int)(a3->compute(a3)) + 1;
  char *r = malloc(sizeof(char) * l);
  int i = 0;
  for (; i < l; i++) {
    r[i] = s[(i + k) % l];
  }
  t->computed = 1;
  t->value = r;
  return r;
}

thunk **rotate_by_all(thunk *t) {
  if (t->computed) return t->value;
  char *s; int k;
  thunk *a1 = t->arguments[0];
  thunk *a2 = t->arguments[1];
  k = (int)(a2->compute(a2)) + 1;
  char **r = malloc(sizeof(thunk *) * k);
  int i;
  for (i = 0; i < k; i++) {
    thunk *a = make_thunk(rotate);
    a->arguments = malloc(sizeof(thunk *) * 3);
    thunk *b = make_thunk(id);
    b->computed = 1; b->value = (void *)i;
    a->arguments[0] = a1;
    a->arguments[1] = b;
    a->arguments[2] = a2;
    r[i] = a;
  }
  t->computed = 1;
  t->value = r;
  return r;
}

char **sorted(thunk *t) {
  if (t->computed) return t->value;
  char **strings; int k;
  thunk *a1 = t->arguments[0];
  thunk *a2 = t->arguments[1];
  thunk **thunks = (thunk **)(a1->compute(a1));
  k = (int)(a2->compute(a2)) + 1;
  strings = malloc(sizeof(char *) * k);
  int i;
  for (i = 0; i < k; i++) {
    strings[i] = (char *)(thunks[i]->compute(thunks[i]));
  }
  for (i = 0; i < k; i++) {
    int j;
    for (j = 0; j < k-1; j++) {
      if (memcmp(strings[j], strings[j+1], k) > 0) {
        char *tmp = strings[j];
        strings[j] = strings[j + 1];
        strings[j + 1] = tmp;
      }
    }
  }
  t->computed = 1;
  t->value = strings;
  return strings;
}

char *get_last(thunk *t) {
  if (t->computed) return t->value;
  char **strings; int k;
  thunk *a1 = t->arguments[0];
  thunk *a2 = t->arguments[1];
  strings = (char **)(a1->compute(a1));
  k = (int)(a2->compute(a2));
  char *s = malloc(sizeof(char) * (k + 1));
  int i = 0;
  for (; i < k+1; i++) {
    s[i] = strings[i][k] ^ 0x37;
  }
  s[k+1] = 0;
  t->value = s;
  t->computed = 1;
  return s;
}

thunk *burrows_wheeler(thunk *t) {
  if (t->computed) return t->value;
  thunk *a = make_thunk(string_length);
  a->arguments = make_args(t->arguments[0]);
  thunk *b = make_thunk(rotate_by_all);
  b->arguments = malloc(sizeof(thunk *) * 2);
  b->arguments[0] = a->arguments[0];
  b->arguments[1] = a;
  thunk *c = make_thunk(sorted);
  c->arguments = malloc(sizeof(thunk *) * 2);
  c->arguments[0] = b;
  c->arguments[1] = a;
  thunk *d = make_thunk(get_last);
  d->arguments = malloc(sizeof(thunk *) * 2);
  d->arguments[0] = c;
  d->arguments[1] = a;
  t->computed = 1;
  t->value = d;
  return d;
}

int verify(thunk *t) {
  thunk *bw = make_thunk(burrows_wheeler);
  bw->arguments = malloc(sizeof(thunk *));
  bw->arguments[0] = t->arguments[0];
  char *secret = "\x04\x19\x19\x68\x43\x41\x5b\x04\xc8\x44\x43\x43\x43\x40\x45\x47\x68\x68\x06\x5a\x52\x56\x03\x56\x44\x03\x5e\x06\x68\x19";
  thunk *bwc = bw->compute(bw);
  char *bwr = bwc->compute(bwc);
  if (memcmp(secret, bwr, 31) == 0)
    return 1;
  return 0;
}

size_t readuntil(int fd, char *buf, size_t nbyte, char endchr) {
  char *rbuf = buf;
  for(size_t i = 1; i < nbyte; i++) {
    ssize_t r = read(fd, rbuf, 1);
    if(r == 0) break;
    if(r < 0) {
      perror("read_until");
      exit(1);
    }
    if(r > 1) exit(-1);
    if(*(rbuf++) == endchr) break;
  }
  *(rbuf-1) = 0xFF; *rbuf = 0;
  return rbuf - buf;
}

int main() {
  char buf[80];
  printf("Enter the password.\n");
  readuntil(STDIN_FILENO, buf, 80, '\n');
  // meh.
  if (strlen(buf) != 30) { puts("Wrong! :("); return; }
  thunk *t = make_thunk(verify);
  thunk *a = make_thunk(id);
  a->value = buf;
  a->computed = 1;
  t->arguments = make_args(a);
  if (t->compute(t)) {
    puts("Congratulations! :)");
  } else {
    puts("Wrong! :(");
  }
}


