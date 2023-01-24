// Source: https://www.danlj.org/mkj/lad/src/userver.c.html 
// (Johnson & Troan, pp.298-299)

#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>

/* Copies data from file descriptor 'from' to file descriptor 'to'
   until nothing is left to be copied. Exits if an error occurs.   */

void copyData(int from, int to) {
    char buf[1024];
    int amount;
    while ((amount = read(from, buf, sizeof(buf))) > 0) {
       if (write(to, buf, amount) != amount) {
            perror("Message from write:");
            exit(1); }}
    if (amount < 0) {
        perror("Message from read:");
        exit(1); } }

/* userver.c - simple server for Unix domain sockets */

/* Waits for a connection on the ./sample-socket Unix domain
   socket. Once a connection has been established, copy data
   from the socket to stdout until the other end closes the
   connection, and then wait for another connection to the socket. */

int main(void) {
    struct sockaddr_un address;
    int sock, conn;
    socklen_t addrLength;
    if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
         perror("Message from socket:");
         exit(1); }
         
    /* Remove any preexisting socket (or other file) */
    unlink("./sample-socket");
    address.sun_family = AF_UNIX;	/* Unix domain socket */
    strcpy(address.sun_path, "./sample-socket");

    /* The total length of the address includes the sun_family element */
    addrLength = sizeof(address.sun_family) + strlen(address.sun_path);

    if (bind(sock, (struct sockaddr *) &address, addrLength)){
        perror("Message from bind:");
        exit(1); }
        
    if (listen(sock, 5)){
        perror("Message from listen:");
        exit(1); }

    while ((conn = accept(sock, (struct sockaddr *) &address, 
                          &addrLength)) >= 0) {
        printf("---- getting data\n");
        copyData(conn, 1);
        printf("---- done\n");
        close(conn); }
    if (conn < 0) {
        perror("Message from accept:");
        exit(1); }
    
    close(sock);
    return 0; }
