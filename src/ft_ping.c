#include "ft_ping.h"

void	print_stats(t_list *values, int cnt_msg_send, char* host)
{
	double	stat[4] = {9999, 0, 0, 0};
	int msg_received_count;

	msg_received_count = calculate_stats(values, stat);
	stat[STAT_STDDEV] = calculate_stddev(values, stat[STAT_AVG], msg_received_count);

	printf("\n--- %s ping statistics ---\n", host);
	printf("%d packets transmitted, %d packets received, %.0f%% packet loss\n",
		   cnt_msg_send, msg_received_count, ((float)(cnt_msg_send - msg_received_count) / (float)cnt_msg_send) * 100.0);

	if (msg_received_count)
		printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
		   stat[STAT_MIN], stat[STAT_AVG], stat[STAT_MAX], stat[STAT_STDDEV]);

}

int receive_pckt(res_ip *res, struct ping_pkt *ppkt, ping_data* data)
{
	struct msghdr msg;
	struct iovec iov[1];
	struct ip_pkt *ippkt = NULL;
	unsigned char buf[IP_HDR];
	int status;

	while(pingloop)
	{
		ft_bzero(&msg, sizeof(msg));
		msg.msg_iov = iov;
		msg.msg_iovlen = 1;
		iov[0].iov_base = buf;
		iov[0].iov_len = IP_HDR;
		if ((status = recvmsg(data->sockfd, &msg, MSG_PEEK)) == IP_HDR) {
			if (is_valid_ipv4_hdr(buf))
			{
				res->size = (unsigned int)((struct ip_pkt *)buf)->hdr.len;
				res->size = REVERSE_ENDIAN16(res->size);
				ippkt = (struct ip_pkt *)malloc(res->size);
				if (!ippkt)
					return (-1);
				ft_bzero(&msg, sizeof(msg));
				msg.msg_name = &res->sa;
				msg.msg_namelen = sizeof(struct sockaddr);
				msg.msg_iov = iov;
				msg.msg_iovlen = 1;
				iov[0].iov_base = ippkt;
				iov[0].iov_len = (size_t)res->size;
				status = recvmsg(data->sockfd, &msg, 0);
				if (status != res->size)
					continue;
				if (msg.msg_flags & MSG_TRUNC)
					printf("truncated\n");
				ft_memcpy(ppkt, ((void*)ippkt) + IP_HDR, res->size - IP_HDR);
				res->ttl = ippkt->hdr.ttl;
				free(ippkt);
				return (1);
			}
		} else if (errno != EHOSTUNREACH) {
			printf("error peeking: %s (%d)\n", strerror(errno), errno);
			return (1);
		}
	}
	return (0);
}

void send_ping(ping_data *data)
{
	int msg_count = 0, flag;

	t_list *lst_values = NULL;
	
	struct timeval tv_start, tv_end, tv_fs, tv_fe;
	struct ping_pkt *pckt, *pckt_res;
	res_ip res;
	int pid = getpid();

	printf("PING %s (%s): %d data bytes", data->hostname, data->hostaddr, data->pktsize);
	if (data->verbose)
		printf(", id 0x0%x = %d\n", pid, pid);
	else
		printf("\n");
	gettimeofday(&tv_fs, NULL);

	pckt = (struct ping_pkt*)malloc(PING_SIZE);
	pckt_res = (struct ping_pkt*)malloc(PING_SIZE);
	ft_bzero(pckt, PING_SIZE);
	ft_bzero(pckt_res, PING_SIZE);

	// send icmp packet in an infinite loop
	while (pingloop && data->count)
	{
		if (data->count > 0)
			data->count--;
		// flag is whether packet was sent or not
		flag = 1;

		if (msg_count) {
			sleep(data->interval.tv_sec);
			usleep(data->interval.tv_usec*1000);
		}

		// send packet
		if (data->deadline > 0 && tv_start.tv_sec - tv_fs.tv_sec >= data->deadline) {
			pingloop = 0;
			break;
		}
		ft_bzero(pckt, PING_SIZE);
		pckt->hdr.type = ICMP_ECHO;
		pckt->hdr.code = 0;
		pckt->hdr.rest.echo.id = pid;
		pckt->hdr.rest.echo.sequence = msg_count++;
		pckt->hdr.checksum = checksum(pckt, PING_SIZE);
		gettimeofday(&tv_start, NULL);
		if ((sendto(data->sockfd, pckt, PING_SIZE, MSG_DONTWAIT, data->ip_addr->ai_addr, sizeof(*data->ip_addr->ai_addr))) != (int)PING_SIZE)
		{
			printf("\nPacket Sending Failed!\n");
			flag = 0;
		}
		if (flag)
		{
			if (receive_pckt(&res, pckt_res, data) > 0)
			{
				res.size -= IP_HDR;
				if ((pckt_res->hdr.type == ICMP_TIME_EXCEEDED && pckt_res->hdr.code == ICMP_EXC_TTL))
				{
					char src_ip[INET_ADDRSTRLEN];
					char src_name[NI_MAXHOST];
					int state;
					struct sockaddr_in *src_in = (struct sockaddr_in *)&(res.sa);
					ft_strcpy(src_ip, inet_ntoa(src_in->sin_addr));
					if ((state = getnameinfo(&(res.sa), sizeof(res.sa), src_name, NI_MAXHOST, NULL, 0, NI_NUMERICSERV)))
						printf("getnameinfo failed %d %s\n", state, gai_strerror(state));
					printf("%d bytes from %s (%s): Time to live exceeded\n",
						res.size, src_name, src_ip);
					if (data->verbose)
					{
						print_HdrDump((void*)pckt_res + PING_HDR);
						printf("ICMP: type %d, code %d, size %ld, id 0x%04x, seq 0x0%03x\n",
							pckt->hdr.type, pckt->hdr.code, PING_SIZE, pid, msg_count
						);
					}
				}
				else if (pckt_res->hdr.type == ICMP_ECHOREPLY)
				{
					gettimeofday(&tv_end, NULL);
					double timeElapsed = ((double)(tv_end.tv_usec - tv_start.tv_usec)) / 1000.0;
					double *rtt_msec = (double*)malloc(sizeof(double));
					*rtt_msec = (tv_end.tv_sec - tv_start.tv_sec) * 1000.0 + timeElapsed;
					ft_lstadd_back(&lst_values, ft_lstnew(rtt_msec));
					printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
							res.size, data->hostaddr,
							pckt_res->hdr.rest.echo.sequence, res.ttl, *rtt_msec);
					ft_bzero(pckt_res, PING_SIZE);
					res.size = 0;
				}
				else
				{
					printf("Error..Packet received with ICMP type %d code %d\n", pckt_res->hdr.type, pckt_res->hdr.code);
				}
			}
		}
	}
	gettimeofday(&tv_fe, NULL);
	print_stats(lst_values, msg_count, data->hostname);

	ft_lstclear(&lst_values, free);
	free(pckt);
	free(pckt_res);
}

