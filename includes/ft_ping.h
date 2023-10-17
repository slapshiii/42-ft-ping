#ifndef FT_PING_H
#define FT_PING_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <arpa/inet.h>
#include <errno.h>

#include "impc.h"
#include "libft.h"

#ifndef NI_MAXHOST
# define NI_MAXHOST		1024
#endif
#ifndef NI_MAXSERV
# define NI_MAXSERV	32
#endif

#ifndef NI_NAMEREQD
# define NI_NAMEREQD	0x04
#endif

// Define the Packet Constants
// ping packet size
#define PING_PKT_S 64
  
// Automatic port number
#define PORT_NO 0
 
// Automatic port number
#define PING_SLEEP_RATE 1000000
 
// Gives the timeout delay for receiving packets
// in seconds
#define RECV_TIMEOUT 1

// ping packet structure
struct ping_pkt
{
    struct icmphdr hdr;
    char msg[PING_PKT_S-sizeof(struct icmphdr)];
};

typedef struct  ping_data_s
{
    char            *hostname;
    char            *reverse_hostname;
    struct addrinfo *ip_addr;
    int             sockfd;
    int             is_addr;
    char            hostaddr[INET_ADDRSTRLEN];
}               ping_data;

typedef struct  arg_s
{
    int count;
    int ttl;
    int interval;
    int paquetsize;
    

}               arg;

extern int pingloop;

// TOOLS
unsigned short	checksum(void *b, int len);
int dns_lookup(char *addr_host, struct addrinfo** res);
char* reverse_dns_lookup(struct addrinfo *p);
int is_valid_ipv4(char *ip_str);

void send_ping(ping_data *data);


#endif //FT_PING_H