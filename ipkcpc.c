#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[256];
    int n;

    char *host = NULL;
    char *port = NULL;
    char *mode = NULL;

    // parse command-line options
    int opt;
    while ((opt = getopt(argc, argv, "h:p:m:")) != -1) {
        switch (opt) {
            case 'h':
                host = optarg;
                break;
            case 'p':
                // check if port is a number
                if (atoi(optarg) != 0 || optarg == "0"){
                    port = optarg;
                }
                else{
                    fprintf(stderr, "Port should be an integer number.");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'm':
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

    // check if required options are present
    if (!host || !port || !mode) {
        fprintf(stderr, "Missing required options.\n");
        fprintf(stderr, "Usage: %s -h <host> -p <port> -m <mode>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

//////////
    int mode_flag = strcmp("tcp", mode);

    // set server address and port
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port));
    serv_addr.sin_addr.s_addr = inet_addr(host);
    
    //program works different depending on mode
    if (mode_flag == 0){ 

        // create socket
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            fprintf(stderr, "Error creating socket");
            exit(1);
        }

        // connect to server
        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            fprintf(stderr, "Error connecting to server.\n");
            exit(1);
        }
        printf("You are connected to server.\n");
        
        signal(SIGINT, handle_sigint);
        while (1){

            // read from input
            char message[BUFFER_SIZE] = {'\0'};
            fgets(message, BUFFER_SIZE, stdin);

            // send data to server
            if (send(sockfd, message, strlen(message), 0) < 0) {
                fprintf(stderr, "Error sending data to server.\n");
                exit(1);
            }

            // receive data from server
            memset(buffer, 0, sizeof(buffer));
            if ((n = recv(sockfd, buffer, sizeof(buffer), 0)) < 0) {
                fprintf(stderr, "Error receiving data from server.\n");
                exit(1);
            }

            printf("%s", buffer);

        }
        // close connection
        close(sockfd);
        

    }
    else{
        // create socket
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            fprintf(stderr, "Error creating socket");
            exit(1);
        }
        
        while (1){
            // read from input
            char message[BUFFER_SIZE] = {'\0'};
            fgets(message, BUFFER_SIZE, stdin);

            char binary_message[message_len * sizeof(int)];




            /////////code (make function)
            // Convert each character in the message to its binary value
            for (int i = 0; i < message_len; i++) {
                // Convert the character to a binary string
                char binary_str[9];
                sprintf(binary_str, "%08d", strtol(message[i], NULL, 2));

                // Convert the binary string to a numeric value
                int binary_val = htonl(strtol(binary_str, NULL, 2));

                // Store the binary value in the character array
                memcpy(&binary_message[i * sizeof(int)], &binary_val, sizeof(int));
            }




            // send data to server
            if (sendto(sockfd, binary_message, sizeof(binary_message), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                fprintf(stderr, "Error sending data to server.\n");
                exit(1);
            }

            // Receive data from server
            memset(buffer, 0, sizeof(buffer));
            socklen_t addr_len = sizeof(serv_addr);
            if ((n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&serv_addr, &addr_len)) < 0) {
                fprintf(stderr, "Error receiving data from server.\n");
                exit(1);
            }

            /////encode make function

            printf("%s", buffer);

            // close connection
            lose(sockfd)c;

        }

        
    }
    }







}