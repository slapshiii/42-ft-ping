#include "ft_ping.h"

int receive_pckt(int fd, struct ip_pkt *ippckt, struct ping_pkt *ppkt, int size)
{
	struct msghdr msg;
	struct iovec iov[1];
	int		id = ppkt->hdr.rest.echo.id;
	struct ping_pkt *tmp = NULL;

	while (1)
	{
		ft_bzero(&msg, sizeof(msg));
		msg.msg_iov = iov;
		msg.msg_iovlen = 1;
		iov[0].iov_base = ippckt;
		iov[0].iov_len = size;
		if ((recvmsg(fd, &msg, 0)) < 0) {
			if (errno == 113) {
				continue;
			}
			uint16_t test = *(uint16_t*)((void*)ippckt + 52);
			if (id == test)
			{
				ft_memcpy(ppkt, ((void*)ippckt) + IP_HDR, sizeof(struct ping_pkt));
				return (2);
			}
			printf("%s\n", strerror(errno));
			return (-1);
		}
		tmp = ((void*)ippckt) + IP_HDR;
		if (tmp->hdr.type == ICMP_ECHO)
			continue;
		if (tmp->hdr.rest.echo.id == id)
		{
			ft_memcpy(ppkt, tmp, sizeof(struct ping_pkt));
			return (1);
		}
	}
	return (0);
}

void send_ping(ping_data *data)
{
	int msg_count = 0, flag, msg_received_count = 0;
	double rtt_min = 999, rtt_max = 0, rtt_avg = 0;
	// double total_msec = 0;

	t_list *lst_values = NULL;
	
	struct timeval tv_start, tv_end, tv_fs, tv_fe;

	struct ping_pkt *pckt;
	struct ip_pkt *res_ip;

	printf("PING %s (%s): %d data bytes\n", data->hostname, data->hostaddr, data->pktsize);
	gettimeofday(&tv_fs, NULL);

	pckt = (struct ping_pkt*)malloc(PING_SIZE);
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
		ft_bzero(res_ip, IP_SIZE);

		if (msg_count) {
			sleep(data->interval.tv_sec);
			usleep(data->interval.tv_usec*1000);
		}

		// send packet
		gettimeofday(&tv_start, NULL);
		if (data->deadline > 0 && tv_start.tv_sec - tv_fs.tv_sec >= data->deadline) {
			pingloop = 0;
			break;
		}
		pckt->hdr.type = ICMP_ECHO;
		pckt->hdr.rest.echo.id = getpid();
		pckt->hdr.rest.echo.sequence = msg_count + 1;
		pckt->hdr.checksum = checksum(pckt, PING_SIZE);

		if (sendto(data->sockfd, pckt, PING_SIZE, 0, data->ip_addr->ai_addr, sizeof(*data->ip_addr->ai_addr)) <= 0)
		{
			printf("\nPacket Sending Failed!\n");
			flag = 0;
		}
		msg_count++;

		// receive packet
		if (receive_pckt(data->sockfd, res_ip, pckt, IP_SIZE) > 0)
		{
			gettimeofday(&tv_end, NULL);
			double timeElapsed = ((double)(tv_end.tv_usec - tv_start.tv_usec)) / 1000.0;
			double *rtt_msec = (double*)malloc(sizeof(double));
			*rtt_msec = (tv_end.tv_sec - tv_start.tv_sec) * 1000.0 + timeElapsed;
			rtt_min = (rtt_min > *rtt_msec) ? *rtt_msec : rtt_min;
			rtt_max = (rtt_max < *rtt_msec) ? *rtt_msec : rtt_max;
			rtt_avg += *rtt_msec;
			ft_lstadd_back(&lst_values, ft_lstnew(rtt_msec));
			if (pckt->hdr.type == ICMP_ECHOREPLY)
				msg_received_count++;
			if (flag)
			{
				if ((pckt->hdr.type == ICMP_TIME_EXCEEDED && pckt->hdr.code == ICMP_EXC_TTL))
				{
					printf("From %s (%s) icmp_seq=%d Time to live exceeded\n", data->reverse_hostname, data->hostaddr, pckt->hdr.rest.echo.sequence);
				}
				else if (pckt->hdr.type != ICMP_ECHOREPLY)
				{
					printf("Error..Packet received with ICMP type %d code %d\n", pckt->hdr.type, pckt->hdr.code);
				}
				else
				{
					printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
						   PING_SIZE, data->hostaddr,
						   pckt->hdr.rest.echo.sequence, res_ip->hdr.ttl, *rtt_msec);
				}
			}
		}
	}
	gettimeofday(&tv_fe, NULL);
	// double timeElapsed = ((double)(tv_fe.tv_usec - tv_fs.tv_usec)) / 1000.0;
	// total_msec = (tv_fe.tv_sec - tv_fs.tv_sec) * 1000.0 + timeElapsed;
	rtt_avg /= msg_received_count;

	printf("\n--- %s ping statistics ---n", data->hostname);
	printf("%d packets transmitted, %d packets received, %.0f%% packet loss\n",
		   msg_count, msg_received_count, ((float)(msg_count - msg_received_count) / (float)msg_count) * 100.0);
	printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
		   rtt_min, rtt_avg, rtt_max, calculate_stddev(lst_values, rtt_avg, msg_received_count));

	ft_lstclear(&lst_values, free);
	free(pckt);
	free(res_ip);
}

