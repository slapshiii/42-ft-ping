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

	// printf("\nResolving DNS..\n"); TODEL
	if ((status = getaddrinfo(addr_host, NULL, &hints, res)) != 0)
	{
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

	// printf("\nResolving reverseDNS..\n"); TODEL
	status = getnameinfo(p->ai_addr, p->ai_addrlen, hbuf, NI_MAXHOST, NULL, 0, 0);
	if (status != 0)
	{
		fprintf(stderr, "getnameinfo: %s\n", gai_strerror(status));
	}
	ret_buf = (char *)malloc((ft_strlen(hbuf) + 1) * sizeof(char));
	ft_strcpy(ret_buf, hbuf);
	return ret_buf;
}

// Checks if the string is a valid ipv4 address
int is_valid_ipv4(char *ip_str)
{
	struct sockaddr_in sa;
	return (inet_pton(AF_INET, ip_str, &(sa.sin_addr)) == 1);
}

void print_HdrDump(struct ip_pkt *pkt)
{

	struct in_addr ip_addr_d;
	struct in_addr ip_addr_s;
	ip_addr_d.s_addr = pkt->hdr.dstadrr;
	ip_addr_s.s_addr = pkt->hdr.srcaddr;
	printf("IP Hdr Dump:\n");
	for (int i = 0; i < (int)IP_HDR; ++i)
	{
		if (!(i % 2))
			printf(" ");
		printf("%02x", ((unsigned char *)pkt)[i]);
	}
	printf("\nVr HL TOS  Len   ID Flg  off TTL Pro  cks      Src   Dst     Data\n");
	printf("%2x %2x %2x%x %04x %04x %3d %04d  %02d %02d %4x %s %s\n",
		pkt->hdr.verlen >> 4, pkt->hdr.verlen & 0xf,
		pkt->hdr.tos_ecn >> 2, pkt->hdr.tos_ecn & 0x2,
		pkt->hdr.len, pkt->hdr.id,
		pkt->hdr.flag_fragoff >> 13, pkt->hdr.flag_fragoff & 0x1fff,
		pkt->hdr.ttl, pkt->hdr.proto, pkt->hdr.checksum,
		inet_ntoa(ip_addr_s), inet_ntoa(ip_addr_d)
	);
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
	printf("\nttl:%d proto:%d checksum:%d\n", data.hdr.ttl, data.hdr.proto, data.hdr.checksum);
}
void DumpPingPck(struct ping_pkt data){
	printf("\ntype:%d code:%d checksum:%d", data.hdr.type, data.hdr.code, data.hdr.checksum);
	printf("\nid:%d seq num:%d\n", data.hdr.rest.echo.id, data.hdr.rest.echo.sequence);
}

int mypow(int x, int n) {
	int res = 1;
	for (int i = 0; i < n; ++i) {
		res *= x;
	}
	return (res);
}

double mypowd(double x, int n) {
	double res = 1;
	for (int i = 0; i < n; ++i) {
		res *= x;
	}
	return (res);
}

double findSQRT(double number)
{
    double start = 0, end = number;
    double mid;
    float ans;
    while (start <= end) {
        mid = (start + end) / 2;
        if (mid * mid == number) {
            ans = mid;
            break;
        }
        if (mid * mid < number) {
            ans=start;
            start = mid + 1;
        }
        else {
            end = mid - 1;
        }
    }
    float increment = 0.1;
    for (int i = 0; i < 5; i++) {
        while (ans * ans <= number) {
            ans += increment;
        }
        ans = ans - increment;
        increment = increment / 10;
    }
    return ans;
}

double calculate_stddev(t_list *values, double mean, int count)
{
	t_list *curr = values;
	double	var;
	double	dev;
	while (curr)
	{
		dev = *(double*)curr->content - mean;
		var += mypowd(dev, 2);
		curr = curr->next;
	}
	var = var / (count - 1);
	return (findSQRT(var));
}
