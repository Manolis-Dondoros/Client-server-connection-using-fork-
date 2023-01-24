#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#define num_of_orders 10
#define num_of_clients 5

int main(int argc, char const *argv[])
{
    // if (argc != 3)
    // {
    //     printf("You need to enter server ip adress and port number!\n");
    //     exit(0);
    // }

    for (unsigned int i = 0; i < num_of_clients; i++)
    {
        srand(time(NULL) ^ (i ^ 2));
        pid_t child = fork();
        if (child < 0)
        {
            printf("Fork failed!\n");
            exit(EXIT_FAILURE);
        }
        else if (child == 0)
        {
            int socket_fd;
            int addr_len = sizeof(struct sockaddr);
            struct sockaddr_in dest_address;
            char buffer[1024];
            char received[1024];
            int port = atoi(argv[2]);

            dest_address.sin_family = AF_INET;
            dest_address.sin_port = port;
            inet_aton(argv[1], &dest_address.sin_addr);

            socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

            connect(socket_fd, (struct sockaddr *)&dest_address, sizeof(struct sockaddr));
            while (1)
            {
                int answer;
                for (unsigned int j = 0; j < num_of_orders; j++)
                {
                    memset(buffer, 0, sizeof(buffer));
                    memset(received, 0, sizeof(received));
                    int product_id = rand() % 20;
                    sprintf(buffer, "%d", product_id);
                    send(socket_fd, buffer, strlen(buffer) + 1, 0);
                    recv(socket_fd, received, sizeof(received), 0);
                    sscanf(received, "%d", &answer);
                    if (answer == 1)
                    {
                        printf("Order accepted by the server!\n");
                    }
                    else
                        printf("Order declined by the server!\n");
                    sleep(1);
                    if (j + 1 == num_of_orders)
                    {
                        memset(buffer, 0, sizeof(buffer));
                        sprintf(buffer, "%d", -1);
                        send(socket_fd, buffer, strlen(buffer) + 1, 0);
                    }
                }
                close(socket_fd);
                break;
            }
            exit(EXIT_SUCCESS);
        }
        else if (child > 0)
        {
            wait(NULL);
        }
    }

    return 0;
}
