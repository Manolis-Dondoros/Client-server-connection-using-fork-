#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define SOCKETNAME "/tmp/MySocket1"

struct items
{
    char description[20];
    int price;
    int item_count;
};

struct items catalog[20];

int main(void)
{
    struct sockaddr_un sa; // AF_UNIX socket
    unlink(SOCKETNAME);    // remove existing socket file
    memset(&sa, 0, sizeof(sa));
    strncpy(sa.sun_path, SOCKETNAME, sizeof(sa.sun_path) - 1);
    sa.sun_family = AF_UNIX;

    int fd_skt, fd_client;
    char buf[100];
    int readitems, readnum;

    int failed_attempts, all_attempts, succ_attempts, totalcost_client, profit, total_item_sold;
    int failclients[] = {0, 0, 0, 0, 0};

    int item_attempts[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int individual_items_sold;

    char buffer[65];
    int mess;

    int i, j, item_n, item_count, curritem = -1;

    if (fork() == 0)
    { /* client */
        if ((fd_skt = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
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
        // After connection the client and the server can communicate each other
        // using read and write functions
        printf("[Client] ==> Connection has been established .. let us write to server\n");
        for (i = 0; i < 5; i++) // 5 pelates
        {
            for (j = 0; j < 10; j++) // 10 paragkelies
            {
                item_n = rand() % 19 + 0;
                item_count = rand() % 2 + 1;
                write(fd_skt, &item_n, sizeof(int));
                write(fd_skt, &item_count, sizeof(int));
                read(fd_skt, &mess, sizeof(int));
                if (mess == -1)
                {
                    failclients[i] = 1;
                }
                else
                {
                    totalcost_client += mess;
                }
            }
            read(fd_skt, &mess, sizeof(int));
            printf("The total cost for client %d was:%d\n", i + 1, totalcost_client);
            totalcost_client = 0;
        }
        close(fd_skt);
        exit(0);
    }

    else
    { /* server */
        // Arxikopoihsh catalog
        for (i = 0; i < 20; i++)
        {
            catalog[i].item_count = 2;
            sprintf(catalog[i].description, "item %d", i + 1);
            catalog[i].price = rand() % 50 + 1;
        }

        if ((fd_skt = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
        {
            perror("Message from socket [server]");
            exit(-1);
        }

        printf("[Server] ==> Socket %d has been created\n", fd_skt);
        if (bind(fd_skt, (struct sockaddr *)&sa, sizeof(sa)) < 0)
        {
            perror("Message from bind [server]");
            exit(-1);
        }
        printf("[Server] ==> Socket %d has been bound to address\n", fd_skt);
        if (listen(fd_skt, 5) < 0)
        {
            perror("Message from listen [server]");
            exit(-1);
        }
        printf("[Server] ==> Listening for incoming messages\n");

        if ((fd_client = accept(fd_skt, NULL, NULL)) < 0)
        {
            perror("Message from accept [server]");
            exit(-1);
        }
        printf("[Server] ==> Connection has been accepted\n");

        // Initialize the counters
        failed_attempts = all_attempts = succ_attempts = totalcost_client = profit = total_item_sold = 0;

        for (i = 0; i < 5; i++)
        {
            // Receive the order from the client
            read(fd_client, &curritem, sizeof(int));
            read(fd_client, &item_count, sizeof(int));

            // Check if the item is in stock
            if (catalog[curritem].item_count < item_count)
            {
                // Send a failure message to the client
                write(fd_client, &totalcost_client, sizeof(int));
                failclients[i] = 1;
                failed_attempts++;
                continue;
            }

            // Update the item count and calculate the total cost
            catalog[curritem].item_count -= item_count;
            totalcost_client = catalog[curritem].price * item_count;

            // Send the total cost to the client
            write(fd_client, &totalcost_client, sizeof(int));

            // Update the counters
            succ_attempts++;
            profit += totalcost_client;
            item_attempts[curritem] += item_count;
            total_item_sold += item_count;
        }
        all_attempts = succ_attempts + failed_attempts;
        close(fd_client);
        // Anafora
        for (i = 0; i < 20; i++)
        {
            printf("Description %s\n", catalog[i].description);
            printf("Αριθμός ετοιμάτων για αγορά: %d\n", catalog[i].item_count);
            individual_items_sold = 2 - catalog[i].item_count;
            if (individual_items_sold == 2)
            {
                printf("Δεν πουληθηκε κανενα τεμάχιο\n\n");
            }
            else if (individual_items_sold == 1)
            {
                printf("Πουληθηκε 1 προιον\n\n");
            }
            else
            {
                printf("Πουληθηκαν %d προιοντα\n\n", individual_items_sold);
            }
        }
        printf("Λίστα χρηστων που δεν εξηπηρετήθηκαν :\n");
        for (i = 0; i < 5; i++)
        {
            j = i + 1;
            if (failclients[i] == 1)
            {
                printf("Χρηστης %d\n", j);
            }
        }
        printf("n");
        //-------------------------------------------------------------------------------------------------------------------------------------------------------------------
        // Sigkentrotikh anafora
        printf("Συνολικός αριθμός παραγγελιών:%d\n", all_attempts);
        printf("Συνολικός αριθμός επιτυχημένων παραγγελιών:%d\n", succ_attempts);
        printf("Συνολικός αριθμός αποτυχημένων παραγγελιών:%d\n ", failed_attempts);
        printf("Συνολικός τζίρος %d/n", profit);
        close(fd_skt);
        exit(0);
    }
}
