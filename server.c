#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 9000
#define BUFFER_SIZE 1024
#define MAX_FILENAME 256

typedef struct {
    char filename[MAX_FILENAME];
    int expiry_seconds;
} FileExpiryTask;

void* delete_file_after_delay(void* arg) {
    FileExpiryTask* task = (FileExpiryTask*)arg;
    sleep(task->expiry_seconds);

    if (remove(task->filename) == 0) {
        printf("[+] File %s has been deleted after expiry\n", task->filename);
    } else {
        perror("[-] Failed to delete file");
    }

    free(task);
    return NULL;
}

void* handle_client(void* arg) {
    int client_sock = *((int*)arg);
    free(arg);

    char filename[MAX_FILENAME];
    int filename_len, file_size, expiry;

    // Receive filename length
    if (recv(client_sock, &filename_len, sizeof(int), 0) <= 0) return NULL;
    if (recv(client_sock, filename, filename_len, 0) <= 0) return NULL;
    filename[filename_len] = '\0';

    // Receive file size
    if (recv(client_sock, &file_size, sizeof(int), 0) <= 0) return NULL;

    // Receive expiry time in seconds
    if (recv(client_sock, &expiry, sizeof(int), 0) <= 0) return NULL;

    // Save the file
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        perror("[-] File open failed");
        close(client_sock);
        return NULL;
    }

    char buffer[BUFFER_SIZE];
    int bytes_received = 0;
    while (bytes_received < file_size) {
        int bytes = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (bytes <= 0) break;
        fwrite(buffer, 1, bytes, fp);
        bytes_received += bytes;
    }

    fclose(fp);
    printf("[+] Received: %s (%d bytes), expires in %d seconds\n", filename, bytes_received, expiry);

    // Schedule deletion
    FileExpiryTask* task = malloc(sizeof(FileExpiryTask));
    strcpy(task->filename, filename);
    task->expiry_seconds = expiry;

    pthread_t tid;
    pthread_create(&tid, NULL, delete_file_after_delay, (void*)task);
    pthread_detach(tid);

    close(client_sock);
    return NULL;
}
int main(){

    return 0;
}