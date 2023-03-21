/** @file ipkcpc.c
 *  @brief A client for the IPK Calculator Protocol.
 *
 *  The client should be able to communicate with any server using IPKCP.
 *
 *  @author Zdebska Kateryna (xzdebs00)
 *  @copyright GNU GENERAL PUBLIC LICENSE v3.0
 */

/* -- Includes -- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>

#ifdef _WIN32
    #define OS "Windows"
#elif __linux__
    #define OS "Linux"
#else
    #define OS "Unknown"
#endif

#define BUFFER_SIZE 1024
#define TIMEOUT_SEC 5

int sockfd = 0;

/**
 * @brief Close connection after pressing Ctrl+C and send closing request according to TCP
 *
 * @param sig signal
 */
void handle_sigint_TCP() {

    char buffer[256];
    int n;

    // Send data to server
    printf("\nBYE\n");
    if (send(sockfd, "BYE\n", 4, 0) < 0) {
        close(sockfd);
        fprintf(stderr, "Error sending data to server.\n");
        exit(EXIT_FAILURE);
    }

    // Receive data from server
    memset(buffer, 0, sizeof(buffer));
    if ((n = recv(sockfd, buffer, sizeof(buffer), 0)) < 0) {
        close(sockfd);
        fprintf(stderr, "Error receiving data from server.\n");
        exit(EXIT_FAILURE);
    }

    printf("%s", buffer);
    close(sockfd);
    printf("You are disconnected from the server.\n");
    exit(EXIT_SUCCESS);
}

/**
 * @brief Close connection after pressing Ctrl+C according to UDP 
 *
 * @param sig signal
 */
void handle_sigint_UDP() {
    close(sockfd);
    exit(EXIT_FAILURE);
}

/**
 * @brief Encodes message from user for UDP request and stores it and it`s length
 *
 * @param message user's message
 * @param binary_message encoded message
 * @param bin_msg_len length encoded message
 */
void message_to_binary(char* message, char* binary_message, int* bin_msg_len){
    int opcode = 0;
    *bin_msg_len = strlen(message) + 1; //ending character

    memcpy(&binary_message[0], &opcode, sizeof(opcode));
    memcpy(&binary_message[1], bin_msg_len, sizeof(*bin_msg_len));
    memcpy(&binary_message[2], message, strlen(message) + 1);
}

/**
 *@brief Decodes udp message from server and stores it and status code
 *
 * @param buffer message from the server
 * @param reseived_message decoded message
 * @param status_code request status
 */
void binary_to_message(char* buffer, char* reseived_message, char* status_code){
    //int opcode = buffer[0];
    *status_code = buffer[1];
    int message_size = buffer[2];

    memcpy(reseived_message, &buffer[3], message_size);
    reseived_message[message_size] = '\0';
}

