#include "ft_ping.h"

int	parse_arg(int ac, char **av) {
	for (int i = 1; i < ac; i++) {
		if (ft_strcmp(av[1], "-V") == 0) {
			ft_putendl_fd("ping from ft_ping v1", 1);
			exit(0);
		} else if (ft_strcmp(av[1], "-v") == 0) {
			;
		} else if (ft_strcmp(av[1], "-h") == 0) {
			return (1);
		}
	}
	return (0);
}

void send_ping(ping_data *data)
{
    int ttl_val=64, msg_count=0, flag=1,
               msg_received_count=0;
    long unsigned int i;
	socklen_t addr_len;
    struct ping_pkt pckt;
    struct sockaddr_in r_addr;
    // struct timespec time_start, time_end, tfs, tfe;

    long double rtt_msec=0;
    long double total_msec=0;
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
                   (const char*)&tv_out, sizeof tv_out);
 
    // send icmp packet in an infinite loop
    while(pingloop)
    {
        // flag is whether packet was sent or not
        flag=1;
      
        //filling packet
        ft_bzero(&pckt, sizeof(pckt));
         
        pckt.hdr.type = ICMP_ECHO;
        pckt.hdr.rest.echo.id = getpid();
         
        for (i = 0; i < sizeof(pckt.msg)-1; i++ )
            pckt.msg[i] = i+'0';
         
        pckt.msg[i] = 0;
        pckt.hdr.rest.echo.sequence = msg_count++;
        pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));
 
 
        usleep(PING_SLEEP_RATE);
 
        //send packet
        gettimeofday(&tv_start, NULL);
        if (sendto(data->sockfd, &pckt, sizeof(pckt), 0,
           (struct sockaddr*) ping_addr,
            sizeof(*ping_addr)) <= 0)
        {
            printf("\nPacket Sending Failed!\n");
            flag=0;
        }
 
        //receive packet
        addr_len=sizeof(r_addr);
 
        if ( recvfrom(data->sockfd, &pckt, sizeof(pckt), 0,
             (struct sockaddr*)&r_addr, &addr_len) <= 0
              && msg_count>1)
        {
            printf("\nPacket receive failed!\n");
        }
 
        else
        {
            gettimeofday(&tv_end, NULL);

            double timeElapsed = ((double)(tv_end.tv_usec - tv_start.tv_usec))/1000.0;
            rtt_msec = (tv_end.tv_sec-tv_start.tv_sec) * 1000.0 + timeElapsed;
             
            // if packet was not sent, don't receive
            if(flag)
            {
                if(!(pckt.hdr.type ==69 && pckt.hdr.code==0))
                {
                    printf("Error..Packet received with ICMP type %d code %d\n", pckt.hdr.type, pckt.hdr.code);
                }
                else
                {
                    printf("%d bytes from %s (%s) icmp_seq=%d ttl=%d time=%.2Lf ms\n",
                          PING_PKT_S, data->reverse_hostname, data->hostaddr,
                          msg_count, ttl_val, rtt_msec);
 
                    msg_received_count++;
                }
            }
        }   
    }
    gettimeofday(&tv_fe, NULL);
    double timeElapsed = ((double)(tv_fe.tv_usec - tv_fs.tv_usec))/1000.0;
     
    total_msec = (tv_fe.tv_sec-tv_fs.tv_sec) * 1000.0 + timeElapsed;
                    
    printf("\n=== %s ping statistics ===\n", data->hostname);
    printf("\n%d packets sent, %d packets received, %.0f%% packet loss. Total time: %.0Lf ms.\n\n",
           msg_count, msg_received_count,
           ((msg_count - msg_received_count)/msg_count) * 100.0,
          total_msec);
}
 