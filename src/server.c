#include "../include/global.h"



void server()
{
	int sockfd;
	int getaddrval;
	int numreceived;

	/*Reference:
	UNIX Network Programming : Networking APIs : Sockets and XTI : Volume 1, Second Edition,
	W. Richard Stevens, Prentice Hall, Oct 1997, ISBN: 013490012X
	Elementary UDP sockets pg 215
	Select Concept pg 162*/

	struct addrinfo hints, *result, *p;
	struct sockaddr_in their_addr, my_addr;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_INET; //ipv4
	hints.ai_socktype = SOCK_DGRAM; //udp
	hints.ai_flags = AI_PASSIVE; //use my ip

	//get listening port from myport variable

	char port[10];
	bzero(&port, sizeof(port));
	sprintf(port, "%d", MYPORT);
	
	//getaddrinfo concept taken from 
	//http://beej.us/guide/bgnet/output/html/multipage/syscalls.html#getaddrinfo

	if ((getaddrval = getaddrinfo(NULL, port, &hints, &result)) < 0)
	{
		printf ("%d\n",getaddrval);
		perror ("getaddrinfo() error");
		exit(-1);
	}

	p=result;
	// try to create socket() and bind() for each element in result
	do
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
		{
			printf ("%d\n", sockfd);
			perror ("socket() error");
			exit(-1);
		}
		
		if ((bind (sockfd, p->ai_addr, p->ai_addrlen)) < 0)
		{	
			close(sockfd);
			printf ("%d\n",sockfd);
			perror ("bind() error");
			exit(-1);
		} 	

		break; //if successful, break

		p=p->ai_next;

	} while(p->ai_next !=NULL);


	if (p==NULL) 
	{
		printf("socket() and/or bind() not successful\n");
		exit(-1);
	}

	printf("\nListening port %s:\n", port);
	printf("\n----------------------------------------\n");

	
	/*socklen_t sockaddrlen_s= (sizeof(struct sockaddr_in));*/
	socklen_t sockaddrlen_r;
	
	fd_set fdset;
	struct timeval tv;
	
	tv.tv_sec = INTERVAL;
	tv.tv_usec = 0;

	int ready;
	int i, j;

	while(1)
	{	
		printf("\nselect()          time: %d\n", (int)tv.tv_sec);

		error_code = 0;
		FD_ZERO(&fdset);
		FD_SET(sockfd, &fdset); //to monitor listening socket
		FD_SET(0, &fdset); //to monitor standard input


		ready = select (sockfd+1, &fdset, NULL, NULL, &tv);
		
		if (ready == 0) //timeout. send update packet to all neighbours.
		{
			printf("select() timeout! time: %d\n", (int)tv.tv_sec);
			
			print_routing_table();
			print_next_hops();


			CURRENT_INTERVAL++;
			int diff = 0;

			for (i =0; i<NUMSERVERS; i++)
			{	
				if (SERVERLIST[i].num_intervals == -1)
				{
					continue;
				}

				if (SERVERLIST[i].isneighbour == 1)
				{

					diff = CURRENT_INTERVAL - SERVERLIST[i].last_interval;
					printf("ID %d diff = %d\n", SERVERLIST[i].id, diff);

					if (diff >1)
					{
						SERVERLIST[i].num_intervals++;
					}

	   				//no message for 3 consecutive intervals
					if(SERVERLIST[i].num_intervals >= 3)
					{
						SERVERLIST[i].cost = 65535;
						SERVERLIST[i].nexthop = -1;
						printf("ID %d->cost changed to 65535 \n",SERVERLIST[i].id );

						bellmanford();
					}

				}
			}

			makepacket();
			sendpacket(sockfd);

			tv.tv_sec = INTERVAL;
			tv.tv_usec = 0;

			printf("----------------------------------------\n");
		}

		else if (ready > 0 )
		{
			if ( FD_ISSET (sockfd, &fdset) ) // received an  updatepacket
			{	
				printf("----------------------------------------\n");
				numreceived = recvfrom (sockfd, receivebuffer, sizeofupdatepacket, 0, (struct sockaddr*)&their_addr, &sockaddrlen_r);
				
				if (numreceived < 0)
				{
					perror ("recvfrom error");
					continue;		
				}

				
				//copy header part into memory pointed by a struct packetheader pointer
				//copy payload part into memory pointed by a struct packetfields pointer
				struct packetheader* local_packetheader;
				struct packetfields* local_packetfields;
				local_packetheader = malloc(sizeof(struct packetheader));
				memcpy(local_packetheader, receivebuffer, (sizeof(struct packetheader)));
				uint16_t local_numfields = ntohs(local_packetheader->numfields);
				local_packetfields = malloc((sizeof(struct packetfields))*local_numfields);
				memcpy(local_packetfields,  (uint16_t*)receivebuffer + 4, local_numfields*(sizeof(struct packetfields)));

				//now store the data in another struct packetheader and struct packetfields
				//by converting the data from network byte order to host byte order 
				struct packetheader received_packetheader;
				struct packetfields received_packetfields[local_numfields];
				received_packetheader.numfields = ntohs(local_packetheader->numfields);
				received_packetheader.myport = ntohs(local_packetheader->myport);
				received_packetheader.myip = local_packetheader->myip;
				
				for (i=0; i<local_numfields; i++)
				{
					received_packetfields[i].ip 	 = 	(local_packetfields+i)->ip;
					received_packetfields[i].port = 	ntohs((local_packetfields+i)->port);
					received_packetfields[i].zero = 	ntohs((local_packetfields+i)->zero);
					received_packetfields[i].id 	 = 	ntohs((local_packetfields+i)->id);
					received_packetfields[i].cost = 	ntohs((local_packetfields+i)->cost);
				}

				/*find id of server which sent the packet 
				by findng the one which has cost = 0*/
				int this_serverid;
				for (i=0; i<local_numfields; i++)
				{
					if (received_packetfields[i].cost == 0)
						this_serverid = received_packetfields[i].id;
				}
				
				//if we have received a packet from a non neighbour, drop it.
				int neighbour = 0;
				for (i =0; i<NUMSERVERS; i++)
				{
					if ( (SERVERLIST[i].id == this_serverid) && (SERVERLIST[i].isneighbour != 1) )
					{	
						printf("error: received packet from id %d: non neighbour:\ndropped\n", this_serverid);
						neighbour = 0;
						break;
					}
					
					if ((SERVERLIST[i].id == this_serverid) && (SERVERLIST[i].isneighbour == 1) )
					{
						neighbour = 1;				
						break;
					}
				}
				
				//receive packet only if neighbour
				if (neighbour == 1)
				{

					NUMRCVD++; //increase counter for number of received packets
					cse4589_print_and_log("RECEIVED A MESSAGE FROM SERVER %d\n", this_serverid); 
					
					for (i=0; i<local_numfields; i++)
					{
						cse4589_print_and_log("%-15d%-15d\n", received_packetfields[i].id, received_packetfields[i].cost);
					}	

					//as we have received a message, 
					//set num_intervals to 0
					//set last_interval to CURRENT_INTERVAL
					printf("CURRENT_INTERVAL = %d\n", CURRENT_INTERVAL);
					for (i =0; i<NUMSERVERS; i++)
					{
						if (SERVERLIST[i].id == this_serverid /*&& SERVERLIST[i].isneighbour == 1*/)
						{	
							SERVERLIST[i].num_intervals = 0;
							SERVERLIST[i].last_interval = CURRENT_INTERVAL;
							printf("ID %d->num_intervals = 0\n", i);
							printf("ID %d->last_interval = %d\n", i, CURRENT_INTERVAL);
						}				
					}

					//now copy the values we received into the routing table
					for (i = 0; i<NUMSERVERS; i++)
					{
						ROUTINGTABLE[this_serverid-1][i] = received_packetfields[i].cost;
					}

					printf("time:%d\n", (int)tv.tv_sec);
					
					bellmanford();
				}

				ready--;
				if (ready <= 0) 
				{
					continue;
				}
			}

			if ( FD_ISSET (0, &fdset) ) //user input perform command
			{
				//tokenize input and store in arg_v_2 buffer
				int arg_c_2;
				char arg_v_2[MAXARGS_2][MAXARGLENGTH_2];
				char buffer_2[MAXTOTALLENGTH_2];
				char *token;
				
				arg_c_2=0;
				bzero(&arg_v_2, sizeof(MAXARGS*MAXARGLENGTH));

				fgets(buffer_2, MAXTOTALLENGTH, stdin);
				token = strtok(buffer_2, "\n ");

				while( token != NULL ) 
				{
					strcpy(arg_v_2[arg_c_2] ,token);
					arg_v_2[arg_c_2][strlen(token)] = '\0';  
					token = strtok(NULL, "\n ");
					arg_c_2++;
				}

			   //compare tokens for each command
				if ((!strcmp(arg_v_2[0], "update")) || (!strcmp(arg_v_2[0], "UPDATE")))
				{	
					int new_cost, my_id, this_serverid;

					if ( (!strcmp(arg_v_2[3], "inf")) || (!strcmp(arg_v_2[0], "INF")) )
						new_cost = 65535;
					else
						new_cost = atoi(arg_v_2[3]);

					my_id = atoi(arg_v_2[1]);
					this_serverid = atoi(arg_v_2[2]);
					printf("id = %d\n", this_serverid);

			   		//check if entered id for me is correct
					if (my_id != MYID)
						cse4589_print_and_log("%s:%s\n", arg_v_2[0], "ERROR:Please enter a valid server ID"); 

					int neighbour = 0;

					for (i =0; i<NUMSERVERS; i++)
					{
						if ( (SERVERLIST[i].id == this_serverid) && (SERVERLIST[i].isneighbour != 1) )
						{	
							neighbour = 0;
							break;
						}
						
						if ((SERVERLIST[i].id == this_serverid) && (SERVERLIST[i].isneighbour == 1) )
						{
							neighbour = 1;				
							break;
						}
					}

					if(neighbour == 0)
					{
						cse4589_print_and_log("%s:%s\n", arg_v_2[0], "ERROR:Link cost can be updated only for neighbours"); 
					}


			   		//no error in command
			   		// change the corresponding link cost
					if ( neighbour == 1 ) 
					{

						for(i =0; i<NUMSERVERS; i++)
						{
							if(SERVERLIST[i].id == this_serverid)
							{
				   				SERVERLIST[i].cost = new_cost;	//change for for given id
				   			}
				   		}

				   		cse4589_print_and_log("%s:SUCCESS\n",arg_v_2[0]);
				   		fflush(stdout);
				   		bellmanford();
				   	} 
				   }

				   else if ((!strcmp(arg_v_2[0], "step")) || (!strcmp(arg_v_2[0], "STEP")))
				   {
				   	makepacket();
				   	sendpacket(sockfd);
				   	cse4589_print_and_log("%s:SUCCESS\n",arg_v_2[0]);
				   }

				   else if ((!strcmp(arg_v_2[0], "packets")) || (!strcmp(arg_v_2[0], "PACKETS")))
				   {
				   	cse4589_print_and_log("%s:SUCCESS\n",arg_v_2[0]);
				   	cse4589_print_and_log("%d\n", NUMRCVD);
				   	NUMRCVD = 0;
				   }

				   else if ((!strcmp(arg_v_2[0], "display")) || (!strcmp(arg_v_2[0], "display")))
				   {	
				   	cse4589_print_and_log("%s:SUCCESS\n",arg_v_2[0]);

				   	for (i =0; i<NUMSERVERS; i++)
				   	{
				   		cse4589_print_and_log("%-15d%-15d%-15d\n", SERVERLIST[i].id, SERVERLIST[i].nexthop, ROUTINGTABLE[MYID-1][i]);
				   		fflush(stdout);
				   	}
				   }

				   else if ((!strcmp(arg_v_2[0], "disable")) || (!strcmp(arg_v_2[0], "DISABLE")))
				   {	
			 		//check if neighbour
				   	int this_serverid = atoi(arg_v_2[1]);

			   		//check if neighbour
				   	int neighbour = 0;
				   	for (i =0; i<NUMSERVERS; i++)
				   	{
				   		if ( (SERVERLIST[i].id == this_serverid) && (SERVERLIST[i].isneighbour != 1) )
				   		{	
				   			neighbour = 0;
				   			break;
				   		}

				   		if ((SERVERLIST[i].id == this_serverid) && (SERVERLIST[i].isneighbour == 1) )
				   		{
				   			neighbour = 1;				
				   			break;
				   		}
				   	}

				   	if(neighbour == 0)
				   	{
				   		cse4589_print_and_log("%s:%s\n", arg_v_2[0], "ERROR:Only links to neighbours can be disabled"); 
				   	}

			   		if (neighbour == 1) //no error in command
			   		{
			   			//disable a link = it is not a neighbour anymore. set costs to inf	
			   			for(i =0; i<NUMSERVERS; i++)
			   			{
			   				if (SERVERLIST[i].id == this_serverid)
			   				{	
			   					SERVERLIST[i].isneighbour = 0;
			   					SERVERLIST[i].cost = 65535;
			   					SERVERLIST[i].nexthop = -1;
			   					SERVERLIST[i].num_intervals = -1;
			   					SERVERLIST[i].last_interval = -1;
			   				}
			   			}

			   			bellmanford();
			   			printf("link disabled to %d. Is not a neighbour anymore.\n", this_serverid);

			   			cse4589_print_and_log("%s:SUCCESS\n",arg_v_2[0]);

			   		}	

			   	}


			   	else if ((!strcmp(arg_v_2[0], "crash")) || (!strcmp(arg_v_2[0], "CRASH")))
			   	{	
			   		while(1)
			   		{

			   		}
			   	}

			   	else if ((!strcmp(arg_v_2[0], "dump")) || (!strcmp(arg_v_2[0], "dump")))
			   	{
			   		makepacket();
			   		cse4589_dump_packet(updatepacket, sizeofupdatepacket);
			   		cse4589_print_and_log("%s:SUCCESS\n",arg_v_2[0]);
			   	}

			   	else if ( (!strcmp(arg_v_2[0], "academic_integrity")) || (!strcmp(arg_v_2[0], "ACADEMIC_INTEGRITY")) )
			   	{
			   		cse4589_print_and_log("I have read and understood the course academic integrity policy located at\nhttp://www.cse.buffalo.edu/faculty/dimitrio/courses/cse4589_f14/index.html#integrity"); 
			   	}

			   	else
			   	{
			   		cse4589_print_and_log("%s:%s\n", arg_v_2[0], "ERROR:Please enter a valid command"); 

			   	}

			   	ready--;
			   	if (ready <= 0) 
			   	{
			   		continue;
			   	}	

			   }
			}	
		}	
	}
