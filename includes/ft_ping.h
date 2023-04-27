#ifndef FT_PING_H
#define FT_PING_H

// #include <sys/types.h>
#include <sys/socket.h>
// #include <netdb.h>

#include "impc.h"
#include "libft.h"

#ifndef NI_MAXHOST
# define NI_MAXHOST		1024
#endif

#ifndef NI_NAMEREQD
# define NI_NAMEREQD	0x04
#endif

<<<<<<< HEAD
// Define the Packet Constants
// ping packet size
#define PING_PKT_S 64
  
// Automatic port number
#define PORT_NO 0
 
// Automatic port number
#define PING_SLEEP_RATE 1000000 x
 
// Gives the timeout delay for receiving packets
// in seconds
#define RECV_TIMEOUT 1

// ping packet structure
struct ping_pkt
{
    struct icmphdr hdr;
    char msg[PING_PKT_S-sizeof(struct icmphdr)];
};
=======
typedef struct  icmp_s {
    uint8_t     type;
    uint8_t     code;
    uint16_t    checksum;
    void        *content;
}               icmp;
>>>>>>> 759c14044795be5d2ea8a66254283c808d76308f

// TOOLS
unsigned short	checksum(void *b, int len);

#endif //FT_PING_H