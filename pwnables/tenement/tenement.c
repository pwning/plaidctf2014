#include <seccomp.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <jansson.h>

#define BUF_SIZE 128

const char *msg = "Welcome to another quality problem. PPP is a big fan of being fair and reasonable.\n"
  "We've therefore constructed this problem to exemplify those two qualities. The flag to\n"
  "this problem has been fairly placed, but we have implemented reasonable protections\n"
  "to overcome and hoops for you to jump through in order to retrieve said flag. If you\n"
  "find this acceptable, please type \"YES\" and we can continue.\n";

int init_seccomp() {
  int rc;

  scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL);
  if (ctx == NULL)
    goto out;

  rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0);
  if (rc < 0)
    goto out;

  rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
  if (rc < 0)
    goto out;

  rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(close), 0);
  if (rc < 0)
    goto out;

  rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(access), 0);
  if (rc < 0)
    goto out;

  rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(dup3), 0);
  if (rc < 0)
    goto out;

  rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(fstat), 0);
  if (rc < 0)
    goto out;

  rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);
  if (rc < 0)
    goto out;

  rc = seccomp_load(ctx);
  if (rc < 0)
    goto out;

  return 0;
out:
  seccomp_release(ctx);
  return -1;
}

int load_key(const char *f) {
  struct {
    size_t s, key_len, i;
    const char *key;
    json_t *json, *tmp, *addrs;
    json_error_t error;
    void *addr, *mapped_addr, *stmp;
  } st;

  st.json = json_load_file(f, 0, &st.error);
  if (st.json == NULL)
    return -1;
  f = NULL;

  st.tmp = json_object_get(st.json, "key");
  if (st.tmp == NULL)
    return -1;

  st.key = json_string_value(st.tmp);
  if (st.key == NULL)
    return -1;
  st.key_len = strlen("pppp: ");
  st.key_len += strlen(st.key);
  st.key_len += 1;

  st.stmp = malloc(st.key_len);
  if (st.stmp == NULL)
    return -1;

  if (snprintf(st.stmp, st.key_len, "%s%s", "pppp: ", st.key) < 0)
    return -1;
  json_decref(st.tmp);

  for (st.i = 0; st.i < 4; st.i++)
    ((char *) st.stmp)[st.i] -= 32;

  st.addrs = json_object_get(st.json, "addrs");
  if (st.addrs == NULL)
    return -1;

  st.s = json_array_size(st.addrs);
  if (st.s == 0)
    return -1;

  while (1) {
    st.tmp = json_array_get(st.addrs, rand() % st.s);
    if (st.tmp == NULL)
      return -1;

    st.addr = (void *) (uintptr_t) json_integer_value(st.tmp);
    if (st.addr == NULL)
      return -1;

    st.mapped_addr = mmap(st.addr, st.key_len, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (st.mapped_addr == MAP_FAILED)
      continue;

    if ((st.mapped_addr - st.addr) > 0x4000 || ((st.addr - st.mapped_addr) > 0x4000)) {
      munmap(st.mapped_addr, st.key_len);
      continue;
    }

    for (st.i = 0; st.i < st.key_len; st.i++) 
      ((char *) st.mapped_addr)[st.i] = ((char *) st.stmp)[st.i];

    if (mprotect(st.mapped_addr, st.key_len, PROT_READ) != 0) {
      munmap(st.mapped_addr, st.key_len);
      continue;
    }

    break;
  }

    json_decref(st.addrs);
    json_decref(st.tmp);
    json_decref(st.json);
    memset(st.stmp, 42, st.key_len);
    free(st.stmp);
    memset(&st, 42, sizeof(st));
    return 0;
}

void run_vuln(void) {
  char buf[BUF_SIZE];
  write(STDOUT_FILENO, msg, strlen(msg));
  read(STDIN_FILENO, buf, BUF_SIZE);
  (* (void(*)()) buf)();
}


int main(int argc, char **argv) {
  srand(time(NULL));

  if (argc != 2) {
    fprintf(stderr, "Usage: ./tenement CONFIG\n");
    return -1;
  }

  if (load_key(argv[1]) < 0)
    return -1;

  if (init_seccomp() < 0)
    return -1;

  run_vuln();
  return 0;
}
