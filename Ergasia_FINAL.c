#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <time.h>

#define SOCKETNAME "/tmp/MySocket1"

struct items
{
    char description[20];
    int price;
    int item_count;
};

struct items catalog[19];

const char *list[5] = {"NFT", "CSGO_SKIN", "CLOTHING", "ELECTRONIC_DEVICE", "VINYL"};//thetoume mia lista me onomata description gia na thesoume meta to desc twn items tou catalog

int main(void)
{
    srand(time(NULL));//gia na vgazei to rand alles times kathe fora pou to trexoume
    struct sockaddr_un sa; // AF_UNIX socket address
    unlink(SOCKETNAME);    // remove existing socket file
    memset(&sa, 0, sizeof(sa));
    strncpy(sa.sun_path, SOCKETNAME, sizeof(sa.sun_path) - 1);
    sa.sun_family = AF_UNIX;
    int fd_skt, fd_client;//File descriptors

    //-------------------------------------------------------------------------------------------------------------------------------------------------------------------
    //Metavlites gia upologismous
    int failed_attempts, all_attempts, succ_attempts, totalcost_client, profit, total_item_sold;
    int failclients[] = {0, 0, 0, 0, 0};

    int item_attempts[20] = {0};

    int individual_items_sold=0;

    int i, j, item_n, item_count;

    //gia na kanoume enhmerwsh tou pelath me read() kai write()    
    char mess_succ[100];
    char mess_fail[]={"Unsuccessful order\n"};
    char buffer[100];

    //-------------------------------------------------------------------------------------------------------------------------------------------------------------------
    if (fork() == 0)//Dimiourgia diergasiwn
    { /* client */
        if ((fd_skt = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)//Thetoume ton file descriptor
        {
            perror("Message from socket [client] : ");
            exit(-1);
        }
        printf("[Client] ==> Socket %d has been created\n", fd_skt);
        // Since cliend and server run concurrently, it is possible for the client
        // to call connect before the creation of the socket file by the server (ENOENT).
        // In this case the function sleeps for a second and tries again. If something
        // else goes wrong, the function fails.

        while (connect(fd_skt, (struct sockaddr *)&sa, sizeof(sa)) == -1)
        {
            if (errno == ENOENT)
            {
                sleep(1);
                continue;
            }
            else
            {
                perror("Message from connect [client]");
                exit(-2);
            }
        }
        //-------------------------------------------------------------------------------------------------------------------------------------------------------------------
        // After connection the client and the server can communicate each other
        // using read and write functions
        printf("[Client] ==> Connection has been established .. let us write to server\n");
        printf("------------------------------------------------------------------------ORDERS-------------------------------------------------------------------------------------------\n");
        for (i = 0; i < 5; i++) // 5 pelates
        {   
            printf("------------------------------------------------------------------------CLIENT_%d-----------------------------------------------------------------------------------------\n",i+1);
            totalcost_client=0;
            for (j = 0; j < 10; j++) // 10 paragkelies
            {   
                //dimiourgia paragkelias
                item_n = rand() % 19;
                item_count = rand() % 2 + 1;
                //write to server and read the response
                write(fd_skt, &item_n, sizeof(int));
                write(fd_skt, &item_count, sizeof(int));
                read(fd_skt, buffer, sizeof(buffer));
                //print the response
                printf("%s",buffer);
                //sleep for 1 sec to simulate the time between orders
                sleep(1);
            }
            //read the total cost of all the client's orders and print it
            read(fd_skt, &totalcost_client, sizeof(int));
            printf("\nThe total cost for client %d was:%d\n\n", i + 1, totalcost_client);
        }
        //close the fd
        close(fd_skt);
        exit(0);
    }
    else//-------------------------------------------------------------------------------------------------------------------------------------------------------------------
    { /* server */
        // Initialize catalog
        for (i = 0; i < 20; i++)
        {   
            int index = rand() % 5;
            int desc_len = strlen(list[index]);
            catalog[i].item_count = 2;
            strncpy(catalog[i].description, list[index], desc_len);
            catalog[i].description[desc_len] = '\0';
            catalog[i].price = rand() % 50 + 1;
        }
        //-------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //This code handles the connection on the server side
        if ((fd_skt = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
        {
            perror("Message from socket [server] : ");
            exit(-1);
        }
        printf("[Server] ==> Socket %d has been created\n", fd_skt);

        if (bind(fd_skt, (struct sockaddr *)&sa, sizeof(sa)) < 0)
        {
            perror("Message from bind [server] : ");
            exit(-2);
        }
        printf("[Server] ==> Socket %d has been bound to address\n", fd_skt);

        if (listen(fd_skt, 5) < 0)
        {
            perror("Message from listen [server] : ");
            exit(-3);
        }
        printf("[Server] ==> Listening for incoming messages\n");

        if ((fd_client = accept(fd_skt, NULL, NULL)) < 0)
        {
            perror("Message from accept [server] : ");
            exit(-4);
        }
        printf("[Server] ==> Connection has been accepted\n\n");

        //-------------------------------------------------------------------------------------------------------------------------------------------------------------------
        all_attempts = 0;
        succ_attempts = 0;
        failed_attempts = 0;

        // Main loop
        for (i = 0; i < 5; i++)
        {
            totalcost_client = 0;
            for (j = 0; j < 10; j++) // paragkelies
            {
                read(fd_client, &item_n, sizeof(int));
                read(fd_client, &item_count, sizeof(int));
                if (catalog[item_n].item_count >= item_count)//Yparxoun ta proionta pou zitithikan
                {
                    catalog[item_n].item_count -= item_count;
                    item_attempts[item_n]++;
                    all_attempts++;
                    succ_attempts++;
                    totalcost_client += catalog[item_n].price*item_count;
                    profit += totalcost_client;
                    total_item_sold += item_count;
                    //write into the array mess_secc
                    sprintf(mess_succ, "Successful order with cost:%d\n",catalog[item_n].price*item_count);
                    write(fd_client, mess_succ, sizeof(mess_succ));//write the successful message to the client about 1 order
                }
                else//Den uparxoun ta proionta pou zitithikan
                {
                    all_attempts++;
                    failed_attempts++;
                    failclients[i] = 1;
                    item_attempts[item_n]++;
                    write(fd_client, mess_fail, sizeof(mess_fail));//write the unsuccessful message to the client about 1 order
                }
                sleep(1);
            }
            write(fd_client, &totalcost_client, sizeof(int));//send the total cost of the whole order to the client
        }
        wait(NULL);//wait for the chile proccess to finish
        //-------------------------------------------------------------------------------------------------------------------------------------------------------------------
        // Anafora
        printf("---------------------------------------------------------------------ITEM_REPORT-----------------------------------------------------------------------------------------\n\n");
        for (i = 0; i < 20; i++)
        {
            printf("Item requests: %d              ", item_attempts[i]);
            individual_items_sold = 2 - catalog[i].item_count;
            if (individual_items_sold == 0)
            {
                printf("Items sold:0              ");
            }
            else if (individual_items_sold == 1)
            {
                printf("Items sold:1              ");
            }
            else
            {
                printf("Items sold:2              ");
            }
            printf("Item %d Description: %s\n\n",i+1, catalog[i].description);

        }
        printf("----------------------------------------------------------List of clients that didnt find a product available-----------------------------------------------------------\n");
        for (i = 0; i < 5; i++)
        {
            j = i + 1;
            if (failclients[i] == 1)
            {
                printf("Client %d\n\n", j);
            }
        }
        //-------------------------------------------------------------------------------------------------------------------------------------------------------------------
        // Sigkentrotikh anafora
        printf("------------------------------------------------------------------------REPORT-------------------------------------------------------------------------------------------\n");
        printf("Total num of orders:%d\n\n", all_attempts);
        printf("Successful Orders:%d\n\n", succ_attempts);
        printf("Unsuccessful Orders:%d\n\n", failed_attempts);
        printf("Total profit:%d\n\n", profit);

        exit(0);
    }
}