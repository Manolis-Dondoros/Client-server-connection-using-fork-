#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define SOCKETNAME "2345"

int main(void)
{
    struct sockaddr_un sa;
    (void)unlink(SOCKETNAME);
    strcpy(sa.sun_path, SOCKETNAME);
    sa.sun_family = AF_UNIX;
    int fd_skt, fd_client;
    char buf[100];
    int written;
    ssize_t readb;

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
        written = write(fd_skt, "Hello!", 7);
        if (written == -1)
        {
            perror("Message from write [client]");
            exit(-3);
        }
        else
            printf("[Client] ==> %d bytes written to server\n", written);
        printf("[Client] ==> Now let us read from server\n");
        readb = read(fd_skt, buf, sizeof(buf));
        if (readb == -1)
        {
            perror("Message from read [client]");
            exit(-4);
        }
        else
            printf("[Client] ==> %zd bytes read from server\n", readb);
        printf("Client got %s\n", buf);
        close(fd_skt);
        exit(0);
    }

    else
    { /* server */

        if ((fd_skt = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
        {
            perror("Message from socket [server]");
            exit(-1);
        }

        printf("[Server] ==> Socket %d has been created\n", fd_skt);
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
        readb = read(fd_client, buf, sizeof(buf));
        if (readb == -1)
        {
            perror("Message from read [server] : ");
            exit(-5);
        }
        printf("Server got %s ==> %zd bytes read from client\n", buf, readb);
        printf("[Server] ==> let us write to client\n");
        if (write(fd_client, "Goodbye!", 9) == -1)
        {
            perror("Message from write [server]");
            exit(-6);
        }
        close(fd_skt);
        close(fd_client);
        exit(0);
    }
}
