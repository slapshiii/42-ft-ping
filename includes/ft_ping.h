#ifndef FT_PING_H
#define FT_PING_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#ifndef NI_MAXHOST
# define NI_MAXHOST		1024
#endif

#ifndef NI_NAMEREQD
# define NI_NAMEREQD	0x04
#endif

typedef struct  icmp_s {
    uint8_t     type;
    uint8_t     code;
    uint16_t    checksum;
    void        *content;
}               icmp;

// TOOLS
unsigned short	checksum(void *b, int len);

#endif //FT_PING_H