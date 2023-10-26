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

// Driver Code
int main(int argc, char *argv[])
{
    ping_data data;
    struct addrinfo *ip_addr;
    int status;

    parse_arg(argc, argv, &data);
    status = dns_lookup(data.hostname, &ip_addr);
    if (status != 0)
    {
        printf("%s: %s: %s\n", argv[0], data.hostname, gai_strerror(status));
        return (2);
    }
    if ((data.is_addr = is_valid_ipv4(data.hostname)) == 0)
    {
        // printf("[%d] IP for %s:\n", getpid(), data.hostname); TODEL
        for (struct addrinfo *p = ip_addr; p != NULL; p = p->ai_next)
        {
            if (p->ai_family == AF_INET)
            {
                data.ip_addr = p;
                inet_ntop(AF_INET, &(((struct sockaddr_in *)p->ai_addr)->sin_addr), data.hostaddr, INET_ADDRSTRLEN);
                data.reverse_hostname = reverse_dns_lookup(p);
                break;
            }
        }
    }

    // struct icmp_filter filter;
    // filter.data = ~((1 << ICMP_ECHOREPLY));
    data.sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (data.sockfd < 0)
    {
        printf("\nSocket file descriptor not received!!\n");
        fprintf(stderr, "%s: %s: %s\n", argv[0], argv[1], strerror(errno));
    }
    // else if (setsockopt(data.sockfd, SOL_RAW, 1, (char *)&filter, sizeof(filter)) < 0)
    // {
    //     printf("\nSocket file descriptor options not set!!\n");
    //     fprintf(stderr, "%s: %s: %s\n", argv[0], argv[1], strerror(errno));
    // }
    else
    {
        signal(SIGINT, intHandler); // catching interrupt
        // send pings continuously
        send_ping(&data);
    }
    if (!data.is_addr)
        free(data.reverse_hostname);
    freeaddrinfo(ip_addr);

    return 0;
}