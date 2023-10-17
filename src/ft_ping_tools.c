#include <ft_ping.h>

// Calculating the Check Sum
unsigned short	checksum(void *b, int len)
{
	unsigned short *buf = b;
	unsigned int sum = 0;
	unsigned short result;

	while (len > 1) {
	/*  This is the inner loop */
		sum += *buf++;
		len -= 2;
	}

	/*  Add left-over byte, if any */
	if (len > 0)
		sum += *(unsigned char*)buf;

	/*  Fold 32-bit sum to 16 bits */
	while (sum>>16)
		sum = (sum & 0xffff) + (sum >> 16);
	result = ~sum;
    return result;
}


// Performs a DNS lookup
int dns_lookup(char *addr_host, struct addrinfo** res)
{
    struct addrinfo hints;
    int status;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    printf("\nResolving DNS..\n");
    if ((status = getaddrinfo(addr_host, NULL, &hints, res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return status;
    }
    return (0);
}
 
// Resolves the reverse lookup of the hostname
char* reverse_dns_lookup(struct addrinfo *p)
{
    char hbuf[NI_MAXHOST];
    char sbuf[NI_MAXSERV];
    char *ret_buf;
    int status;

    printf("\nResolving reverseDNS..\n");
    status = getnameinfo(p->ai_addr, p->ai_addrlen, hbuf, NI_MAXHOST, sbuf, NI_MAXSERV, 0);
    if (status == 0) {
        printf("Hostname/Serv: %s - %s\n", hbuf, sbuf);
    } else {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(status));
    }
    ret_buf = (char*)malloc((strlen(hbuf) +1)*sizeof(char) );
    strcpy(ret_buf, hbuf);
    return ret_buf;
}

int is_valid_ipv4(char *ip_str)
{
    struct sockaddr_in sa;
    return (inet_pton(AF_INET, ip_str, &(sa.sin_addr)) == 1);
}

// void print_statistics()
