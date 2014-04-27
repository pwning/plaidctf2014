#include <assert.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>  
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <memory>

const char kAllowedOrigin[] = "http://portal.essolutions.largestctf.com";

ssize_t recvlen(int fd, char *buf, size_t n) {
    ssize_t rc;
    size_t nread = 0;
    while (nread < n) {
        rc = recv(fd, buf + nread, n - nread, 0);
        if (rc == -1) {
            if (errno == EAGAIN || errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (rc == 0) {
            break;
        }
        nread += rc;
    }
    return nread;
}

ssize_t sendlen(int fd, const char *buf, size_t n) {
    ssize_t rc;
    size_t nsent = 0;
    while (nsent < n) {
        rc = send(fd, buf + nsent, n - nsent, 0);
        if (rc == -1) {
            if (errno == EAGAIN || errno == EINTR) {
                continue;
            }
            return -1;
        }
        nsent += rc;
    }
    return nsent;
}

ssize_t sendstr(int fd, const char *str) {
    return sendlen(fd, str, strlen(str));
}

std::string urldecode(const std::string &s) {
    std::string decoded;

    size_t i = 0;
    while (i < s.length()) {
        if (s[i] != '%') {
            decoded += s[i];
            ++i;
            continue;
        }

        ++i;

        if (i + 1 >= s.length()) {
            break;
        }

        int c = 0;
        sscanf(&s[i], "%02x", &c);
        decoded += c;

        i += 2;
    }

    return decoded;
}

bool debug = false;
uint16_t port = 80;
const size_t kMaxRequestSize = 4096;
enum HttpMethod {
    HTTP_GET,
    HTTP_POST,
};

enum HttpCode {
    HTTP_OK = 200,
    HTTP_NOT_FOUND = 404,
    HTTP_BAD_REQUEST = 400,
};

const char kHttpOkStr[] = "OK";
const char kHttpNotFound[] = "Not Found";
const char kHttpBadRequest[] = "Bad Request";

struct HashEntry {
    HashEntry(const std::string &key) : key(key) {}
    ~HashEntry() {}
    std::string value;
    std::string key;
    HashEntry *next;
};

class HashFunction {
    public:
        virtual ~HashFunction() {}
        virtual const char *name() = 0;
        virtual size_t operator()(const char *) = 0;
};

class PHPHash : public HashFunction {
    public:
        PHPHash() {}
        virtual ~PHPHash() {}
        virtual const char *name() { return "PHPHash"; }
        virtual size_t operator()(const char *key) {
            size_t length = 0;
            for (; *key != '\0'; ++key, ++length) {}
            return length;
        }
};

const size_t kHashBuckets = 257;
class HashTable {
    public:
        HashTable() : size_(0), hash_function_(new PHPHash) {}
        ~HashTable() {
            Clear();
        }

        void Insert(const std::string &key, const std::string &value) {
            HashEntry *entry = Lookup(key);
            if (entry == nullptr) {
                entry = new HashEntry(key);
                size_t bucket = Hash(key);
                entry->next = buckets_[bucket];
                buckets_[bucket] = entry;
                ++size_;
            }
            entry->value = value;
        }

        HashEntry *Lookup(const std::string &key) const {
            if (size_ == 0) {
                return nullptr;
            }
            size_t bucket = Hash(key);
            for (HashEntry *entry = buckets_[bucket];
                 entry != nullptr;
                 entry = entry->next) {
                if (entry->key == key) {
                    return entry;
                }
            }
            return nullptr;
        }

        bool Delete(const std::string &key) {
            size_t bucket = Hash(key);
            HashEntry **entryp = &buckets_[bucket];
            while (*entryp != nullptr) {
                HashEntry *entry = *entryp;
                if (entry->key == key) {
                    *entryp = entry->next;
                    delete entry;
                    --size_;
                    return true;
                }

                entryp = &entry->next;
            }

            return false;
        }

        void Clear() {
            for (size_t i = 0; i < kHashBuckets; ++i) {
                HashEntry *entry = buckets_[i];
                while (entry != nullptr) {
                    HashEntry *next = entry->next;
                    delete entry;
                    entry = next;
                }
                buckets_[i] = nullptr;
            }
        }

        void set_hash_function(std::unique_ptr<HashFunction> hash_function) {
            hash_function_ = std::move(hash_function);
        }

    private:
        size_t Hash(const std::string &key) const {
            assert(hash_function_ != nullptr);
            return (*hash_function_)(key.c_str());
        }

        size_t size_;
        HashEntry *buckets_[kHashBuckets] = {0};
        std::unique_ptr<HashFunction> hash_function_;
};

class HttpResponse {
    public:
        HttpResponse() : code_(HTTP_OK), http_code_str_(kHttpOkStr) {}
        ~HttpResponse() {}

        void SendTo(int fd) {
            dprintf(fd, "HTTP/1.1 %d %s\r\n", code_, http_code_str_);
            for (const auto &header : headers_) {
                dprintf(fd, "%s: %s\r\n", header.first.c_str(), header.second.c_str());
            }
            dprintf(fd, "Content-Length: %lu\r\n\r\n", buf_.size());
            sendlen(fd, buf_.data(), buf_.size());
        }

        void set_code(HttpCode code) { code_ = code; }
        void set_code_str(const char *http_code_str) { http_code_str_ = http_code_str; }

        void set_header(const std::string &name, const std::string &value) {
            headers_[name] = value;
        }

        void set_data(const std::string &data) {
            buf_ = data;
        }

        void set_data(const char* data) {
            buf_ = data;
        }

        void set_data(const char* data, size_t n) {
            buf_ = std::string(data, n);
        }

    private:
        HttpCode code_;
        const char *http_code_str_;
        std::map<std::string, std::string> headers_;
        std::string buf_;
};

class HttpRequest {
    public:
        HttpRequest()
            : buf_(new char[kMaxRequestSize]),
              length_(0) {}
        ~HttpRequest() {}

        bool ReadFrom(int fd) {
            size_t i = 0;
            char *buf = buf_.get();
            while (i < kMaxRequestSize && strstr(buf, "\r\n\r\n") == nullptr) {
                ssize_t rc = recv(fd, buf + i, kMaxRequestSize - i, 0);
                if (rc == -1) {
                    if (errno == EAGAIN || errno == EINTR) {
                        continue;
                    }
                    return -1;
                }
                i += rc;
            }
            length_ = i;

            char *end = strstr(buf, "\r\n\r\n");
            if (end == nullptr) {
                return false;
            }

            *end = '\0';

            return true;
        }

        bool Parse() {
            char *ptr = buf_.get();
            char *request_line = strsep(&ptr, "\r");
            if (!ParseRequestLine(request_line)) {
                return false;
            }

            char *headers_start = ptr;
            if (*headers_start == '\n') {
                *headers_start++ = '\0';
            }
            if (!ParseHeaders(headers_start)) {
                return false;
            }

            if (!ParseUri()) {
                return false;
            }

            if (!ParseQueryString()) {
                return false;
            }

            return true;
        }

        bool ParseRequestLine(char *request_line) {
            char *buf = request_line;
            char *method;
            char *request_uri;
            char *http_version;

            method = strsep(&buf, " ");
            uri_ = strsep(&buf, " ");
            http_version = buf;

            if (strcmp(method, "GET") == 0) {
                method_ = HTTP_GET;
            } else {
                return false;
            }

            if (strcmp(http_version, "HTTP/1.0") != 0 &&
                strcmp(http_version, "HTTP/1.1") != 0) {
                return false;
            }

            return true;
        }

        bool ParseHeaders(char *headers_start) {
            char *buf = headers_start;

            while (buf != nullptr) {
                char *line = strsep(&buf, "\r");
                if (*line == '\n') {
                    ++line;
                }

                std::string name = strsep(&line, ":");
                if (line == NULL) {
                    return false;
                }
                while (*line == ' ') { ++line; }
                std::string value(line);

                headers_[name] = value;
            }

            return true;

        }

        bool ParseUri() {
            char *buf = &uri_[0];
            path_ = strsep(&buf, "?");
            if (buf != NULL) {
                query_string_ = buf;
            }
            return true;
        }

        bool ParseQueryString() {
            char *query_string = &query_string_[0];
            while (query_string != nullptr) {
                char *item = strsep(&query_string, "&");
                std::string key = strsep(&item, "=");
                std::string value;
                if (item != nullptr) {
                    value = item;
                }

                key = urldecode(key);
                value = urldecode(value);
                get_vars_.Insert(key, value);
            }
            return true;
        }

        const std::string *header(const std::string &name) const {
            const auto it = headers_.find(name);
            if (it == headers_.end()) {
                return nullptr;
            }
            return &it->second;
        }

        const std::string &path() const { return path_; }
        const std::string &query_string() const { return query_string_; }
        const HashTable &get_vars() const { return get_vars_; }
    private:
        std::unique_ptr<char[]> buf_;
        size_t length_;

        std::string uri_;
        std::string path_;
        std::string query_string_;
        HttpMethod method_;
        std::map<std::string, std::string> headers_;
        HashTable get_vars_;
        //Thing x_;
};

void NotFoundHandler(const std::string &path,
                     const HttpRequest &request,
                     HttpResponse *response) {
    response->set_code(HTTP_NOT_FOUND);
    response->set_code_str(kHttpNotFound);
    response->set_header("Content-Type", "text/plain");
    response->set_data("Not found.");
}

void BadRequestHandler(const std::string &path,
                       const HttpRequest &request,
                       HttpResponse *response) {
    response->set_code(HTTP_BAD_REQUEST);
    response->set_code_str(kHttpBadRequest);
    response->set_header("Content-Type", "text/plain");
    response->set_data("Bad request.");
}

void MaybeAddCORSHeader(const std::string &path,
                        const HttpRequest &request,
                        HttpResponse *response) {
    const std::string *origin = request.header("Origin");
    if (origin == nullptr) {
        return;
    }

    if (*origin == kAllowedOrigin) {
        response->set_header("Access-Control-Allow-Origin", *origin);
    }

    if (debug) {
        response->set_header("Access-Control-Allow-Origin", "*");
    }
}

void IndexHandler(const std::string &path,
                  const HttpRequest &request,
                  HttpResponse *response) {
    MaybeAddCORSHeader(path, request, response);
    response->set_header("Content-Type", "text/html");

    const HashEntry *entry = request.get_vars().Lookup("file");
    if (entry == nullptr) {
        BadRequestHandler(path, request, response);
        return;
    }

    const std::string &filename = entry->value;
    std::ifstream file(filename, std::ios::in);
    if (!file) {
        NotFoundHandler(path, request, response);
        return;
    }

    std::string file_contents;
    while (true) {
        int c = file.get();
        if (c == -1 || file.eof()) {
            break;
        }
        file_contents += c;
    }
    response->set_data(file_contents);
}

void StatusHandler(const std::string &path,
                  const HttpRequest &request,
                  HttpResponse *response) {
    MaybeAddCORSHeader(path, request, response);
    response->set_header("Content-Type", "application/json");
    response->set_data("{\"status\": \"ok\"}");
}

class Dispatcher {
    public:
        typedef void(*RequestHandler)(const std::string &,
                                      const HttpRequest &,
                                      HttpResponse *);

        Dispatcher() : default_handler_(&NotFoundHandler) {
            AddHandler("/index", &IndexHandler);
            AddHandler("/status", &StatusHandler);
        }
        ~Dispatcher() {}

        void Dispatch(int fd) const {
            HttpRequest request;
            HttpResponse response;
            RequestHandler handler = nullptr;

            if (!request.ReadFrom(fd)) {
                return;
            }
            if (!request.Parse()) {
                handler = &BadRequestHandler;
            }

            const std::string &path = request.path();
            if (handler == nullptr) {
                const auto it = handlers_.find(path);
                if (it == handlers_.end()) {
                    handler = default_handler_;
                } else {
                    handler = it->second;
                }
            }

            handler(path, request, &response);

            response.SendTo(fd);
        }

        void AddHandler(const std::string &path, RequestHandler handler) {
            handlers_[path] = handler;
        }

        void SetDefaultHandler(RequestHandler handler) {
            default_handler_ = handler;
        }
    private:
        std::map<std::string, RequestHandler> handlers_;
        RequestHandler default_handler_;
};

Dispatcher dispatcher;

int drop_privs(const char *username) {
    if (debug) {
        return 0;
    }

    struct passwd *pw = getpwnam(username);
    if (pw == nullptr) {
        fprintf(stderr, "User %s not found\n", username);
        return -1;
    }

    if (chdir(pw->pw_dir) != 0) {
        perror("chdir");
        return -1;
    }

    if (setgroups(0, nullptr) != 0) {
        perror("setgroups");
        return -1;
    }

    if (setgid(pw->pw_gid) != 0) {
        perror("setgid");
        return -1;
    }

    if (setuid(pw->pw_uid) != 0) {
        perror("setuid");
        return -1;
    }

    return 0;
}

void handle(int fd) {
    dispatcher.Dispatch(fd);
}

int main(int argc, char **argv) {
    int rc;
    int opt;
    int sockfd;
    int clientfd;
    pid_t pid;
    struct sockaddr_in saddr = {0};

    if (argc == 2 && strcmp(argv[1], "debug") == 0) {
        port = 8080;
        debug = true;
    }

    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
        fputs("Failed to set SIGCHLD handler.", stderr);
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1) {
        perror("socket");
        return 1;
    }

    opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
        perror("setsockopt");
        return 1;
    }

    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *) &saddr, sizeof(saddr)) != 0) {
        perror("bind");
        return 1;
    }

    if (listen(sockfd, 20) != 0) {
        perror("listen");
        return 1;
    }

    while (1) {
        clientfd = accept(sockfd, nullptr, nullptr);
        if (clientfd == -1) {
            perror("accept");
            continue;
        }

        pid = fork();
        if (pid == -1) {
            perror("fork");
            close(clientfd);
            continue;
        }

        if (pid == 0) {
            close(sockfd);
            rc = drop_privs("bigson");
            if (rc == 0) {
                handle(clientfd);
            }
            close(clientfd);
            exit(rc);
        }

        close(clientfd);
    }

    return 0;
}
