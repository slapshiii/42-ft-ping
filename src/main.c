// run as sudo ./ft_ping <hostname>

#include "ft_ping.h"

// Define the Ping Loop
int pingloop = -1;

// Interrupt handler
void intHandler(int dummy)
{
	(void)dummy;
	pingloop = 0;
}

static void usage(char *name, int code)
{
	printf("Usage: %s [OPTION...] HOST ...\nSend ICMP ECHO_REQUEST packets to network hosts.\n\n", name);
	printf(" Options valid for all request types:\n\n");
	printf("  -c,\t\tstop after sending NUMBER packets\n"\
			"  -i,\t\twait NUMBER seconds between sending each packet\n"\
			"  -l,\t\tspecify N as time-to-live\n"\
			"  -v,\t\tverbose output\n"\
			"  -w,\t\tstop after N seconds\n"\
			"  -W,\t\tnumber of seconds to wait for response\n"\
			"  -s,\t\tsend NUMBER data octets\n"\
			"  -?, -h\tgive this help list\n"\
			"  -V,\t\tprint program version\n");

	exit(code);
}

void init_data(ping_data *res)
{
	ft_bzero(res, sizeof(res));
	res->interval.tv_usec = 1000;
	res->interval.tv_sec = 0;
	res->ttl = 64;
	res->count = -1;
	res->timeout = 1;
	res->pktsize = 56;
	res->deadline = -1;
	res->verbose = 0;
}

void parse_arg(int ac, char **av, ping_data *res)
{
	int i = 1;
	for (; i < ac; i++)
	{
		if ((av[i][0] == '-'))
		{
			switch (av[i][1])
			{
			case '?':
				// fall through
			case 'h':
				usage(av[0], 0);
				break;
			case 'c': // set count
				if (i + 1 < ac && (res->count = ft_atoi(av[++i])) != 0)
					break;
				// fall through
			case 'i': // set interval
				if (i + 1 < ac)
				{
					res->interval.tv_sec = ft_atoi(av[++i]);
					char *frac = ft_strrchr(av[i], '.');
					res->interval.tv_usec = (frac) ? ft_atoi(frac + 1) * (mypow(10, 4 - ft_strlen(frac))) : 0;
					break;
				}
				// fall through
			case 'l': // set TTL
				if (i + 1 < ac && (res->ttl = ft_atoi(av[++i])) != 0)
					break;
				// fall through
			case 's': // set packetsize
				if (i + 1 < ac && (res->pktsize = ft_atoi(av[++i])) != 0)
					break;
				// fall through
			case 'v': // set verbose
				res->verbose = 1;
				break;
				// fall through
			case 'w': // set deadline
				if (i + 1 < ac && (res->deadline = ft_atoi(av[++i])) != 0)
					break;
				// fall through
			case 'W': // set timeout
				if (i + 1 < ac && (res->timeout = ft_atoi(av[++i])) != 0)
					break;
				// fall through
			default:
				usage(av[0], 1);
				break;
			}
		}
		else
		{
			break;
		}
	}
	res->ac = i;
	if (res->ac == 0 || i == ac)
	{
		printf("%s: usage error: Destination address required\n", av[0]);
		exit(1);
	}
}

int init_socket(ping_data *data)
{
	int on = 1;
	struct timeval tv_out = {data->timeout, 0};

	if (setsockopt(data->sockfd, IPPROTO_IP, IP_TTL, (char *)&data->ttl, sizeof(data->ttl)) != 0)
	{
		printf("\nSetting socket options to TTL failed! %s\n", strerror(errno));
		return 1;
	}
	if (setsockopt(data->sockfd, IPPROTO_IP, IP_RECVERR, (char *)&on, sizeof(on)) != 0)
	{
		printf("\nSetting socket options to REVCERR failed! %s\n", strerror(errno));
		return 1;
	}
	if (setsockopt(data->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof tv_out) != 0)
	{
		printf("\nSetting socket options to RCV timeout failed! %s\n", strerror(errno));
		return 1;
	}
	if (setsockopt(data->sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv_out, sizeof tv_out) != 0)
	{
		printf("\nSetting socket options to SND timeout failed! %s\n", strerror(errno));
		return 1;
	}
	return 0;
}

// Driver Code
int main(int argc, char *argv[])
{
	ping_data data;
	struct addrinfo * addrinfo;
	int status;

	init_data(&data);
	parse_arg(argc, argv, &data);
	for (int i = data.ac; i < argc; ++i){
		data.hostname = argv[i];
		status = dns_lookup(data.hostname, &addrinfo);
		if (status != 0)
		{
			printf("%s: %s: %s\n", argv[0], data.hostname, gai_strerror(status));
			return (2);
		}
		for (struct addrinfo *p = addrinfo; p != NULL; p = p->ai_next)
		{
			if (p->ai_family == AF_INET)
			{
				data.ip_addr = p;
				inet_ntop(AF_INET, &(((struct sockaddr_in *)p->ai_addr)->sin_addr), data.hostaddr, INET_ADDRSTRLEN);
				data.reverse_hostname = reverse_dns_lookup(p);
				break;
			}
		}
		if ((data.is_addr = is_valid_ipv4(data.hostname)) != 0)
			ft_strcpy(data.hostaddr, data.hostname);

		data.sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
		if (data.sockfd < 0)
		{
			printf("\nSocket file descriptor not received!!\n");
			fprintf(stderr, "%s: %s: %s\n", argv[0], argv[1], strerror(errno));
		}
		else
		{
			signal(SIGINT, intHandler); // catching interrupt

			init_socket(&data);
			send_ping(&data);
		}
		if (!data.is_addr)
			free(data.reverse_hostname);
		freeaddrinfo(addrinfo);
		close(data.sockfd);
	}

	return 0;
}