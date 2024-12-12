#include "headers.h"
#include "defs.h"

#define MAX_BUFFER_SIZE 1024
#define MAX_SIZE 10000
#define NM_PORT 2345
#define TIMEOUT 3

char *ip = "127.0.0.1";

int wtf = 0;

void clt_to_nm(char *request, char *response, int clientSocket)
{
    bzero(response, 1024);
    // Send request to Naming Server
    // printf("sending : %s\n", request);
    if (send(clientSocket, request, strlen(request), 0) < 0)
    {
        perror("Error in sending data to nm.");
        exit(1);
    }

    // Receive response from Naming Server
    // if (recv(clientSocket, response, MAX_BUFFER_SIZE, 0) < 0)
    // {
    //     perror("Error in receiving data from nm.");
    //     exit(1);
    // }

    // printf("received first response : %s\n", response);

    // if(atoi(response) == 0){
    //     if (recv(clientSocket, response, MAX_BUFFER_SIZE, 0) < 0)
    //     {
    //         perror("Error in receiving data from nm.");
    //         exit(1);
    //     }
    // }

    int resp = 0;
    while (resp == 0)
    {
        // num = recv(clientSocket, response, MAX_BUFFER_SIZE, 0);
        if (recv(clientSocket, response, MAX_BUFFER_SIZE, 0) < 0)
        {
            perror("Error in receiving data from nm.");
            exit(1);
        }
        resp = atoi(response);
    }

    // if(wtf == 0){
    //    if (recv(clientSocket, response, MAX_BUFFER_SIZE, 0) < 0)
    //     {
    //         perror("Error in receiving data from nm.");
    //         exit(1);
    //     }
    //     wtf = 1;
    // }

    // printf("response in clt_to_nm: %s\n", response);

    // Close the socket
    // close(clientSocket);
}

int clt_to_ss(char *ssIP, int ssPort, char *request)
{
    int clientSocket;
    struct sockaddr_in serverAddr;

    // Create socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // Initialize server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(ssPort);
    inet_pton(AF_INET, ssIP, &serverAddr.sin_addr);

    // Connect to Storage Server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Connection to Storage Server failed");
        exit(EXIT_FAILURE);
    }

    send(clientSocket, request, strlen(request), 0);

    char *toks[6];
    int tokc = 0;
    char *token = (char *)(malloc(256 * sizeof(char)));
    token = strtok(request, " \t\n");
    while (token != NULL)
    {
        toks[tokc] = (char *)(malloc(256 * sizeof(char)));
        strcpy(toks[tokc++], token);
        token = strtok(NULL, " \t\n");
    }

    // if(strncmp(request, "write", 5) == 0) {
    if (strcmp(toks[0], "write") == 0)
    {
        char buffer[MAX_BUFFER_SIZE];
        int consecutiveNewlines = 0;

        // get data in chunks of MAX_BUFFER_SIZE
        while (fgets(buffer, MAX_BUFFER_SIZE - 1, stdin) != NULL)
        {
            // Check for consecutive newline characters
            if (strcmp(buffer, "\n") == 0)
            {
                consecutiveNewlines++;
            }
            else
            {
                consecutiveNewlines = 0; // Reset the counter if a non-newline character is encountered
            }
            if (consecutiveNewlines == 2)
                break;
            send(clientSocket, buffer, strlen(buffer), 0);
        }
        bzero(buffer, MAX_BUFFER_SIZE);
        sprintf(buffer, "%d", DATA_END);
        if (send(clientSocket, buffer, strlen(buffer), 0) < 0)
        {
            perror("Send");
            exit(1);
        }
    }

    // if (strcmp(toks[0], "write") == 0)
    // {
    //     char input[MAX_SIZE];
    //     fgets(input, MAX_SIZE - 1, stdin);
    //     char buffer[MAX_BUFFER_SIZE];
    //     int chunks = strlen(input) / MAX_BUFFER_SIZE;
    //     for (int i = 0; i < chunks; i++)
    //     {
    //         bzero(buffer, MAX_BUFFER_SIZE);
    //         strncpy(buffer, input + i * MAX_BUFFER_SIZE, MAX_BUFFER_SIZE);
    //         if (send(clientSocket, buffer, MAX_BUFFER_SIZE, 0))
    //         {
    //             perror("Error in sending");
    //             exit(1);
    //         }
    //     }
    //     bzero(buffer, MAX_BUFFER_SIZE);
    //     sprintf(buffer, "%d", DATA_END);
    //     if (send(clientSocket, buffer, strlen(buffer), 0))
    //     {
    //         perror("Error in sending");
    //         exit(1);
    //     }
    // }

    if (strcmp(toks[0], "read") == 0)
    {
        char buffer[MAX_BUFFER_SIZE];
        // get data in chunks of MAX_BUFFER_SIZE
        while (1)
        {
            bzero(buffer, MAX_BUFFER_SIZE);
            int bytesRead = recv(clientSocket, buffer, MAX_BUFFER_SIZE, 0);
            if (bytesRead < 0)
            {
                perror("Error in receving\n");
                exit(EXIT_FAILURE);
            }

            if (buffer[0] == UNSUCCESSFUL)
            {
                print_errors(UNSUCCESSFUL);
                exit(EXIT_FAILURE);
            }

            // printf("received: %s\n", buffer);

            char buf[5];
            sprintf(buf, "%d", DATA_END);
            if (strcmp(buffer, buf) == 0)
            {
                // printf("Received ack\n");
                break;
            }

            printf("%s", buffer);
        }
        printf("\n");
    }

    if (strcmp(toks[0], "get") == 0)
    {
        char buffer[MAX_BUFFER_SIZE];
        bzero(buffer, MAX_BUFFER_SIZE);
        int bytesRead = recv(clientSocket, buffer, MAX_BUFFER_SIZE, 0);
        if (bytesRead < 0)
        {
            perror("Error in receiving\n");
            exit(EXIT_FAILURE);
        }

        if (buffer[0] == UNSUCCESSFUL)
        {
            print_errors(UNSUCCESSFUL);
            exit(EXIT_FAILURE);
        }

        printf("%s\n", buffer);
    }

    // while (1) {
    //     char response[MAX_BUFFER_SIZE];
    //     memset(response, 0, sizeof(response));

    //     // Receive information packet
    //     int bytesRead = recv(clientSocket, response, MAX_BUFFER_SIZE, 0);
    //     if (bytesRead <= 0) {
    //         break;
    //     }

    //     printf("Received information packet from Storage Server: %s\n", response);

    //     // add condition as given in the problem statement
    // }

    // Close the socket

    char ack_received[10];
    if (recv(clientSocket, ack_received, 10, 0) < 0)
    {
        perror("Error in receiving data from nm.");
        exit(1);
    }

    int ack = atoi(ack_received);
    print_errors(ack);

    close(clientSocket);

    return ack;
}

