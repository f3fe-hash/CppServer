#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <string>

#define BUFFER_SIZE 8192
#define LOG_LENGTH  16384

std::string SERVER_IP      = "192.168.1.30";    // Server IP to test
std::string SERVER_PAGE    = "/api";            // Server page to test
constexpr int SERVER_PORT  =  8080;             // Server port to test
constexpr int NUM_REQUESTS = 30000;             // Number of requests to test the server with

double time_diff(struct timeval start, struct timeval end)
{
    return (end.tv_sec - start.tv_sec) * 1000.0 + 
           (end.tv_usec - start.tv_usec) / 1000.0;
}

int main(int argc, char** argv)
{
    double total_time = 0.00;

    char log[LOG_LENGTH];

    struct timeval total_start, total_end;
    gettimeofday(&total_start, NULL);

    // Hide console cursor
    printf("\e[?25l"); 


    // --- Build HTTP Request ---
    char req[512];
    snprintf(req, 512,
             "GET %s HTTP/1.1\r\n"
             "Host: %s:%d\r\n"
             "User-Agent: stress-client\r\n"
             "Connection: close\r\n\r\n",
             SERVER_PAGE.c_str(), SERVER_IP.c_str(), SERVER_PORT);
    
    // Cast to const char* to save performance
    const char* request = (const char *)req;

    for (int i = 0; i < NUM_REQUESTS; i++)
    {
        // --- Setup Socket ---
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            perror("Socket creation failed");
            continue;
        }

        struct sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(SERVER_PORT);
        inet_pton(AF_INET, SERVER_IP.c_str(), &server.sin_addr);

        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
            //perror("Connection failed"); Don't flood console
            close(sock);
            continue;
        }

        // --- Send Request ---
        struct timeval send_start, send_end;
        gettimeofday(&send_start, NULL);

        ssize_t sent = send(sock, request, strlen(request), 0);
        if (sent < 0)
        {
            perror("Send failed");
            close(sock);
            continue;
        }

        gettimeofday(&send_end, NULL);
        double send_time = time_diff(send_start, send_end);

        // --- Receive Response ---
        struct timeval recv_start, recv_end;
        gettimeofday(&recv_start, NULL);

        char buffer[BUFFER_SIZE];
        ssize_t received;
        size_t total_received = 0;
        while ((received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0)
        {
            total_received += received;
        }

        gettimeofday(&recv_end, NULL);
        double recv_time = time_diff(recv_start, recv_end);

        close(sock);

        double request_total = send_time + recv_time;
        total_time += request_total;

        printf("[Request %d (%.2f%%, %d req/sec)]\tSend: %.2f ms,\tRecv: %.2f ms,\tTotal: %.2f ms (%zu bytes)\r",
               i + 1, (double)((double)(i + 1) / (double)NUM_REQUESTS) * 100, (int)(1000 / request_total), send_time, recv_time, request_total, total_received);
    }

    // Show console cursor
    printf("\e[?25h"); 

    gettimeofday(&total_end, NULL);
    double total_elapsed_ms = time_diff(total_start, total_end);
    double total_elapsed_sec = total_elapsed_ms / 1000.0;
    double req_per_sec = NUM_REQUESTS / total_elapsed_sec;

    printf("\n=== Total time for %d requests: %.2f ms ===\n", NUM_REQUESTS, total_time);
    printf("=== Average latency per request: %.2f ms ===\n", total_time / NUM_REQUESTS);
    printf("=== Overall throughput: %.2f requests/sec ===\n", req_per_sec);

    return 0;
}