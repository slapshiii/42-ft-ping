#include "ft_ping.h"

static void usage(char *name, int code)
{
	printf("\nUsage\n  %s [options] <destination>\n", name);
	printf("\nOptions:\n");
	exit(code);
}

ping_data parse_arg(int ac, char **av)
{
	ping_data res;
	ft_bzero(&res, sizeof(res));
	if (ac < 2)
	{
		printf("%s: usage error: Destination address required\n", av[0]);
		exit(1);
	}
	for (int i = 1; i < ac; i++)
	{
		if ((av[i][0] == '-'))
		{
			switch (av[i][1])
			{
			case 'c': //set count
                if (i+1 >= ac || (res.count = ft_atoi(av[++i])) == 0)
                    usage(av[0], 1);
                break;
			case 'h':
				usage(av[0], 0);
				break;
			case 'i': // set interval
				if (i + 1 >= ac || (res.interval = ft_atoi(av[++i])) == 0)
					usage(av[0], 1);
				break;
			case 'l': // set TTL
				if (i + 1 >= ac || (res.ttl = ft_atoi(av[++i])) == 0)
					usage(av[0], 1);
				break;
			case 's': // set packetsize
				if (i + 1 >= ac || (res.pktsize = ft_atoi(av[++i])) == 0)
					usage(av[0], 1);
				break;
			default:
				usage(av[0], 1);
				break;
			}
		}
		else
		{
			res.hostname = av[i];
		}
	}
	return (res);
}

void send_ping(ping_data *data)
{
	int ttl_val = 64, msg_count = 0, flag = 1,
		msg_received_count = 0;
	long unsigned int i;
	struct ping_pkt pckt, res_ping;
	struct ip_pkt res_ip;

	long double rtt_msec = 0;
	long double total_msec = 0;
	struct timeval tv_out;
	struct timeval tv_start, tv_end, tv_fs, tv_fe;
	tv_out.tv_sec = RECV_TIMEOUT;
	tv_out.tv_usec = 0;
	struct sockaddr_in *ping_addr = (struct sockaddr_in *)data->ip_addr->ai_addr;

	gettimeofday(&tv_fs, NULL);

	// set socket options at ip to TTL and value to 64,
	// change to what you want by setting ttl_val
	if (setsockopt(data->sockfd, SOL_IP, IP_TTL,
				   &ttl_val, sizeof(ttl_val)) != 0)
	{
		printf("\nSetting socket options to TTL failed!\n");
		return;
	}
	// setting timeout of recv setting
	setsockopt(data->sockfd, SOL_SOCKET, SO_RCVTIMEO,
			   (const char *)&tv_out, sizeof tv_out);

	// send icmp packet in an infinite loop
	while (pingloop)
	{
		// flag is whether packet was sent or not
		flag = 1;

		// filling packet
		ft_bzero(&pckt, sizeof(pckt));
		ft_bzero(&res_ip, sizeof(res_ip));
		ft_bzero(&res_ping, sizeof(res_ping));

		pckt.hdr.type = ICMP_ECHO;
		pckt.hdr.rest.echo.id = getpid();

		for (i = 0; i < sizeof(pckt.msg) - 1; i++)
			pckt.msg[i] = i + '0';

		pckt.msg[i] = 0;
		pckt.hdr.rest.echo.sequence = msg_count++;
		pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));

		usleep(PING_SLEEP_RATE);

		// send packet
		gettimeofday(&tv_start, NULL);

		if (sendto(data->sockfd, &pckt, sizeof(pckt), 0,
				   (struct sockaddr *)ping_addr,
				   sizeof(*ping_addr)) <= 0)
		{
			printf("\nPacket Sending Failed!\n");
			flag = 0;
		}

		// receive packet
		while ((receive_pckt(data->sockfd, &res_ip, &res_ping) <= 0 && msg_count > 1) || res_ping.hdr.rest.echo.id != pckt.hdr.rest.echo.id)
		{
			; // drop packet
		}
		if (res_ping.hdr.type == 0)
		{
			gettimeofday(&tv_end, NULL);

			double timeElapsed = ((double)(tv_end.tv_usec - tv_start.tv_usec)) / 1000.0;
			rtt_msec = (tv_end.tv_sec - tv_start.tv_sec) * 1000.0 + timeElapsed;

			// if packet was not sent, don't receive
			if (flag)
			{
				if (!(res_ping.hdr.type == 0 && res_ping.hdr.code == 0))
				{
					printf("Error..Packet received with ICMP type %d code %d\n", res_ping.hdr.type, res_ping.hdr.code);
				}
				else
				{
					printf("%d bytes from %s (%s) icmp_seq=%d ttl=%d time=%.2Lf ms\n",
						   PING_PKT_S, data->reverse_hostname, data->hostaddr,
						   res_ping.hdr.rest.echo.sequence, res_ip.hdr.ttl, rtt_msec);

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
}