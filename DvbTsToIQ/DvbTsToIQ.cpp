// DvbTsToIQ.cpp : définit le point d'entrée pour l'application console.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include "../libdvbmod/libdvbmod.h"



#ifndef WINDOWS

/* Non-windows includes */

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>     /* IPPROTO_IP, sockaddr_in, htons(), 
htonl() */
#include <arpa/inet.h>      /* inet_addr() */
#include <netdb.h>
#else 

/* Windows-specific includes */

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#endif /* WINDOWS */


FILE *input, *output;
#define BUFFER_SIZE (188*7) 

void RunWithFile()
{
	unsigned char BufferTS[BUFFER_SIZE];
	while (1)
	{
		int NbRead = fread(BufferTS, 1, BUFFER_SIZE, input);
		if (NbRead < 0) break;
		if (NbRead > 0)
		{
			if(NbRead%188!=0) fprintf(stderr, "TS alignment Error\n");
			if (BufferTS[0] != 0x47) fprintf(stderr, "TS Sync Error\n");
			for (int i = 0; i < NbRead; i += 188)
			{
				//int len = Dvbs2AddTsPacket(BufferTS + i);
				int len = DvbsAddTsPacket(BufferTS + i);
				
						
				if (NbRead == BUFFER_SIZE)
				{
					//sfcmplx *Frame = Dvbs2_get_IQ();
					sfcmplx *Frame = Dvbs_get_IQ();
					fwrite(Frame, sizeof(sfcmplx), len, output);
					
				}
				else
					fprintf(stderr, "Incomplete UDP\n");
				
			}
		}
	}

}

#define DEST_PORT 10000
#define RECV_IP_ADDR "230.0.0.10"

#ifdef NETWORK_UNDER_WORK

void RunWithNetwork()
{
#ifdef WINDOWS
	WSADATA 			wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		
	}
#endif
	int				sockin,sockout,
	len;
	char			buffer[4096];

	struct	addrinfo			hints;

	if ((sockin = socket(AF_INET, SOCK_DGRAM,	0)) < 0)
	{
	
	}
	
	struct sockaddr_in	local_sin;
	local_sin.sin_family = AF_INET;
	local_sin.sin_port = htons(DEST_PORT);
	local_sin.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockin,
		(struct sockaddr FAR *) &local_sin,
		sizeof(local_sin)) == SOCKET_ERROR)
	{
		printf("Binding socket failed! Error: %d\n", SOCKET_ERRNO);
		
		return FALSE;
	}
	struct ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = inet_addr(RECV_IP_ADDR);
	mreq.imr_interface.s_addr = INADDR_ANY;
	if ((sockout = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{

	}
	multicast_setup_listen(sockin, config->group, config->source,
		config->iface) < 0)
	unsigned char BufferTS[BUFFER_SIZE * 100];
	while (1)
	{
		int NbRead = fread(BufferTS, 1, BUFFER_SIZE, input);
		if (NbRead < 0) break;
		if (NbRead > 0)
		{
			if (BufferTS[0] != 0x47) fprintf(stderr, "TS Sync Error\n");
			for (int i = 0; i < NbRead; i += 188)
			{
				int len = Dvbs2AddTsPacket(BufferTS + i);

				if (len > 0)
				{
					sfcmplx *Frame = Dvbs2_get_IQ();
					for (int j = 0; j < len; j++)
					{
						short Re, Im;
						Re = (short)round((Frame[i].re * 32767));
						Im = (short)round((Frame[i].im * 32767));
						fwrite(&Re, 1, sizeof(short), output);
						fwrite(&Im, 1, sizeof(short), output);
					}
				}
			}
		}
	}

}
#endif

int main(int argc, char **argv)
{
	if (argc > 1) {
		if (!strcmp(argv[1], "-")) {
			input = stdin;
		}
		else {
			input = fopen(argv[1], "r");
			if (NULL == input) {
				fprintf(stderr, "Unable to open '%s': %s\n",
					argv[1], strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
	}
	else {
		input = stdin;
	}

	if (argc > 2) {
		if (!strcmp(argv[2], "-")) {
			output = stdout;
		}
		else {
			output = fopen(argv[2], "wb");
			if (NULL == input) {
				fprintf(stderr, "Unable to open '%s': %s\n",
					argv[1], strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
	}
	else {
		output = stdout;
	}

	//int Bitrate=Dvbs2Init(1000000, CR_5_6, M_32APSK, 1, RO_0_35);
	//fprintf(stderr,"TS bitrate should be %d\n",Bitrate);

	 int Bitrate = DvbsInit(1000000, CR_7_8);
	fprintf(stderr,"TS bitrate should be %d\n",Bitrate);

	RunWithFile();
    return 0;
}

