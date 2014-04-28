#define _BSD_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

int compar(const void *a, const void *b) {
    intptr_t x = *(intptr_t*)a;
    intptr_t y = *(intptr_t*)b;
    if(x > y) return 1;
    if(x < y) return -1;
    return 0;
}

size_t read_numbers(intptr_t *numbers) {
    char buffer[4096];
    size_t count = 0;
    
    bool prevNewline = false;
    bool hexDigits = false;
    bool shouldNegate = false;
    uintptr_t accu = 0;
    while(1) {
        ssize_t bytesread = read(0, buffer, sizeof buffer);
        assert(bytesread > 0);
        for(ssize_t i = 0; i < bytesread; i++) {
            switch(buffer[i]) {
                case '-':
                    shouldNegate = true;
                    break;
                case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                    accu *= hexDigits ? 16 : 10;
                    accu += buffer[i] - '0';
                    break;
                case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                    accu *= 16;
                    accu += buffer[i] - 'A' + 10;
                    break;
                case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                    accu *= 16;
                    accu += buffer[i] - 'a' + 10;
                    break;
                case 'x':
                    hexDigits = true;
                    break;
                case '\n':
                    if(prevNewline) return count;
                    else prevNewline = true;
                    // Fall through
                case ' ':
                case '\t':
                    hexDigits = false;
                    numbers[count] += shouldNegate ? -accu : accu;
                    accu = 0;
                    count++;
                    break;
                default:
                    assert(!"Bad input.");
            }
            if(buffer[i] != '\n') prevNewline = false;
        }
    }
}


int main(int argc, char *argv[]) {
    alarm(60);
    setlinebuf(stdout);

    intptr_t numbers[32] = {0};
    puts("Sorting as a service!");
    puts("Enter your numbers:");
    size_t n = read_numbers(numbers);
    puts("Sorting...");
    qsort(numbers, n, sizeof(*numbers), compar);
    puts("OK, here you are:");
    for(size_t i = 0; i < n; i++)
        printf("0x%0*lx\n", 2*(int)sizeof(*numbers), numbers[i]);
    puts("Thank you, come again!");
    return 0;
}
