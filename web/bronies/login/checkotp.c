// gcc -s -m32 -fstack-protector-all -Wl,-z,relro,-z,now -O2 -o checkotp checkotp.c -lcrypto
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <openssl/sha.h>

const int kOtpPeriod = 10 * 60;
const char kLogFile[] = "/dev/null";
char g_username[16];
char g_encrypted_password[128];

void log_attempt(char *username, char *otp, int correct) {
    char buf[64];
    sprintf(buf, "%s OTP attempt: %s:%s",
            correct ? "Correct" : "Incorrect",
            username,
            otp);
    FILE *f = fopen(kLogFile, "r");
    fputs(buf, f);
    fclose(f);
}

int verify_otp(char *username, char *password, char *otp) {
    size_t password_len = strlen(password);
    memfrob(password, password_len);
    memcpy(g_encrypted_password, password, sizeof g_encrypted_password);
    g_encrypted_password[sizeof g_encrypted_password - 1] = 0;
    memset(password, 0, password_len);

    strncpy(g_username, username, sizeof g_username);
    g_username[sizeof g_username - 1] = 0;

    time_t t = time(NULL) / kOtpPeriod;
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, g_username, strlen(g_username));
    SHA256_Update(&ctx, ":", 1);
    SHA256_Update(&ctx, g_encrypted_password, strlen(g_encrypted_password));
    SHA256_Update(&ctx, ":", 1);
    SHA256_Update(&ctx, &t, sizeof t);

    unsigned char hashbuf[SHA256_DIGEST_LENGTH];
    SHA256_Final(hashbuf, &ctx);

    int login_success = 0;
    unsigned int correct_otp = *(unsigned int *) hashbuf;
    if (atoi(otp) == correct_otp % 3133337) {
        login_success = 1;
    }

    log_attempt(g_username, otp, login_success);
    return login_success;
}

int main(int argc, char **argv) {
    char *username = NULL;
    char *password = NULL;
    char *otp = NULL;

    size_t x;
    getline(&username, &x, stdin);
    getline(&password, &x, stdin);
    getline(&otp, &x, stdin);

    int login_success = verify_otp(username, password, otp);

    free(username);
    free(password);
    free(otp);

    if (login_success) {
        puts("<li>OTP correct...</li>");
        return 0;
    }

    puts("<li style=\"color: red\">OTP incorrect!</li>");
    return 1;
}
