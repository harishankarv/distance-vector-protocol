#include "../include/global.h"

void makepacket()
{
	updatepacket = malloc(sizeofupdatepacket);
	bzero(updatepacket, sizeofupdatepacket);

	//header part
	uint16_t NUMSERVERS_n = htons(NUMSERVERS);
	memcpy(updatepacket, &NUMSERVERS_n, 2);	//copy numfields
	
	int i;
	for (i = 0; i<NUMSERVERS; i++)
	{	
		if (SERVERLIST[i].id == MYID) //if loop is running for self
		{
			uint16_t port_n = htons(SERVERLIST[i].port); //convert port to network byte order
			uint32_t ip_n = SERVERLIST[i].ip; //ip addr is already stored in network byte order
			memcpy( ( (  (uint16_t*)updatepacket) + 1 ), &port_n, 2);
			memcpy( ( (  (uint16_t*)updatepacket) + 2 ), &ip_n, 4);
			break;
		}
	}
	
	//payload part
	for (i = 0; i<NUMSERVERS; i++)
	{	
		uint32_t ip_n = SERVERLIST[i].ip; //ip addr is already stored in network byte order
		uint16_t port_n = htons(SERVERLIST[i].port); //convert port to network byte order
		uint16_t id_n = htons(SERVERLIST[i].id); //convert id to network byte order
		uint16_t cost_n = htons(ROUTINGTABLE[MYID-1][i]); //convert cost to network byte order

		memcpy( ( (  (uint16_t*)updatepacket) + 4 + i*6 ), &ip_n, 4);
		memcpy( ( (  (uint16_t*)updatepacket) + 6 + i*6 ), &port_n, 2);
		memcpy( ( (  (uint16_t*)updatepacket) + 8 + i*6 ), &id_n, 2);
		memcpy( ( (  (uint16_t*)updatepacket) + 9 + i*6 ), &cost_n, 2);
	}

	printf("\nPacket made.\n");

}	


void sendpacket(int sockfd)
{
	socklen_t sockaddrlen_s= (sizeof(struct sockaddr_in));
	
	int i, numsent;
	int count = 0;
	for (i = 0; i<NUMSERVERS; i++)
	{
		if (count == NUMNEIGHBOURS)
			break;
		
		//send only to neighbours
		if (SERVERLIST[i].isneighbour == 1)
		{
			struct sockaddr_in to_addr;
			bzero( &to_addr, (sizeof(to_addr)) ) ;
			to_addr.sin_family = AF_INET;
			to_addr.sin_port = htons(SERVERLIST[i].port);
			to_addr.sin_addr.s_addr = SERVERLIST[i].ip; 
			
			numsent = sendto(sockfd, updatepacket, sizeofupdatepacket, 0, (struct sockaddr*)&to_addr, sockaddrlen_s);
			if (numsent < 0)
				{
			   	 	perror ("sendto error");
					continue;		
				}
			
			printf("Sent % d bytes to %d\n", numsent , SERVERLIST[i].port);
			count++;
		}
	}
}

void bellmanford()
{
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("bellmanford\n");
	int i, j, k;
	uint16_t distof_jtoi, costof_metoj;
	int index;


	print_routing_table();
	print_next_hops();

	//bellman ford algorithm start
	for (i =0; i<NUMSERVERS; i++)
	{	
		printf("i =%d \n", i);

		if (i == MYID-1)
		{
			// i dont not want to change my entry, as it will always be 0
			printf("%d...", i);
			continue;
		}

		int min_val = 9999999;
		int min_index = 0;
		int changed =0;

		for (j=0; j<NUMSERVERS; j++)
		{
			printf("j =%d ", j);
			
			if (j == MYID-1 || (SERVERLIST[j].isneighbour!= 1) )
			{
				// i dont not want to route through myself
				printf("%d...\n", j);
				continue;
			}

			costof_metoj = 	SERVERLIST[j].cost;
			distof_jtoi = ROUTINGTABLE[j][i];
			printf("costof_metoj = %d\n", costof_metoj);
			printf("distof_jtoi = %d\n", distof_jtoi);
			

			if ( (min_val > (costof_metoj + distof_jtoi)) && (costof_metoj != 65535) && (distof_jtoi != 65535) )
			{
				min_val = costof_metoj + distof_jtoi;
				min_index = j;	
				changed = 1;			
			}
			
		}		
		
		printf("min_val = %d\n", min_val);
		printf("min_index = %d\n", min_index );		
		
		if (changed ==1 )
		{
			ROUTINGTABLE[MYID-1][i] = min_val;
			SERVERLIST[i].nexthop = SERVERLIST[min_index].id;			
		}	
	}

	print_routing_table();
	print_next_hops();

	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}


