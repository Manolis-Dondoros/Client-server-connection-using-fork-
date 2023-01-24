#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#define SIZE 20
#define LENGTH 5
#define max_price 250
#define min_price 30
#define itm_count 2
#define TRUE 1
#define FALSE 0

typedef struct item
{
    char description[15];
    int count;
    int price;
    int id;
} item;

const char *list[5] = {"Gaming", "Hardware", "Software", "Tech", "Clothing"};

int main(int argc, char const *argv[])
{
    // if (argc != 2)
    // {
    //     printf("You need to insert a valid port number!\n");
    //     exit(0);
    // }

    srand(time(NULL));
    // Catalog initialization
    printf("\t\t\t--------Catalog--------\n\n");
    item catalog[SIZE];
    for (unsigned int i = 0; i < SIZE; i++)
    {
        int index = rand() % LENGTH;
        catalog[i].id = i + 1;
        catalog[i].count = itm_count;
        catalog[i].price = rand() % (max_price - min_price + 1) + min_price;
        int desc_len = strlen(list[index]);
        strncpy(catalog[i].description, list[index], desc_len);
        catalog[i].description[desc_len] = '\0';
        printf("Item id: %d\tPrice: %d$\tCount: %d\tDescription: %s\n", catalog[i].id, catalog[i].price, catalog[i].count, catalog[i].description);
    }

    // Server Communication
    int port = atoi(argv[1]);
    int master_socket, client_handler;
    struct sockaddr_in server_address, client_addres;
    int addr_len = sizeof(struct sockaddr);

    if ((master_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        perror("Problem with master socket!");
        exit(EXIT_FAILURE);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_port = port;
    server_address.sin_addr.s_addr = INADDR_ANY;
    char request_buffer[1024];
    char answer_buffer[1024];

    if (bind(master_socket, (struct sockaddr *)&server_address, sizeof(struct sockaddr)) == -1)
    {
        perror("Binding problem!");
        exit(EXIT_FAILURE);
    }
    int succesful_orders = 0;
    int failed_orders = 0;
    int profit = 0;
    listen(master_socket, 5);

    while (1)
    {
        if (succesful_orders + failed_orders > 20)
        {
            printf("Shall we close for today[y/n]?\n");
            char ans = getchar();
            if (ans == 'y')
                break;
        }

        printf("Waiting for the client to connect...\n");
        client_handler = accept(master_socket, (struct sockaddr *)&client_addres, &addr_len);
        if (client_handler <= 0)
        {
            perror("Problem with accept!");
            exit(EXIT_FAILURE);
        }
        printf("Client %s, %u has made a succesful connection\n", inet_ntoa(client_addres.sin_addr), ntohs(client_addres.sin_port));
        while (1)
        {
            memset(request_buffer, 0, sizeof(request_buffer));
            memset(answer_buffer, 0, sizeof(answer_buffer));
            int product_id;
            if (recv(client_handler, request_buffer, sizeof(request_buffer), 0) == -1)
            {
                perror("Problem with the receiving message function!");
                exit(EXIT_FAILURE);
            }
            sscanf(request_buffer, "%d", &product_id);
            if (product_id == -1)
            {
                printf("Client exited!\n");
                break;
            }
            sleep(1);
            if (catalog[product_id].count > 0)
            {
                sprintf(answer_buffer, "%d", TRUE);
                send(client_handler, answer_buffer, strlen(answer_buffer) + 1, 0);
                catalog[product_id].count -= 1;
                profit += catalog[product_id].price;
                succesful_orders++;
            }
            else
            {
                sprintf(answer_buffer, "%d", FALSE);
                send(client_handler, answer_buffer, strlen(answer_buffer) + 1, 0);
                failed_orders++;
            }
        }
    }

    printf("Deliveries requested: %d\n", succesful_orders + failed_orders);
    printf("Successful deliveries: %d\n", succesful_orders);
    printf("Failed deliveries: %d\n", failed_orders);
    printf("Day's profit: %d\n", profit);
    close(client_handler);
    close(master_socket);
    return 0;
}
