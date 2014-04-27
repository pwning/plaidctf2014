#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

const char *filename = "./use_exe_to_read_me.txt";

int main() {
	int fd = open(filename, O_RDONLY);
	if(fd < 0) {
		perror("open");
		printf("Must be run in same directory as %s\n", filename);
		return -1;
	}

	char buf[1024];
	ssize_t len = read(fd, buf, sizeof buf);
	if(len < 0) {
		perror("read");
		return -1;
	}

	if(write(STDOUT_FILENO, buf, len)) {
		perror("write");
		return -1;
	}

	return 0;
}

