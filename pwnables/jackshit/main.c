#include <stdlib.h>
#include <stdio.h>
#include <seccomp.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <pwd.h>

void setup_seccomp() {
	scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL);
	if(!ctx) exit(-1);
	if(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 0)) exit(-1);
	if(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(socket), 0)) exit(-1);
#if (SCMP_SYS(setsockopt))
#warning System does not have sys_setsockopt, the expected way to do SO_REUSEADDR.
	if(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(setsockopt), 0)) exit(-1);
#endif
#ifdef NEEDS_SOCKETCALL
#warning NEEDS_SOCKETCALL=1 has been set, I wish it was not needed on some systems.
	if(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(socketcall), 0)) exit(-1);
#endif
	if(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(bind), 0)) exit(-1);
	if(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(listen), 0)) exit(-1);
	if(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(rt_sigaction), 0)) exit(-1);
	if(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(accept), 0)) exit(-1);
	if(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(clone), 0)) exit(-1);
	if(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(setresuid32), 0)) exit(-1);
	if(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(setresgid32), 0)) exit(-1);
	if(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(alarm), 0)) exit(-1);
	if(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(dup2), 0)) exit(-1);
	if(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(close), 0)) exit(-1);
	if(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0)) exit(-1);
	if(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0)) exit(-1);
	if(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0)) exit(-1);
	
	// Allow further restrictions.
	if(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(prctl), 1, SCMP_A0(SCMP_CMP_EQ, PR_SET_NO_NEW_PRIVS))) exit(-1);
	if(seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(prctl), 1, SCMP_A0(SCMP_CMP_EQ, PR_SET_SECCOMP))) exit(-1);
	
	if(seccomp_load(ctx)) exit(-1);
	seccomp_release(ctx);
}

void drop_sys(int syscall) {
	scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ALLOW);
	if(seccomp_rule_add(ctx, SCMP_ACT_KILL, syscall, 0)) exit(-1);
	if(seccomp_load(ctx)) exit(-1);
	seccomp_release(ctx);
}

int problem();

#define MYPORT (1282)

int main(int argc, char *argv[]) {
#ifdef LOCAL_TESTING
	fprintf(stderr, "Local test version %s.\nOpening at 127.0.0.1:%d\n", argv[0], MYPORT);
#endif
	
	struct passwd *jack = getpwnam("jack");
	uid_t euid = jack->pw_uid;
	uid_t egid = jack->pw_gid;
	
	int ret;
	setup_seccomp();
	
	struct sockaddr_in addr = { .sin_family = AF_INET,
#ifdef LOCAL_TESTING
	                            .sin_addr = { .s_addr = htonl(INADDR_LOOPBACK) },
#else
	                            .sin_addr = { .s_addr = htonl(INADDR_ANY) },
#endif
	                            .sin_port = htons(MYPORT) };
	socklen_t addrlen = sizeof(addr);
	
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); drop_sys(SCMP_SYS(socket));
	if(sock < 0) exit(-1);
	
#if (SCMP_SYS(setsockopt)) || defined(NEEDS_SOCKETCALL)
	int truth = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &truth, sizeof truth);
#endif
#if (SCMP_SYS(setsockopt))
	drop_sys(SCMP_SYS(setsockopt));
#endif

	ret = bind(sock, (struct sockaddr*)&addr, addrlen); drop_sys(SCMP_SYS(bind));
	if(ret) exit(-1);
	
	listen(sock, 200); drop_sys(SCMP_SYS(listen));
	if(ret) exit(-1);
	
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = SA_NOCLDWAIT;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGCHLD, &sa, NULL); drop_sys(SCMP_SYS(rt_sigaction));
	
	while(1) {
		int fd = accept(sock, (struct sockaddr*)&addr, &addrlen);
		if(fd > 0 &&
#ifdef LOCAL_TESTING
		   (fork() || (exit(0),1))
#else
		   fork() == 0
#endif
		) {
			drop_sys(SCMP_SYS(accept));
#if NEEDS_SOCKETCALL
			drop_sys(SCMP_SYS(socketcall));
#endif
			drop_sys(SCMP_SYS(clone));
			
			close(sock);
			
#ifdef LOCAL_TESTING
			alarm(9999);
#else
			alarm(60);
#endif
			drop_sys(SCMP_SYS(alarm));
			
			
			setresuid(euid, euid, euid); drop_sys(SCMP_SYS(setresuid));
			setresgid(egid, egid, egid); drop_sys(SCMP_SYS(setresgid));
			
			dup2(fd, STDIN_FILENO);
			dup2(fd, STDOUT_FILENO);
			drop_sys(SCMP_SYS(dup2));
			close(STDERR_FILENO);
			
			close(fd);
			
			exit(problem());
			drop_sys(SCMP_SYS(exit_group));
		}
		close(fd);
	}
	
	return -1;
}

