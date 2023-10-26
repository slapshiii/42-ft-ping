#include <ft_ping.h>

// Calculating the Check Sum
unsigned short checksum(void *b, int len)
{
	unsigned short *buf = b;
	unsigned int sum = 0;
	unsigned short result;

	while (len > 1)
	{
		/*  This is the inner loop */
		sum += *buf++;
		len -= 2;
	}

	/*  Add left-over byte, if any */
	if (len > 0)
		sum += *(unsigned char *)buf;

	/*  Fold 32-bit sum to 16 bits */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);
	result = ~sum;
	return result;
}

// Performs a DNS lookup
int dns_lookup(char *addr_host, struct addrinfo **res)
{
	struct addrinfo hints;
	int status;
	ft_memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	printf("\nResolving DNS..\n");
	if ((status = getaddrinfo(addr_host, NULL, &hints, res)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return status;
	}
	return (0);
}

// Resolves the reverse lookup of the hostname
char *reverse_dns_lookup(struct addrinfo *p)
{
	char hbuf[NI_MAXHOST];
	char *ret_buf;
	int status;

	printf("\nResolving reverseDNS..\n");
	status = getnameinfo(p->ai_addr, p->ai_addrlen, hbuf, NI_MAXHOST, NULL, 0, 0);
	if (status == 0)
	{
		printf("Hostname: %s\n", hbuf);
	}
	else
	{
		fprintf(stderr, "getnameinfo: %s\n", gai_strerror(status));
	}
	ret_buf = (char *)malloc((strlen(hbuf) + 1) * sizeof(char));
	ft_strcpy(ret_buf, hbuf);
	return ret_buf;
}

// Checks if the string is a valid ipv4 address
int is_valid_ipv4(char *ip_str)
{
	struct sockaddr_in sa;
	return (inet_pton(AF_INET, ip_str, &(sa.sin_addr)) == 1);
}

int receive_pckt(int fd, struct ip_pkt *ippckt, struct ping_pkt *ppkt)
{
	struct msghdr msg;
	struct iovec iov[1];
	ft_bzero(&msg, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	iov[0].iov_base = ippckt;
	iov[0].iov_len = sizeof(struct ip_pkt);

	if (recvmsg(fd, &msg, 0) <= 0)
	{
		printf("\nPacket hdr receive failed! %s\n", strerror(errno));
	}
	ippckt->hdr.len = (ippckt->hdr.len >> 8) | (ippckt->hdr.len << 8);
	*ppkt = *(struct ping_pkt *)ippckt->data;
	// DumpIpPck(*ippckt);
	return (1);
}

void DumpHex(const void *data, size_t size)
{
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i)
	{
		printf("%02X ", ((unsigned char *)data)[i]);
		if (((unsigned char *)data)[i] >= ' ' && ((unsigned char *)data)[i] <= '~')
		{
			ascii[i % 16] = ((unsigned char *)data)[i];
		}
		else
		{
			ascii[i % 16] = '.';
		}
		if ((i + 1) % 8 == 0 || i + 1 == size)
		{
			printf(" ");
			if ((i + 1) % 16 == 0)
			{
				printf("|  %s \n", ascii);
			}
			else if (i + 1 == size)
			{
				ascii[(i + 1) % 16] = '\0';
				if ((i + 1) % 16 <= 8)
				{
					printf(" ");
				}
				for (j = (i + 1) % 16; j < 16; ++j)
				{
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
}

void DumpIpPck(struct ip_pkt data){
	printf("\nversion:%d IHL:%d tos:%d len:%d", data.hdr.verlen>>4, data.hdr.verlen&15, data.hdr.tos_ecn, data.hdr.len);
	printf("\nid:%d flag et offset:%d", data.hdr.id, data.hdr.flag_fragoff);
	printf("\nttl:%d proto:%d checksum:%d", data.hdr.ttl, data.hdr.proto, data.hdr.checksum);
}

int mypow(int x, int n) {
	int res = 1;
	for (int i = 0; i < n; ++i) {
		res *= x;
	}
	return (res);
}
