#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h> // for close()


#define SERVER_IP "127.0.0.1"
#define PORT 9000
#define BUFFER_SIZE 4096
#define MAX_FILE_SIZE (10 * 1024 * 1024)

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <file> <server_ip> <expire_secs>\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];
    char *server_ip = argv[2];
    int expire_in = atoi(argv[3]);

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("File open failed");
        return 1;
    }

    struct stat st;
    stat(filename, &st);
    if (st.st_size > MAX_FILE_SIZE) {
        printf("ERROR: File size exceeds 10MB.\n");
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT)
    };
    inet_pton(AF_INET, server_ip, &serv_addr.sin_addr);

    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    char *base = strrchr(filename, '/');
    base = base ? base + 1 : filename;

    send(sock, base, 256, 0);

    uint32_t size_net = htonl((uint32_t)st.st_size);
    send(sock, &size_net, sizeof(size_net), 0);

    uint32_t expire_net = htonl((uint32_t)expire_in);
    send(sock, &expire_net, sizeof(expire_net), 0);

    char buffer[BUFFER_SIZE];
    size_t n;
    while ((n = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
        send(sock, buffer, n, 0);
    }

    printf("[+] File uploaded: %s (expires in %d secs)\n", base, expire_in);
    fclose(fp);
    close(sock);
    return 0;
}
