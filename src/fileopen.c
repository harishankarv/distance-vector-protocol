#include "../include/global.h"


void fileopen(char *path)
{

	char ch, *token;
	FILE *fp;
	int arg_c;
	char arg_v[MAXARGS][MAXARGLENGTH];
	char buffer[MAXTOTALLENGTH];
	
   fp = fopen(path,"r"); // read mode

   if( fp == NULL )
   {
   	perror("Error while opening the file.\n");
   	exit(1);
   }

   
   arg_c=0;
   bzero(&arg_v, MAXARGS*MAXARGLENGTH );
   
   //read topology file into buffer
   fread(buffer, MAXTOTALLENGTH, 1 ,fp );
   
   //tokenize and store in buffer[][]
   token = strtok(buffer, "\n ");

   while( (token != NULL) ) 
   {
   	strcpy(arg_v[arg_c] ,token);
   	arg_v[arg_c][strlen(token)] = '\0';  
   	token = strtok(NULL, "\n ");
   	arg_c++;
   }
   
   fclose(fp);

   //store values from topology files into globals
   MYID = atoi(arg_v[arg_c-3]);
   NUMSERVERS = atoi(arg_v[0]);
   NUMNEIGHBOURS = atoi(arg_v[1]);

   bzero( &SERVERLIST, ( (sizeof(struct serverlist)) * MAXSERVERS ) );

   int i,j,k;	

 	//copy values from buffer into struct SERVERLIST
   for (i = 0, j =2; i<NUMSERVERS; i++)
   {	
   	for(k = 0; k< NUMSERVERS; k++)
   	{
   		if ( atoi((arg_v[j + 3*k])) == i+1)
   		{
				SERVERLIST[i].id = atoi(arg_v[j + 3*k]);		//store id
				SERVERLIST[i].port = atoi(arg_v[j+ 3*k + 2]);  //store port
				inet_pton(AF_INET, arg_v[j+ 3*k + 1], &SERVERLIST[i].ip); //store ip in network byte order 		

				if (SERVERLIST[i].id == MYID) //if loop is running for self
					MYPORT = SERVERLIST[i].port; //save listening port info into MYPORT
			}
		}

	}


	/*initially set all 
	costs to infinty and cost to self as 0
	nexthops to -1 and nexthop to self as self
	num_intervals to -1
	last_intervals to -1*/

	for (i = 0; i<NUMSERVERS; i++)
	{	
		if (SERVERLIST[i].id == MYID) //if loop is running for self
		{
			SERVERLIST[i].cost = 0; //cost to self is 0
			SERVERLIST[i].nexthop = MYID; //next hop to self is self
			
			SERVERLIST[i].num_intervals = -1;
			SERVERLIST[i].last_interval = -1;
			continue;
		}
		
		SERVERLIST[i].cost = 65535; //cost to others is infinity		
		SERVERLIST[i].nexthop = -1; //nexthop to others is -1			
		
		SERVERLIST[i].num_intervals = -1;
		SERVERLIST[i].last_interval = -1;
	}

	/*now change the costs for neighbours from topology file, 
	change nexthop and set isneighbour to 1*/

	int count = 0;

	for (i = 0, j =2+(3*NUMSERVERS); i<NUMNEIGHBOURS; i++)
	{
		for (k=0; k<NUMSERVERS; k++)
		{
			int thisid = atoi(arg_v[j+1]);
			int thiscost = atoi(arg_v[j+2]);
			
			if (SERVERLIST[k].id == thisid)
			{
				SERVERLIST[k].cost = thiscost;
				SERVERLIST[k].nexthop = thisid;
				SERVERLIST[k].isneighbour = 1;
				j+=3;
			}

		}	
	}

	print_parsed_topology_file();
	
	//initially set all values in routing table to inf
	for(i = 0; i < MAXSERVERS; i++)
	{
		for(j = 0; j < MAXSERVERS; j++)
		{  
			ROUTINGTABLE[i][j] = 65535;
		}
	}
	
	//change own row in routing table
	for (i = 0; i<NUMSERVERS; i++)
	{
		ROUTINGTABLE[MYID-1][i] = SERVERLIST[i].cost;
	}
	
	// calculate the size update packet we will send or receive
	sizeofupdatepacket = ( (sizeof(struct packetheader)) + (NUMSERVERS*(sizeof(struct packetfields))) );
	
	//allocate memory for the update packet and receive buffer
	updatepacket = malloc(sizeofupdatepacket);
	bzero(updatepacket, sizeofupdatepacket);	

	receivebuffer = malloc(sizeofupdatepacket);
	bzero(receivebuffer, sizeofupdatepacket);	
	
	print_routing_table();
	print_next_hops() ;
}

void print_routing_table(){

	int i, j;
	printf("\nRouting table:\n");
	for (i =0; i<NUMSERVERS; i++)
	{
		for (j =0; j<NUMSERVERS ; j++)
		{
			printf("%d\t", ROUTINGTABLE[i][j] );
		}
		printf("\n");
	}
}

void print_next_hops() {
	int i;
	printf("\nNext hops:\n");
	for (i =0; i<NUMSERVERS; i++)
	{
		printf("%d  ", SERVERLIST[i].nexthop );
	}
	printf("\n");
}

void print_parsed_topology_file() {
	int i ;
	printf("\nParsed topology file:\n");
	printf("ID\tCOST\tNEXTHOP\tPORT\tIP\t\tISNEIGHBOUR\n");
	for (i=0; i<NUMSERVERS; i++)
	{
		printf("%d\t", SERVERLIST[i].id);
		printf("%d\t", SERVERLIST[i].cost);
		printf("%d\t", SERVERLIST[i].nexthop);
		printf("%d\t", SERVERLIST[i].port);
		printf("%d\t\t", SERVERLIST[i].ip);
		printf("%d\n", SERVERLIST[i].isneighbour);
	}
}