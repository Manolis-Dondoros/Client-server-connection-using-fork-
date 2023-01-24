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
        
/* uclient.c - simple client for Unix domain sockets */

/* Connect to the ./sample-socket Unix domain socket, copy stdin
   into the socket, and then exit. */

int main(void) {
    struct sockaddr_un address;
    int sock;
    socklen_t addrLength;

    if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0){
         perror("Message from socket:");
         exit(1); }

    address.sun_family = AF_UNIX;    /* Unix domain socket */
    strcpy(address.sun_path, "./sample-socket");

    /* The total length of the address includes the sun_family element */
    addrLength = sizeof(address.sun_family) + strlen(address.sun_path);

    if (connect(sock, (struct sockaddr *) &address, addrLength)){
         perror("Message from connect:");
         exit(1); }

    copyData(0, sock);
    
    close(sock);
    return 0; }
