// compile as -o ping
// run as sudo ./ping <hostname>

#include "ft_ping.h"
 
// Define the Ping Loop
int pingloop = 1;
 
// Interrupt handler
void intHandler(int dummy)
{
    (void)dummy;
    pingloop=0;
}

// Driver Code
int main(int argc, char *argv[])
{
    ping_data data;
    struct addrinfo *ip_addr;
    int status;
 
    if(argc!=2)
    {
        printf("%s: usage error: Destination address required\n", argv[0]);
        return (1);
    }
    status = dns_lookup(argv[1], &ip_addr);
    if(status != 0)
    {
        printf("%s: %s: %s\n", argv[0], argv[1], gai_strerror(status));
        return (2);
    }
    if ((data.is_addr = is_valid_ipv4(argv[1])) == 0) {
        printf("IP for %s:\n", argv[1]);
        for (struct addrinfo *p = ip_addr; p != NULL; p = p->ai_next) {
            if (p->ai_family == AF_INET) {
                data.ip_addr = p;
                data.reverse_hostname = reverse_dns_lookup(p);
                break;
            }
        }
    }
    data.host = argv[1];
 
    //socket()
    data.sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(data.sockfd < 0)
    {
        printf("\nSocket file descriptor not received!!\n");
        fprintf(stderr, "%s: %s: %s\n", argv[0], argv[1], strerror(errno));
        return 0;
    }
 
    signal(SIGINT, intHandler);//catching interrupt
 
    //send pings continuously
    send_ping(&data);
    freeaddrinfo(ip_addr);
     
    return 0;
}