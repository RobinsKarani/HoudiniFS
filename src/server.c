#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <time.h>

#define PORT 9000
#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10MB
#define BUFFER_SIZE 4096
#define STORAGE_DIR "files"
#define EXPIRY_DB "expiry.db"

void *expiry_thread(void *arg);
void check_and_delete_expired_files();

void save_expiry(const char *filename, time_t expiry) {
    FILE *fp = fopen(EXPIRY_DB, "a");
    if (fp) {
        fprintf(fp, "%s %ld\n", filename, expiry);
        fclose(fp);
    }
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen = sizeof(client_addr);
    pthread_t tid;

    mkdir(STORAGE_DIR, 0777);
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_fd, 5);
    printf("[+] Server listening on port %d...\n", PORT);

    pthread_create(&tid, NULL, expiry_thread, NULL);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
        char filename[256];
        uint32_t filesize;
        uint32_t expire_in;
        FILE *fp;

        recv(client_fd, filename, sizeof(filename), 0);
        recv(client_fd, &filesize, sizeof(filesize), 0);
        filesize = ntohl(filesize);

        if (filesize > MAX_FILE_SIZE) {
            char *msg = "ERROR: File exceeds 10MB.\n";
            send(client_fd, msg, strlen(msg), 0);
            close(client_fd);
            continue;
        }

        recv(client_fd, &expire_in, sizeof(expire_in), 0);
        expire_in = ntohl(expire_in);

        char path[300];
        snprintf(path, sizeof(path), "%s/%s", STORAGE_DIR, filename);
        fp = fopen(path, "wb");

        char buffer[BUFFER_SIZE];
        int bytes, received = 0;
        while (received < filesize) {
            bytes = recv(client_fd, buffer, BUFFER_SIZE, 0);
            fwrite(buffer, 1, bytes, fp);
            received += bytes;
        }

        fclose(fp);

        time_t expiry = time(NULL) + expire_in;
        save_expiry(filename, expiry);

        printf("[+] Received: %s (%d bytes), expires in %d seconds\n", filename, filesize, expire_in);
        close(client_fd);
    }

    return 0;
}

void *expiry_thread(void *arg) {
    while (1) {
        sleep(60); // check every minute
        check_and_delete_expired_files();
    }
    return NULL;
}

void check_and_delete_expired_files() {
    FILE *fp = fopen(EXPIRY_DB, "r");
    FILE *tmp = fopen("temp.db", "w");
    if (!fp || !tmp) return;

    char filename[256];
    time_t expiry;
    time_t now = time(NULL);

    while (fscanf(fp, "%s %ld", filename, &expiry) != EOF) {
        if (expiry <= now) {
            char path[300];
            snprintf(path, sizeof(path), "%s/%s", STORAGE_DIR, filename);
            remove(path);
            printf("[-] Deleted expired file: %s\n", filename);
        } else {
            fprintf(tmp, "%s %ld\n", filename, expiry);
        }
    }

    fclose(fp);
    fclose(tmp);
    rename("temp.db", EXPIRY_DB);
}
