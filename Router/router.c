#include "queue.h"
#include "skel.h"

#define MAX_RTABLE_LEN 100000
#define MAX_ATABLE_LEN 1000

int LPM(struct route_table_entry *route_tab, int route_len, unsigned int ip)
{
	int poz = -1;
	int mask = 0;
	int s = 0, d = route_len-1;
	while(s <= d)
	{
		int mij = (s + d) / 2;
		int net = (ip & route_tab[mij].mask);
		if(ntohl(net) == ntohl(route_tab[mij].prefix))
		{
			if(ntohl(route_tab[mij].mask) > ntohl(mask))
			{
				poz = mij;
				mask = route_tab[mij].mask;
			}
			s = mij + 1;
		}
		else
		{
			if(ntohl(ip) < ntohl(route_tab[mij].prefix))
				d = mij - 1;
			if(ntohl(ip) > ntohl(route_tab[mij].prefix))
				s = mij + 1;
		}
	}
	return poz;
}
int find_mac(struct arp_entry *arp_tab, int arp_len, unsigned int ip)
{
	int poz = -1;
	for(int i=0; i<arp_len; i++)
	{
		if(ntohl(ip) == ntohl(arp_tab[i].ip))
			poz = i;
	}
	return poz;
}
int comparator(const void *r1, const void *r2)
{
    struct route_table_entry R1 = *((struct route_table_entry*)r1);
    struct route_table_entry R2 = *((struct route_table_entry*)r2);
    if(ntohl(R1.prefix) == ntohl(R2.prefix))
    {
    	if(ntohl(R1.mask) > ntohl(R2.mask))
    		return 1;
    	else
    		return -1;
    }
    else
    {
    	if(ntohl(R1.prefix) > ntohl(R2.prefix))
    		return 1;
    	else
    		return -1;
    }
}
void icmp(packet m, uint8_t type, uint8_t code) {
	struct ether_header *eth = (struct ether_header*) m.payload;
	memcpy(eth->ether_dhost, eth->ether_shost, 6);
	get_interface_mac(m.interface, eth->ether_shost);
	m.len = sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct icmphdr);

	struct iphdr *ip_head = (struct iphdr*)(m.payload + sizeof(struct ether_header));
	ip_head->ihl = 5;
	ip_head->version = IPVERSION;
	ip_head->tos = 0;
	ip_head->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr));
	ip_head->id = htons(1);
	ip_head->frag_off = 0;
	ip_head->ttl = 64;
	ip_head->protocol = IPPROTO_ICMP;
	ip_head->daddr = ip_head->saddr;
	unsigned int ip = inet_addr(get_interface_ip(m.interface));
	ip_head->saddr = ip;
	ip_head->check = 0;
	ip_head->check = ip_checksum((uint8_t*)ip_head, sizeof(struct iphdr));

	struct icmphdr *icmp_head = (struct icmphdr*)(m.payload + sizeof(struct ether_header) + sizeof(struct iphdr));
	icmp_head->type = type;
	icmp_head->code = code;
	icmp_head->checksum = 0;
	icmp_head->checksum = icmp_checksum((uint16_t*)icmp_head, sizeof(struct icmphdr));
	send_packet(&m);
}
int ip_packet(packet* m, struct route_table_entry* route_tab, struct arp_entry* arp_tab, int route_len, int arp_len)
{
	struct ether_header *eth = (struct ether_header*) m->payload;
	struct iphdr *ip_head = (struct iphdr*)(m->payload + sizeof(struct ether_header));

	if (ip_checksum((uint8_t*)ip_head, sizeof(struct iphdr)) != 0)
		return 0;

	if (ip_head->ttl < 2)
		return -1;

	if(ip_head->protocol == IPPROTO_ICMP)
	{
		struct icmphdr *icmp_head = (struct icmphdr*)(m->payload + sizeof(struct ether_header) + sizeof(struct iphdr));
		if(icmp_head->type == 8 && icmp_head->code == 0)
		{
			unsigned int ip = inet_addr(get_interface_ip(m->interface));
			if(ip_head->daddr == ip)
			return -2;
		}
	}

    int poz_route = LPM(route_tab, route_len, ip_head->daddr);
	struct route_table_entry route;
	if(poz_route == -1)
		return -3;
	else
        route = route_tab[poz_route];

	(ip_head->ttl)--;
	ip_head->check = 0;
	ip_head->check = ip_checksum((uint8_t*)ip_head, sizeof(struct iphdr));

    int poz_arp = find_mac(arp_tab, arp_len, route.next_hop);
	if(poz_arp == -1)
		return 0;
	else {
	    struct arp_entry arp = arp_tab[poz_arp];
	    get_interface_mac(route.interface, eth->ether_shost);
		m->interface = route.interface;
		memcpy(eth->ether_dhost, arp.mac, 6);
		send_packet(m);
	}
	return 0;
}

int main(int argc, char *argv[])
{
	packet m;
	int rc;

	init(argc - 2, argv + 2);

	struct route_table_entry *route_tab = malloc(sizeof(struct route_table_entry) * MAX_RTABLE_LEN);
	int route_len = read_rtable(argv[1], route_tab);
	qsort(route_tab, route_len, sizeof(struct route_table_entry), comparator);

	struct arp_entry *arp_tab = malloc(sizeof(struct arp_entry) * MAX_ATABLE_LEN);
	int arp_len = parse_arp_table("arp_table.txt", arp_tab);

	while (1) {
		rc = get_packet(&m);
		DIE(rc < 0, "get_packet");

		struct ether_header *eth = (struct ether_header*) m.payload;

		unsigned char *mac = malloc(sizeof(unsigned char) * 6);
		get_interface_mac(m.interface, mac);
		if(memcmp(mac, eth->ether_dhost, 6) != 0)
			continue;

		if (ntohs(eth->ether_type) == 2048) {
			int err = ip_packet(&m, route_tab, arp_tab, route_len, arp_len);
			if(err == 0)
				continue;
			if(err == -1)
				icmp(m, 11, 0);
			if(err == -2)
				icmp(m, 0, 0);
			if(err == -3)
				icmp(m, 3, 0);
		}
	}
}