int main()
{

    int clientSocket;
    struct sockaddr_in serverAddr;

    // Create socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    printf("Client socket created\n");

    // Initialize server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(NM_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    // Connect to Naming Server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Connection to Naming Server failed");
        exit(EXIT_FAILURE);
    }

    char ack_received[10];
    if (recv(clientSocket, ack_received, 10, 0) < 0)
    {
        perror("Error in receiving data from nm.");
        exit(1);
    }

    // implement timeout here

    // printf("ack received: %s\n", ack_received);

    int ack = atoi(ack_received);

    // printf("ack: %d\n", ack);

    if (ack == CONNECT_CLT_TO_NM)
    {
        print_errors(ack);
    }
    else
    {
        printf("Error in connecting to naming server.\n");
        close(clientSocket);
        exit(1);
    }

    while (1)
    {
        char request[MAX_BUFFER_SIZE];
        char response[MAX_BUFFER_SIZE];
        bzero(response, 1024);

        printf("Enter the request: ");
        // scanf("%[^\n]%*c", request);
        fgets(request, MAX_BUFFER_SIZE - 1, stdin);

        char temp[MAX_BUFFER_SIZE];
        strcpy(temp, request);
        
        char *toks[6];
        int tokc = 0;
        char *token = (char *)(malloc(256 * sizeof(char)));
        token = strtok(request, " \t\n");
        while (token != NULL)
        {
            toks[tokc] = (char *)(malloc(256 * sizeof(char)));
            strcpy(toks[tokc++], token);
            token = strtok(NULL, " \t\n");
        }

        // if(strncmp(request, "exit", 4) == 0) {
        if (strcmp(toks[0], "exit") == 0)
        {
            printf("Exiting...\n");
            close(clientSocket);
            break;
        }
        // reponse = "ok";
        // printf("wtf in main\n");
        clt_to_nm(temp, response, clientSocket);
        // int pri = atoi(response);
        // printf("response from nm: %s\n", response);
        // printf("response from nm: %d\n", pri);

        // if(strncmp(request, "create", 6) == 0 || strncmp(request, "delete", 6) == 0 || strncmp(request, "copy", 4) == 0) {
        //     printf("Task completed successfully\n");
        //     return 0;
        // }

        // if the response is greater than 900, then the storage port is sent
        // else a code is sent which represents error or success

        int port = atoi(response);

        // printf("port: %d\n", port);

        if (port < 900)
        {
            // handle for error/success
            print_errors(port);
            continue;
        }

        int ack_to_nm = clt_to_ss(ip, port, temp);

        // send ack to naming server
        char ack_to_nm_str[MAX_BUFFER_SIZE];
        sprintf(ack_to_nm_str, "%d", ack_to_nm);
        if (send(clientSocket, ack_to_nm_str, strlen(ack_to_nm_str), 0) < 0)
        {
            perror("Error in sending data to nm.");
            exit(1);
        }

        // printf("Task completed successfully!\n");
    }

    return 0;
}
