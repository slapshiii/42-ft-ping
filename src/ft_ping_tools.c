#include <ft_ping.h>

// Calculating the Check Sum
unsigned short	checksum(void *b, int len)
{
	unsigned short *buf = b;
	unsigned int sum = 0;
	unsigned short result;

	while (count > 1) {
	/*  This is the inner loop */
		sum += *buf++;
		count -= 2;
	}

	/*  Add left-over byte, if any */
	if (count > 0)
		sum += *(unsigned char*)buf;

	/*  Fold 32-bit sum to 16 bits */
	while (sum>>16)
		sum = (sum & 0xffff) + (sum >> 16);
	result = ~sum;
}

// Resolves the reverse lookup of the hostname
<<<<<<< HEAD
// char* reverse_dns_lookup(char *ip_addr)
// {
// 	struct sockaddr_in temp_addr;   
// 	socklen_t len;
// 	char buf[NI_MAXHOST], *ret_buf;

// 	if (inet_pton(AF_INET, ip_addr, void *restrict dst) == 0) {
// 		printf("Could not resolve reverse lookup of hostname\n");
// 		return NULL;
// 	}
=======
char* reverse_dns_lookup(char *ip_addr)
{
	struct in_addr		buf_addr;
	struct sockaddr_in	temp_addr;
	socklen_t len;
	char buf[NI_MAXHOST], *ret_buf;

	if (inet_pton(AF_INET, ip_addr, &buf_addr) == 0) {
		printf("Could not resolve reverse lookup of hostname\n");
		return NULL;
	}
>>>>>>> 759c14044795be5d2ea8a66254283c808d76308f

// 	temp_addr.sin_family = AF_INET;
// 	temp_addr.sin_addr.s_addr = inet_addr(ip_addr);
// 	len = sizeof(struct sockaddr_in);
 
// 	if (getnameinfo((struct sockaddr *) &temp_addr, len, buf,
// 					sizeof(buf), NULL, 0, NI_NAMEREQD))
// 	{
// 		printf("Could not resolve reverse lookup of hostname\n");
// 		return NULL;
// 	}
// 	ret_buf = (char*)malloc((strlen(buf) +1)*sizeof(char) );
// 	strcpy(ret_buf, buf);
// 	return ret_buf;
// }