#include "ft_ping.h"

int receive_pckt(int fd, struct ip_pkt *ippckt, struct ping_pkt *ppkt, int size)
{
	struct msghdr msg;
	struct iovec iov[1];
	struct ping_pkt tmp;
	int		flag = 1;

	while (pingloop && flag)
	{
		ft_bzero(&msg, sizeof(msg));
		msg.msg_iov = iov;
		msg.msg_iovlen = 1;
		iov[0].iov_base = ippckt;
		iov[0].iov_len = size;
		if ((recvmsg(fd, &msg, 0)) < 0) {
			return (-1);
		}
		ippckt->hdr.len = (ippckt->hdr.len >> 8) | (ippckt->hdr.len << 8);
		ft_memcpy(&tmp, (void*)ippckt + IP_HDR, size - IP_HDR); // ptr to location instead of cpy
		if (tmp.hdr.type == 0 && tmp.hdr.code == 0 && tmp.hdr.rest.echo.id == ppkt->hdr.rest.echo.id){
			flag = 0;
		} else if (tmp.hdr.type == 11 && tmp.hdr.code == 0) {
			flag = 0;
		}
	}
	ft_memcpy(ppkt, &tmp, size - IP_HDR);
	return (1);
}

void send_ping(ping_data *data)
{
	int msg_count = 0, flag = 1,
		msg_received_count = 0;
	struct ping_pkt *pckt;
	struct ip_pkt *res_ip;

	long double rtt_msec = 0;
	long double total_msec = 0;
	
	struct timeval tv_start, tv_end, tv_fs, tv_fe;
	struct sockaddr_in *ping_addr = (struct sockaddr_in *)data->ip_addr->ai_addr;

	printf("PING %s (%s) %d(%ld) bytes of data.\n", data->hostname, data->hostaddr, data->pktsize, data->pktsize+PING_HDR+IP_HDR);
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
		pckt->hdr.rest.echo.sequence = ++msg_count;
		pckt->hdr.checksum = checksum(pckt, PING_SIZE);

		if (sendto(data->sockfd, pckt, PING_SIZE, 0, (struct sockaddr *)ping_addr, sizeof(*ping_addr)) <= 0)
		{
			printf("\nPacket Sending Failed!\n");
			flag = 0;
		}

		// receive packet
		if (receive_pckt(data->sockfd, res_ip, pckt, IP_SIZE) > 0 && msg_count > 0)
		{
			gettimeofday(&tv_end, NULL);
			double timeElapsed = ((double)(tv_end.tv_usec - tv_start.tv_usec)) / 1000.0;
			rtt_msec = (tv_end.tv_sec - tv_start.tv_sec) * 1000.0 + timeElapsed;
			if (flag && pingloop)
			{
				if ((pckt->hdr.type == 11 && pckt->hdr.code == 0))
				{
					printf("From %s (%s) icmp_seq=%d Time to live exceeded\n", data->reverse_hostname, data->hostaddr, pckt->hdr.rest.echo.sequence);
				}
				else if (!(pckt->hdr.type == 0 && pckt->hdr.code == 0))
				{
					printf("Error..Packet received with ICMP type %d code %d\n", pckt->hdr.type, pckt->hdr.code);
				}
				else if (data->is_addr)
				{
					printf("%ld bytes from %s icmp_seq=%d ttl=%d time=%.2Lf ms\n",
						   PING_SIZE, data->hostaddr,
						   pckt->hdr.rest.echo.sequence, res_ip->hdr.ttl, rtt_msec);
					msg_received_count++;
				}
				else
				{
					printf("%ld bytes from %s (%s) icmp_seq=%d ttl=%d time=%.2Lf ms\n",
						   PING_SIZE, data->reverse_hostname, data->hostaddr,
						   pckt->hdr.rest.echo.sequence, res_ip->hdr.ttl, rtt_msec);
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
		   ((msg_count - msg_received_count) / msg_count) * 100.0, //Float to not truncate
		   total_msec);

	free(pckt);
	free(res_ip);
}

