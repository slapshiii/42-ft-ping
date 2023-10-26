#include "ft_ping.h"

static void usage(char *name, int code)
{
	printf("\nUsage\n  %s [options] <destination>\n", name);
	printf("\nOptions:\n");
	exit(code);
}

void parse_arg(int ac, char **av, ping_data *res)
{
	ft_bzero(res, sizeof(res));
	if (ac < 2)
	{
		printf("%s: usage error: Destination address required\n", av[0]);
		exit(1);
	}
	res->interval.tv_usec = 1000;
	res->interval.tv_sec = 0;
	res->ttl = 128;
	res->count = -1;
	res->timeout = 1;
	res->pktsize = 56;
	for (int i = 1; i < ac; i++)
	{
		if ((av[i][0] == '-'))
		{
			switch (av[i][1])
			{
			case 'c': //set count
                if (i+1 >= ac || (res->count = ft_atoi(av[++i])) == 0)
                    usage(av[0], 1);
                break;
			case 'h':
				usage(av[0], 0);
				break;
			case 'i': // set interval
				if (i + 1 >= ac)
					usage(av[0], 1);
				res->interval.tv_sec = ft_atoi(av[++i]);
				char *frac = ft_strrchr(av[i], '.');
				res->interval.tv_usec = (frac)?ft_atoi(frac + 1) * (mypow(10, 4-ft_strlen(frac))):0;
				break;
			case 'l': // set TTL
				if (i + 1 >= ac || (res->ttl = ft_atoi(av[++i])) == 0)
					usage(av[0], 1);
				break;
			case 's': // set packetsize
				if (i + 1 >= ac || (res->pktsize = ft_atoi(av[++i])) == 0)
					usage(av[0], 1);
				break;
			case 'W': // set timeout
				if (i + 1 >= ac || (res->timeout = ft_atoi(av[++i])) == 0)
					usage(av[0], 1);
				break;
			default:
				usage(av[0], 1);
				break;
			}
		}
		else
		{
			res->hostname = av[i];
		}
	}
}

int receive_pckt(int fd, struct ip_pkt *ippckt, struct ping_pkt *ppkt, int size)
{
	struct msghdr msg;
	struct iovec iov[1];
	ft_bzero(&msg, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	iov[0].iov_base = ippckt;
	iov[0].iov_len = size;
	if (recvmsg(fd, &msg, 0) < 0)
	{
		printf("\nPacket receive failed! %s\n", strerror(errno));
	}
	ippckt->hdr.len = (ippckt->hdr.len >> 8) | (ippckt->hdr.len << 8);
	ft_memcpy(ppkt, (void*)ippckt + IP_HDR, size - IP_HDR);
	return (1);
}

void send_ping(ping_data *data)
{
	int msg_count = 0, flag = 1,
		msg_received_count = 0, on = 1;
	struct ping_pkt *pckt, *res_ping;
	struct ip_pkt *res_ip;

	long double rtt_msec = 0;
	long double total_msec = 0;
	struct timeval tv_out;
	struct timeval tv_start, tv_end, tv_fs, tv_fe;
	tv_out.tv_sec = data->timeout;
	tv_out.tv_usec = 0;
	struct sockaddr_in *ping_addr = (struct sockaddr_in *)data->ip_addr->ai_addr;

	printf("PING %s (%s) %d(%ld) bytes of data.\n", data->hostname, data->hostaddr, data->pktsize, data->pktsize+PING_HDR+IP_HDR);
	gettimeofday(&tv_fs, NULL);
	if (setsockopt(data->sockfd, SOL_IP, IP_TTL, &data->ttl, sizeof(data->ttl)) != 0)
	{
		printf("\nSetting socket options to TTL failed! %s\n", strerror(errno));
		return;
	}
	if (setsockopt(data->sockfd, SOL_IP, IP_RECVERR,
				   &on, sizeof(on)) != 0)
	{
		printf("\nSetting socket options to REVCERR failed! %s\n", strerror(errno));
		return;
	}
	// setting timeout of recv setting
	setsockopt(data->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof tv_out);
	setsockopt(data->sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv_out, sizeof tv_out);

	pckt = (struct ping_pkt*)malloc(PING_SIZE);
	res_ping = (struct ping_pkt*)malloc(PING_SIZE);
	res_ip = (struct ip_pkt*)malloc(IP_SIZE);
	// send icmp packet in an infinite loop
	while (pingloop && data->count)
	{
		if (data->count > 0)
			data->count--;
		// flag is whether packet was sent or not
		flag = 1;

		// filling packet
		ft_bzero(pckt, PING_SIZE);
		ft_bzero(res_ping, PING_SIZE);
		ft_bzero(res_ip, IP_SIZE);

		if (msg_count) {
			sleep(data->interval.tv_sec);
			usleep(data->interval.tv_usec*1000);
		}

		pckt->hdr.type = ICMP_ECHO;
		pckt->hdr.rest.echo.id = getpid();
		pckt->hdr.rest.echo.sequence = msg_count++;
		pckt->hdr.checksum = checksum(pckt, PING_SIZE);

		// send packet
		gettimeofday(&tv_start, NULL);

		if (sendto(data->sockfd, pckt, PING_SIZE, 0, (struct sockaddr *)ping_addr, sizeof(*ping_addr)) <= 0)
		{
			printf("\nPacket Sending Failed!\n");
			flag = 0;
		}

		// receive packet
		while (pingloop && ((receive_pckt(data->sockfd, res_ip, res_ping, IP_SIZE) <= 0 && msg_count > 1) || res_ping->hdr.rest.echo.id != pckt->hdr.rest.echo.id))
		{
			; // drop packet
		}
		if (pingloop)
		{
			gettimeofday(&tv_end, NULL);
			double timeElapsed = ((double)(tv_end.tv_usec - tv_start.tv_usec)) / 1000.0;
			rtt_msec = (tv_end.tv_sec - tv_start.tv_sec) * 1000.0 + timeElapsed;
			if (flag)
			{
				if (!(res_ping->hdr.type == 0 && res_ping->hdr.code == 0))
				{
					printf("Error..Packet received with ICMP type %d code %d\n", res_ping->hdr.type, res_ping->hdr.code);
				}
				else
				{
					printf("%ld bytes from %s (%s) icmp_seq=%d ttl=%d time=%.2Lf ms\n",
						   PING_SIZE, data->reverse_hostname, data->hostaddr,
						   res_ping->hdr.rest.echo.sequence, res_ip->hdr.ttl, rtt_msec);

					msg_received_count++;
				}
			}
		}
	}
	gettimeofday(&tv_fe, NULL);
	double timeElapsed = ((double)(tv_fe.tv_usec - tv_fs.tv_usec)) / 1000.0;

	total_msec = (tv_fe.tv_sec - tv_fs.tv_sec) * 1000.0 + timeElapsed;

	printf("\n=== %s ping statistics ===\n", data->hostname);
	printf("%d packets sent, %d packets received, %.0f%% packet loss. Total time: %.0Lfms.\n\n",
		   msg_count, msg_received_count,
		   ((msg_count - msg_received_count) / msg_count) * 100.0,
		   total_msec);

	free(pckt);
	free(res_ping);
	free(res_ip);
}