int myprintf(char* format, ...) {
	static char buffer[4096];
	va_list ap;

	va_start(ap, format);
	int len = vsnprintf(buffer, sizeof(buffer), format, ap);
	va_end(ap);
	int i = 0;
	while(i < len) {
		int ret = write(STDOUT_FILENO, buffer, len);
		if(ret < 0) exit(-1);
		if(ret == 0) exit(0);
		i += ret;
	}
	return len;
}

char *mygets(char *str) {
	char *c = str;
	while(1) {
		int r = read(STDIN_FILENO, c, 1);
		if(r == 0) { *c = 0; return str; }
		if(r < 0) return str;
		if(*c == '\n') { *++c = 0; return str; }
		c++;
	}
}

char *contrived_open(char *filename) {
	static int done = 0;
	static char buffer[1024];
	if(done) return buffer;
	int fd = open(filename, O_RDONLY); drop_sys(SCMP_SYS(open));
	int len = read(fd, buffer, sizeof(buffer)-1);
	if(len >= 0) buffer[len] = 0;
	close(fd); drop_sys(SCMP_SYS(close));
	done = 1;
	return len >= 0 ? buffer : NULL;
}

int problem() {
	unsigned money = 0;
	bool haveBlackjack = true;
	bool haveHookers = false;
	char buffer[16];
	
	// Ehh, if I get inspired this should be replaced with a themed thingy.
	// The important parts are:
	// 1. It gives ROP upon return from problem()
	// 2. All syscalls except maybe exit are dropped before return.
	// 3. Leaks enough addresses to make rop comfy. Like, arb stack leak is nice.
	// 4. Must place ./flag in memory, not leakable within problem.
	// (maybe ./flag should be the admin password it compares to).
	myprintf("%s%c", "Welcome to black jack!", 10);
	while(1) {
		myprintf("We've got %s%s%s%s.\n", haveBlackjack ? "black jack" : "",
		     (int)haveBlackjack + (int)haveHookers == 2 ? " and " : "",
		                                    haveHookers ? "hookers" : "",
		                 !haveBlackjack && !haveHookers ? "jack shit" : "");
		if(money) myprintf("You've got $%d\n", money);
		else myprintf("You've got jack shit.\n");
		
		myprintf("\nwhu%snuduu%s", "duyugu", "butut?\n");
		
		myprintf("1. imma beat yo' ass.\n");
		myprintf("2. imma be yo' bitch.\n");
		myprintf("3. imma beimmaboobop.\n");
		myprintf("4. imma be or not 2b.\n");
		switch(atoi(mygets(buffer))) {
			case 1: if(!haveBlackjack) {
				myprintf("got no game, dis all I got:");
				int n = atoi(&buffer[1]);
				int i;
				for(i = 0; i < n; i++) {
					myprintf("%c%02x", i%16 ? ' ' : '\n', (unsigned char)buffer[i]);
				}
				myprintf("\n");
			} else {
				if(money == 0) {
					myprintf("You've got jack shit.\n");
					break;
				}
				char t = buffer[4] ^ buffer[-100] ^ buffer[-50] ^ buffer[-10];
				if(t > -10) {
					myprintf("deal with it.\n");
					money -= t;
				} else {
					myprintf("k\n");
					money -= t;
					myprintf("what u want better game, write one for me:\n");
					mygets(buffer);
				}
				if(money > 0xffffff) money = 0;
			}
				break;
			case 2: myprintf("%s\n", haveHookers ? "got one" : (haveHookers = 1, money += 10, "k")); break;
			case 3: myprintf("wololo\n"); haveHookers = !haveHookers; haveBlackjack = !haveBlackjack; money ^= (!haveBlackjack << 5)|(!haveHookers << 4); break;
			case 4: if(money < 200) myprintf("u wot m8?");
			        else { myprintf("$ $ $ %d$$> ", money -= 200); myprintf(strcmp(contrived_open("./flag.txt")+25, mygets(buffer)) ? "ring\n" : "ding\n\n"); } break;
			default: myprintf("n-n-nope\n"); {
				drop_sys(SCMP_SYS(read));
				drop_sys(SCMP_SYS(write));
				//TODO: Check that almost all the syscalls are in fact gone.
				return 0;
			}
		}
	}
}