int main(int argc, char *argv[]) {
    struct sockaddr_in serv_addr;
    char buffer[256];
    int n;

    char *host = NULL;
    char *port = NULL;
    char *mode = NULL;

    // Parse command-line options
    int opt;
    while ((opt = getopt(argc, argv, "h:p:m:")) != -1) {
        switch (opt) {
            case 'h':
                host = optarg;
                break;
            case 'p':
                // Check if port is a number
                if (atoi(optarg) != 0 || strcmp(optarg, "0") == 0){
                    port = optarg;
                }
                else{
                    fprintf(stderr, "Port should be an integer number.");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'm':
                // Check if mode is valid
                if (strcmp("tcp", optarg) != 0 && strcmp("udp", optarg) != 0){
                    fprintf(stderr, "Mode should be \"tcp\" or \"udp\".");
                    exit(EXIT_FAILURE);
                }
                mode = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s -h <host> -p <port> -m <mode>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Check for system compatybility 
    if (strcmp("Windows", OS) == 0 ||  strcmp("Unknown", OS) == 0){
        fprintf(stderr, "This program is available only on Linux based OS.");
        exit(EXIT_FAILURE);
    }

    // Check if required options are present
    if (!host || !port || !mode) {
        fprintf(stderr, "Missing required options.\n");
        fprintf(stderr, "Usage: %s -h <host> -p <port> -m <mode>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Set server address and port
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port));
    serv_addr.sin_addr.s_addr = inet_addr(host);
    
    // Program works different depending on mode
    // TCP mode
    if (strcmp("tcp", mode) == 0){ 
        // Create socket
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            fprintf(stderr, "Error creating socket");
            exit(EXIT_FAILURE);
        }

        // Connect to server
        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            fprintf(stderr, "Error connecting to server.\n");
            exit(EXIT_FAILURE);
        }
        printf("You are connected to the server.\n");
        
        // Checks for keystrokes Ctrl+C
        signal(SIGINT, handle_sigint_TCP);
        while (1){
            // Read from input
            char message[BUFFER_SIZE] = {'\0'};
            char* ptr = fgets(message, BUFFER_SIZE, stdin);
            if (ptr == NULL){
                fprintf(stderr, "Error reading data from input.\n");
                close(sockfd);
                exit(EXIT_FAILURE);
            }

            // Send data to server
            if (send(sockfd, message, strlen(message), 0) < 0) {
                close(sockfd);
                fprintf(stderr, "Error sending data to server.\n");
                exit(EXIT_FAILURE);
            }

            // Receive data from server
            memset(buffer, 0, sizeof(buffer));
            if ((n = recv(sockfd, buffer, sizeof(buffer), 0)) < 0) {
                close(sockfd);
                fprintf(stderr, "Error receiving data from server.\n");
                exit(EXIT_FAILURE);
            }

            printf("%s", buffer);

            // Check if the connection should be closed due to "BUY" from user
            if (strcmp("BYE\n", buffer) == 0){
                printf("You are disconnected from the server.\n");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
        }
        // Close connection
        close(sockfd);
    }
    // UDP mode
    else{
        // Structure for timeout
        struct timeval tv;

        // Create socket
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            fprintf(stderr, "Error creating socket");
            exit(EXIT_FAILURE);
        }

        // Checks for keystrokes Ctrl+C
        signal(SIGINT, handle_sigint_UDP);
        while (1){
            // Read from input
            char message[BUFFER_SIZE] = {'\0'};
            char* ptr = fgets(message, BUFFER_SIZE, stdin);
            if (ptr == NULL){
                fprintf(stderr, "Error reading data from input.\n");
                close(sockfd);
                exit(EXIT_FAILURE);
            }

            // Encode message 
            int bin_msg_len;
            char binary_message[BUFFER_SIZE + 3] = {'\0'};
            message_to_binary(message, binary_message, &bin_msg_len);

            // Send data to server
            if (sendto(sockfd, binary_message, bin_msg_len, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                fprintf(stderr, "Error sending data to server.\n");
                close(sockfd);
                exit(EXIT_FAILURE);
            }

            // Set timeout for socket receive
            tv.tv_sec = TIMEOUT_SEC;
            tv.tv_usec = 0;
            if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv))<0){
                fprintf(stderr, "Timeout.\n");
                close(sockfd);
                exit(EXIT_FAILURE);
            }

            // Receive data from server
            memset(buffer, 0, sizeof(buffer));
            socklen_t addr_len = sizeof(serv_addr);
            if ((n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&serv_addr, &addr_len)) < 0 && errno == EAGAIN) {
                fprintf(stderr, "Timeout.\n");
                close(sockfd);
                exit(EXIT_FAILURE);
            }

            // Decode message
            char reseived_message[BUFFER_SIZE] = {'\0'};
            char status_code;
            binary_to_message(buffer, reseived_message, &status_code);

            // Print message according to the status
            switch((int)status_code){
                case 0:
                    printf("OK:%s\n", reseived_message);
                    break;
                case 1:
                    printf("ERR:%s\n", reseived_message);
                    break;
                default:
                    fprintf(stderr, "ERR:%s\n", reseived_message);
                    fprintf(stderr, "Error status code:%d from server.\n", (int)status_code);
                    close(sockfd);
                    exit(EXIT_FAILURE);     
            }    
        }
        // Close connection
        close(sockfd);
    }
}