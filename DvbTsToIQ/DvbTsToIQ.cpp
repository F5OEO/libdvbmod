// DvbTsToIQ.cpp : définit le point d'entrée pour l'application console.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../libdvbmod/libdvbmod.h"
FILE *input, *output;

#define BUFFER_SIZE (188*7*100) 

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
		if (!strcmp(argv[1], "-")) {
			output = stdout;
		}
		else {
			output = fopen(argv[1], "wb");
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

	unsigned char BufferTS[BUFFER_SIZE];

	Dvbs2Init(1000, CR_8_9, M_8PSK, 1, RO_0_35);
	while (1)
	{
		int NbRead = fread(BufferTS, 1, BUFFER_SIZE, input);
		if (NbRead < 0) break;
		if (NbRead > 0)
		{
			if (BufferTS[0] != 0x47) fprintf(stderr, "TS Sync Error\n");
			for (int i = 0; i < NbRead ; i+=188)
			{
				int len=Dvbs2AddTsPacket(BufferTS + i);
				
				if (len > 0)
				{
					sfcmplx *Frame = Dvbs2_get_IQ();
					for (int j = 0; j < len; j++)
					{
						short Re, Im;
						Re= (short)round((Frame[i].re * 32767));
						Im= (short)round((Frame[i].im * 32767));
						fwrite(&Re, 1, sizeof(short), output);
						fwrite(&Im, 1, sizeof(short), output);
					}
				}
			}
		}
	}
    return 0;
}

