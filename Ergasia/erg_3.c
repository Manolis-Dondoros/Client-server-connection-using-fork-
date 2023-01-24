#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

#define SOCKETNAME "/tmp/MySocket1"

struct items
{
    const char *description;
    int price;
    int item_count;
};

struct items catalog[19];

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

    int p = fork(); // create child proccess

    if (p < 0) // If fork fails
    {
        fprintf(stderr, "fork Failed");
        return 1;
    }
    else if (p = 0) // child proccess
    {

        //-------------------------------------------------------------------------------------------------------------------------------------------------------------------
        // δημιουργείται το passive socket τύπου UNIX (PF_UNIX) για επικοινωνία
        // με σύνδεση (SOCK_STREAM) στο οποίο ο διακομιστής αναμένει συνδέσεις πελατών.
        if ((fd_skt = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
        {
            perror("Message from socket [client] : ");
            exit(-1);
        }
        printf("[Client] ==> Socket %d has been created\n", fd_skt);

        // Since cliend and server run concurrently, it is possible for the client
        // to call connect before the creation of the socket file by the server (ENOENT).
        // In this case the function sleeps for a second and tries again. If something
        // else goes wrong, the function fails
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
        printf("[Client] ==> Connection has been established .. let us write to server\n");
        //-------------------------------------------------------------------------------------------------------------------------------------------------------------------
        // Apo edw kai pera grafoume kwdika kai ton stelnoume ston server (Allazoume apo edw kai katw)
        //
        for (size_t i = 0; i < 5; i++) // 10 pelates
        {
            for (j = 0; j < 10; j++) // 5 paragkelies
            {
                item_n = rand() % 19 + 0;
                item_count = rand() % 2 + 1;
                write(fd_skt, &item_n, sizeof(int));
                write(fd_skt, &item_count, sizeof(int));
                sleep(1);
            }
            read(fd_skt, &mess, sizeof(int));
            printf("The total cost for client %d was:%d", j, mess);
        }
        close(fd_skt);
        exit(0);
    }
    else // parent proccess//-------------------------------------------------------------------------------------------------------------------------------------------------------------------
    {
        // Arxikopoihsh catalog
        for (i = 0; i < 20; i++)
        {
            catalog[i].item_count = 2;
            catalog[i].description = "item";
            catalog[i].price = rand() % 50 + 1;
            // strcat(i+1, catalog[i].description);
        }
        //-------------------------------------------------------------------------------------------------------------------------------------------------------------------
        // δημιουργείται το passive socket
        if ((fd_skt = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
        {
            perror("Message from socket [server]");
            exit(-1);
        }

        printf("[Server] ==> Socket %d has been created\n", fd_skt);
        // bind the address of the server and client
        if (bind(fd_skt, (struct sockaddr *)&sa, sizeof(sa)))
        {
            perror("Message from bind [server]");
            exit(-2);
        }
        printf("[Server] ==> Socket %d has been bound to address\n", fd_skt);

        if (listen(fd_skt, 5))
        {
            perror("Message from listen [server]");
            exit(-3);
        }

        printf("[Server] ==> Listening for incoming messages\n");

        if ((fd_client = accept(fd_skt, NULL, 0)) < 0)
        {
            perror("Message from accept [server]");
            exit(-4);
        }
        printf("[Server] ==> let us read from client via socket %d\n", fd_client);

        //-------------------------------------------------------------------------------------------------------------------------------------------------------------------
        for (i = 0; i < 5; i++) // 10 pelates
        {
            totalcost_client = 0;
            for (j = 0; j < 10; j++) // paragkelies
            {
                readitems = read(fd_client, &item_n, sizeof(int));
                readnum = read(fd_client, &item_count, sizeof(int));

                if (catalog[readitems].item_count < readnum)
                {
                    if (write(fd_client, "Not enough items", 9) == -1)
                    {
                        perror("Message from write [server]");
                        exit(-6);
                    }
                    failed_attempts++;
                    all_attempts++;
                    failclients[i] = 1;
                    item_attempts[readitems]++;
                }
                else
                {
                    catalog[readitems].item_count -= readnum;
                    total_item_sold += readnum;
                    profit += catalog[readitems].price;
                    totalcost_client += catalog[readitems].price;
                    succ_attempts++;
                    item_attempts[readitems]++;
                }
                sleep(1);
            }
            write(fd_client, &totalcost_client, sizeof(int));
        }
        //-------------------------------------------------------------------------------------------------------------------------------------------------------------------
        close(fd_skt);
        close(fd_client);
        //-------------------------------------------------------------------------------------------------------------------------------------------------------------------
        // Anafora
        for (i = 0; i < 20; i++)
        {
            printf("Item %d description %s\n", i, catalog[i].description);
            printf("Αριθμός ετοιμάτων για αγορά: %d\n", item_attempts[i]);
            individual_items_sold = 2 - catalog[i].item_count;
            if (individual_items_sold = 2){
                printf("Δεν πουληθηκε κανενα τεμάχιο\n\n");
            }else if(individual_items_sold = 1){
                printf("Πουληθηκε 1 προιον\n\n");
            }else{
                printf("Πουληθηκαν 2 προιοντα\n\n");
            }
        }
        printf("Λίστα χρηστων που δεν εξηπηρετήθηκαν :\n");
        for (i = 0; i < 5; i++)
        {
            j = i + 1;
            if (failclients[i] = 1)
            {
            printf("Χρηστης %d\n", j);
            }
        }
        printf("n");
        //-------------------------------------------------------------------------------------------------------------------------------------------------------------------
        //Sigkentrotikh anafora
        printf("Συνολικός αριθμός παραγγελιών:%d/n",all_attempts);
        printf("Συνολικός αριθμός επιτυχημένων παραγγελιών:%d/n",succ_attempts);
        printf("Συνολικός αριθμός αποτυχημένων παραγγελιών:%d/n ",failed_attempts);
        printf("Συνολικός τζίρος%d/n",profit);

        exit(0);
    }
}