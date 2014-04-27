#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <stdint.h>

#define DATASIZE 131072
#define NUMWORKERS 7
enum Ops {
  UPDATE_PID,
  INIT_DFA,
  REDUCE,
  STEP_DFA,
  GET_STATE
};

struct pdata {
  int gotmsg;
  int target;
  int op;
  int msg[DATASIZE];
};

struct pdata data;

void sigusr1_handler(int sig) { }

static int pid;
static int parent;

static __attribute__((always_inline)) void ptracew(enum __ptrace_request request, pid_t pid, void *addr, void *d) {
  if (ptrace(request, pid, addr, d) < 0) {
    perror("ptrace error!");
    kill(pid, SIGKILL);
    exit(1);
  }
}

struct dfa_state {
  int type;
  int transitions[256];
};

typedef struct dfa_state dfa_state;

struct dfa_def {
  int len;
  dfa_state states[];
};

static struct dfa_def *dfa = NULL;
static int currstate = 0;

static __attribute__((always_inline)) void send_data(int tgtpid, int op, int tgtid, int len, int *msg) {
  // sketchy-ass synchronization
  usleep(50*1000);
  ptracew(PTRACE_ATTACH, tgtpid, NULL, NULL);
  wait(NULL);
  ptracew(PTRACE_POKEDATA, tgtpid, &data.gotmsg, 1);
  ptracew(PTRACE_POKEDATA, tgtpid, &data.target, tgtid);
  ptracew(PTRACE_POKEDATA, tgtpid, &data.op, op);
  int i;
  for (i = 0; i < len; i++)
    ptracew(PTRACE_POKEDATA, tgtpid, &data.msg[i], msg[i]);
  ptracew(PTRACE_DETACH, tgtpid, NULL, NULL);
  kill(tgtpid, SIGUSR1);
}

static void __attribute__((always_inline)) forward_message(void) {
  send_data(pid, data.op, data.target, DATASIZE, data.msg);
}

static __attribute__((always_inline)) void handle_init_dfa(int *args) {
  int len = args[0];
  if (dfa) free(dfa);
  dfa = malloc(sizeof(dfa_state) * len + 4);
  dfa->len = len;
  memcpy(dfa->states, &args[1], sizeof(dfa_state) * len);
}

static __attribute__((always_inline)) void handle_reduce(int id) {
  int mypid = getpid();
  int i;
  int accum = 1;
  for (i = 0; i < NUMWORKERS; i++) {
    if (i == id) {
      accum &= dfa->states[currstate].type;
      continue;
    }
    send_data(pid, GET_STATE, i, 1, &mypid);
    while (!data.gotmsg) {
      sleep(1000);
    }
    data.gotmsg = 0;
    accum &= data.msg[0];
  }
  send_data(parent, 0, 0, 1, &accum);
}

int __attribute__((always_inline)) handle_getstate(int tgt) {
  int st = dfa->states[currstate].type;
  send_data(tgt, 0, 0, 1, &st);
}

static void handle_message(int id, int op, int *args) {
  switch(op) {
  case UPDATE_PID:
    pid = args[0];
    break;
  case REDUCE:
    handle_reduce(id);
    break;
  case INIT_DFA:
    printf(".");
    fflush(stdout);
    handle_init_dfa(args);
    break;
  case STEP_DFA:
    currstate = dfa->states[currstate].transitions[args[0]];
    if (id + 1 < NUMWORKERS) {
      data.target = id + 1;
      data.gotmsg = 1;
      forward_message();
      data.gotmsg = 0;
    } else {
      sleep(1); // we rest, and then alert our parent that all children have stepped.
      kill(parent, SIGUSR1);
    }
    break;
  case GET_STATE:
    handle_getstate(args[0]);
    break;
  }
}

static void wait_for_message(int id) {
  int done = 0;
  data.gotmsg = 0;
  while (!done) {
    while (!data.gotmsg) {
      sleep(1000);
    }
    data.gotmsg = 0;
    if (data.target == id)
      handle_message(id, data.op, data.msg);
    else
      forward_message();
  }
  exit(0);
}

static int pids[NUMWORKERS];

static __attribute__((always_inline)) void send_worker(int id, int op, int len, int *data) {
  send_data(pids[id], op, id, len, data);
}

int dfalens[NUMWORKERS] = { 1029, 8739, 1286, 1286, 4627, 2571, 2314 };
char *dfas[NUMWORKERS] = {
#include "braces.txt"
#include "length32.txt"
#include "underscores.txt"
#include "my.txt"
#include "synchronization.txt"
#include "skills.txt"
#include "suck.txt"
};

static inline void do_parent(int lastpid) {
  send_worker(0, UPDATE_PID, 1, &lastpid);
  // Okay, we have told the worker about the pid.
  // Now, we just "make sure" that it has received it...
  sleep(1);
  // Jesus the races.
  // Okay, now we give each worker its DFA.
  int worker = 0;
  for (worker = 0; worker < NUMWORKERS; worker++) {
    send_worker(worker, INIT_DFA, dfalens[worker], dfas[worker]);
  }
  sleep(1);
  puts("");
  // Okay. Each worker has its DFA. Now, we read our string.
  char buf[100];
  int *ibuf = (int*)(&buf[0]);
  printf("Please enter a string: ");
  if (fgets(buf, 80, stdin) == NULL) {
    perror("String not read!");
    kill(pid, SIGKILL);
    exit(1);
  }
  int blen = strlen(buf) - 1;
  buf[blen] = '\0';
  int i = 0;
  for (i = 0; i < blen; i++) {
    int chr = buf[i];
    printf(".");
    fflush(stdout);
    send_worker(0, STEP_DFA, 1, &chr);
    // Okay, now we sleep and wait to be alerted by a child.
    sleep(1000);
  }
  puts("");
  data.gotmsg = 0;
  send_worker(0, REDUCE, 0, NULL);
  while (!data.gotmsg) sleep(1000);
  int result = data.msg[0];
  if (result) puts("Congratulations! That's the right key!");
  else puts("Sorry, wrong.");
  for (i = 0; i < NUMWORKERS; i++) {
    kill(pids[i], SIGKILL);
  }
  exit(0);
}

int main(void)
{
  struct sigaction sa;
  sa.sa_handler = sigusr1_handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  puts("This may take a while...");
  if (sigaction(SIGUSR1, &sa, NULL) == -1) {
      perror("sigaction");
      exit(1);
  }
  parent = pid = getpid();
  {
    int id = 0;
    int lastpid = pid;
    for (id = 0; id < NUMWORKERS; id++) {
      int child;
      if (child = fork()) {
        lastpid = child;
        pids[id] = child;
      }
      else {
        pid = lastpid;
        wait_for_message(id);
      }
    }
    sleep(1);
    do_parent(lastpid);
  }
  return 0;
}


