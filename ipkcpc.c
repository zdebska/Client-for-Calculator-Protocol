#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

#ifdef _WIN32
    #define OS "Windows"
#elif __linux__
    #define OS "Linux"
#else
    #define OS "Unknown"
#endif

#define BUFFER_SIZE 1024

int sockfd = 0;

void handle_sigint_TCP(int sig) {

    char buffer[256];
    int n;

    // send data to server
    printf("\nBYE\n");
    if (send(sockfd, "BYE\n", 4, 0) < 0) {
        close(sockfd);
        fprintf(stderr, "Error sending data to server.\n");
        exit(EXIT_FAILURE);
    }

    // receive data from server
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

void handle_sigint_UDP(int sig) {
    close(sockfd);
    exit(EXIT_FAILURE);
}

void message_to_binary(char* message, char* binary_message, int* bin_msg_len){
    int opcode = 0;
    *bin_msg_len = sizeof(message) + 1; //ending character

    memcpy(&binary_message[0], &opcode, sizeof(opcode));
    memcpy(&binary_message[1], &bin_msg_len, sizeof(bin_msg_len));
    memcpy(&binary_message[2], message, sizeof(message) + 1);
}

void binary_to_message(char* buffer, char* reseived_message, char* status_code){
    int opcode = buffer[0];
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

    // if (strcmp("Windows", OS) ||  strcmp("Unknown", OS)){
    //     fprintf(stderr, "This program is available only on Linux based OS.");
    //     exit(EXIT_FAILURE);
    // }

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
        printf("You are connected to the server.\n");
        
        signal(SIGINT, handle_sigint_TCP);
        while (1){
            // read from input
            char message[BUFFER_SIZE] = {'\0'};

            fgets(message, BUFFER_SIZE, stdin);

            // send data to server
            if (send(sockfd, message, strlen(message), 0) < 0) {
                close(sockfd);
                fprintf(stderr, "Error sending data to server.\n");
                exit(EXIT_FAILURE);
            }

            // receive data from server
            memset(buffer, 0, sizeof(buffer));
            if ((n = recv(sockfd, buffer, sizeof(buffer), 0)) < 0) {
                close(sockfd);
                fprintf(stderr, "Error receiving data from server.\n");
                exit(EXIT_FAILURE);
            }

            printf("%s", buffer);

            if (strcmp("BYE\n", buffer) == 0){
                printf("You are disconnected from the server.\n");
                close(sockfd);
                exit(EXIT_FAILURE);
            }

        }

        // close connection
        close(sockfd);
        

    }
    else{
        //create socket
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            fprintf(stderr, "Error creating socket");
            exit(1);
        }

        signal(SIGINT, handle_sigint_UDP);
        while (1){
            // read from input
            char message[BUFFER_SIZE] = {'\0'};
            fgets(message, BUFFER_SIZE, stdin);
            int bin_msg_len;
            char binary_message[BUFFER_SIZE + 3] = {'\0'};

            message_to_binary(message, binary_message, &bin_msg_len);
            
            // send data to server
            if (sendto(sockfd, binary_message, bin_msg_len, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                fprintf(stderr, "Error sending data to server.\n");
                close(sockfd);
                exit(EXIT_FAILURE);
            }

            // Receive data from server
            memset(buffer, 0, sizeof(buffer));
            socklen_t addr_len = sizeof(serv_addr);
            if ((n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&serv_addr, &addr_len)) < 0) {
                fprintf(stderr, "Error receiving data from server.\n");
                close(sockfd);
                exit(EXIT_FAILURE);
            }

            char reseived_message[BUFFER_SIZE] = {'\0'};
            char status_code;
            binary_to_message(buffer, reseived_message, &status_code);

            switch(status_code){
                case '0':
                    printf("OK:%s\n", reseived_message);
                    break;
                case '1':
                    printf("ERR:%s\n", reseived_message);
                    break;
                default:
                    printf("ERR:%s, %s\n", reseived_message, &status_code);
                    fprintf(stderr, "Error status code from server.\n");
                    close(sockfd);
                    exit(EXIT_FAILURE);
                
            }
            
        }

        // close connection
        close(sockfd);

    }
    


}