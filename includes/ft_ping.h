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

//BONUS

#include "impc.h"
#include "libft.h"

#ifndef NI_MAXHOST
#define NI_MAXHOST 1024
#endif
#ifndef NI_MAXSERV
#define NI_MAXSERV 32
#endif

#ifndef NI_NAMEREQD
#define NI_NAMEREQD 0x04
#endif

// Define the Packet Constants
#define PING_PKT_SIZE data->pktsize
#define PING_HDR sizeof(struct icmphdr)
#define PORT_NO 0	  // Automatic port number
#define PING_SLEEP_RATE 1000000
#define RECV_TIMEOUT 1 // Gives the timeout delay for receiving packets in seconds
#define PING_SIZE (PING_HDR + PING_PKT_SIZE)

#define IP_HDR sizeof(struct iphdr)
#define IPHDR_LEN(h) (((h).verlen & 0b1111) * 4)
#define IP_SIZE (IP_HDR + PING_SIZE)

#define REVERSE_ENDIAN16(x) ((x << 8) | (x >> 8))

// ping packet structure
struct ping_pkt
{
	struct icmphdr hdr;
	char *msg;
};

struct iphdr
{
	uint8_t verlen;
	uint8_t tos_ecn;
	uint16_t len;
	uint16_t id;
	uint16_t flag_fragoff;
	uint8_t ttl;
	uint8_t proto;
	uint16_t checksum;
	uint32_t srcaddr;
	uint32_t dstadrr;
};
// ping packet structure
struct ip_pkt
{
	struct iphdr hdr;
	unsigned char *data;
};

typedef struct ping_data_s
{
	char			*hostname;
	char			*reverse_hostname;
	struct addrinfo	*ip_addr;
	int				sockfd;
	int				is_addr;
	char			hostaddr[INET_ADDRSTRLEN];
	int				count;
	int				deadline;
	int				ttl;
	int				pktsize;
	int				timeout;
	int				verbose;
	struct timeval	interval;
}				ping_data;

typedef struct res_ip_s
{
	struct sockaddr	sa;
	uint16_t		size;
	int				ttl;
	struct ip_pkt	*ip_pck;
}				res_ip;

#define STAT_MIN 0
#define STAT_AVG 1
#define STAT_MAX 2
#define STAT_STDDEV 3

extern int pingloop;

// TOOLS
unsigned short checksum(void *b, int len);
int dns_lookup(char *addr_host, struct addrinfo **res);
char *reverse_dns_lookup(struct addrinfo *p);
int is_valid_ipv4(char *ip_str);
int is_valid_ipv4_hdr(unsigned char *buf);
int	calculate_stats(t_list *values, double *tab);

void send_ping(ping_data *data);

void parse_arg(int ac, char **av, ping_data *data);
void DumpHex(const void *data, size_t size);
int receive_pckt(res_ip *res, struct ping_pkt *ppckt, ping_data *data);

void DumpIpPck(struct ip_pkt data);
void DumpPingPck(struct ping_pkt data);
void print_HdrDump(struct ip_pkt *pkt);
int mypow(int x, int n);
double mypowd(double x, int n);
double findSQRT(double number);
double calculate_stddev(t_list *values, double mean, int count);

#endif // FT_PING_H