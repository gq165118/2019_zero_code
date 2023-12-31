
#include <stdio.h>

#include <sys/poll.h>

#define NETMAP_WITH_LIBS
#include <net/netmap_user.h>

#pragma pack(1)


#define ETH_ADDR_LEN	6

#define PROTO_IP		0X0800
#define PROTO_ARP		0X8060
#define PROTO_UDP		17
#define PROTO_ICMP		1

struct ethhdr
{
	unsigned char h_dst[ETH_ADDR_LEN];
	unsigned char h_src[ETH_ADDR_LEN];
	unsigned short h_proto;
};

struct iphdr
{
	unsigned char hdrlen:4,
				  version:4;
				  
	unsigned char tos;
	unsigned short totlen;
	
	unsigned short id;
	unsigned short flagOffset;
	
	unsigned char ttl;
	unsigned char type;
	unsigned short check;

	unsigned int sip;

	unsigned int dip;
};

struct udphdr
{
	unsigned short sport;
	unsigned short dport;

	unsigned short length;
	unsigned short check; 
};

struct udppkt
{
	struct ethhdr eh;
    struct iphdr ip;
    struct udphdr udp;

    unsigned char data[0];   //柔性数组，数据长度
};

struct arphdr
{
	unsigned short h_type;
	unsigned short h_proto;

	unsigned char h_addrlen;
	unsigned char h_protolen;

	unsigned short oper;

	unsigned char smac[ETH_ADDR_LEN];

	unsigned int sip;

	unsigned char dmac[ETH_ADDR_LEN];

	unsigned int dip;
};

struct arppkt
{
	struct ethhdr eh;
	struct arphdr arp;
};



int str2mac(char *mac, char *str) {

	char *p = str;
	unsigned char value = 0x0;
	int i = 0;

	while (p != '\0') {
		
		if (*p == ':') {
			mac[i++] = value;
			value = 0x0;
		} else {
			
			unsigned char temp = *p;
			if (temp <= '9' && temp >= '0') {
				temp -= '0';
			} else if (temp <= 'f' && temp >= 'a') {
				temp -= 'a';
				temp += 10;
			} else if (temp <= 'F' && temp >= 'A') {
				temp -= 'A';
				temp += 10;
			} else {	
				break;
			}
			value <<= 4;
			value |= temp;
		}
		p ++;
	}

	mac[i] = value;

	return 0;
}


void echo_arp_pkt(struct arppkt *arp, struct arppkt *arp_rt, char *mac)
{
	memcpy(arp_rt, arp, sizeof(struct arppkt));

	memcpy(arp_rt->eh.h_dst, arp->eh.h_src, ETH_ADDR_LEN);
	str2mac(arp_rt->eh.h_src, mac);
	arp_rt->eh.h_proto = arp->eh.h_proto;

	arp_rt->arp.h_addrlen = 6;
	arp_rt->arp.h_protolen = 4;
	arp_rt->arp.oper = htons(2);

	str2mac(arp_rt->arp.smac, mac);
	arp_rt->arp.sip = arp->arp.dip;

	memcpy(arp_rt->arp.dmac, arp->arp.smac, ETH_ADDR_LEN);
	arp_rt->arp.dip = arp->arp.sip;
}

int main()
{

    struct nm_pkthdr h;
    struct nm_desc *nmr = nm_open("netmap:eth0", NULL, 0, NULL);
    if (nmr == NULL)
    {
        return -1;
    }

    struct pollfd pfd = {0};
    pfd.fd = nmr->fd;
    pfd.events = POLLIN;

    while (1)
    {
		int ret = poll(&pfd, 1, -1);
	    if (ret < 0) continue;

	    if (pfd.revents & POLLIN)
	    {
	        unsigned char *stream = nm_nextpkt(nmr, &h);

			struct ethhdr *eh = (struct ethhdr *)stream;
			if (ntohs(eh->h_proto) == PROTO_IP)
			{
				struct udppkt *udp = (struct udppkt *)stream;

				if (udp->ip.type == PROTO_UDP)
				{
					int udplength = ntohs(udp->udp.length);
					
					udp->data[udplength-8] = '\0';

					printf("UDP --> %s\n", udp->data);
				}
			} 
			else if (ntohs(eh->h_proto) == PROTO_ARP)
			{
				struct arppkt *arp = (struct arppkt *)stream;

				struct arppkt arp_rt;

				if (arp->arp.dip == inet_addr("192.168.218.131"))
				{
					echo_arp_pkt(arp, &arp_rt, "00:0c:29:08:68:2c");

					nm_inject(nmr, &arp_rt, sizeof(arp_rt));

					printf("arp ret\n");
				}
			}
	    }
    }
    
}








