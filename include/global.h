#ifndef GLOBAL_H_
#define GLOBAL_H_

#define HOSTNAME_LEN 128
#define MAXARGS 50
#define MAXARGLENGTH 100
#define MAXTOTALLENGTH 1000
#define MAXSERVERS 10

#define MAXTOTALLENGTH_2 500
#define MAXARGS_2 5
#define MAXARGLENGTH_2 100


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

uint16_t NUMSERVERS;
uint16_t NUMNEIGHBOURS;

uint16_t MYID;
uint16_t MYPORT;

int INTERVAL;

int NUMRCVD;

size_t sizeofupdatepacket;

void *receivebuffer;
void *updatepacket;

struct packetfields
{
	uint32_t ip;
	uint16_t port;
	uint16_t zero;
	uint16_t id;
	uint16_t cost;
};

struct packetheader
{
	uint16_t numfields;
	uint16_t myport;
	uint32_t myip;
};

struct serverlist
{
	uint16_t id;
	uint16_t cost;
	int16_t nexthop;
	uint16_t port;
	uint32_t ip; 
	uint16_t isneighbour;
	int16_t num_intervals;
	int16_t last_interval;
}SERVERLIST[MAXSERVERS];

uint16_t ROUTINGTABLE[MAXSERVERS][MAXSERVERS];
int sizeofroutingtable;

int CURRENT_INTERVAL;

int error_code;

void fileopen(char *path);
void server();
void makepacket();
void sendpacket(int sockfd);
void bellmanford();
void print_routing_table();
void print_parsed_topology_file();
void print_next_hops() ;

#endif


