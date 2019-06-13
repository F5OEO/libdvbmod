#include "libdvbmod.h"
#include "./DVB-S2/DVBS2.h"
#include "./DVB-S/dvbs.h"
#include "math.h"
#include <stdlib.h> 


#define UPSAMPLE_MAX 4
size_t m_upsample=1;
float CoefFec[12] = { 1 / 4.0,1 / 3.0,2 / 5.0,1 / 2.0,3 / 5.0,2 / 3.0,3 / 4.0,4 / 5.0,5 / 6.0,8 / 9.0,7 / 8.0,9 / 10.0 };
DVBS2 DvbS2Modulator;

size_t DVBS2Length=0;
static sfcmplx *dvbs_symbols_short;


int Dvbs2Init(int SRate,int CodeRate,int Constellation,int PilotesOn,int RollOff,int Upsample,bool ShortFrame)
{
	DVB2FrameFormat S2Format;
	if(ShortFrame)
	{
		S2Format.frame_type = FRAME_SHORT;
		fprintf(stderr,"DVB ShortFrame\n");
	}	
	else
	{
		S2Format.frame_type = FRAME_NORMAL;
		fprintf(stderr,"DVB LongFrame\n");
	}	
	
	
	S2Format.code_rate = CodeRate;
	S2Format.constellation = Constellation;
	S2Format.pilots = PilotesOn;
	S2Format.broadcasting = 1;
	S2Format.roll_off = RollOff;
	S2Format.null_deletion = 0;
	S2Format.dummy_frame = 0;
	DvbS2Modulator.s2_set_configure(&S2Format);
	m_upsample=Upsample;

	int FrameSize=(S2Format.frame_type==FRAME_NORMAL)?FRAME_SIZE_NORMAL:FRAME_SIZE_SHORT;
	fprintf(stderr,"Frame Size=%d\n",FrameSize);
	dvbs_symbols_short=(sfcmplx*)malloc(FrameSize*m_upsample*sizeof(sfcmplx));
	return (int)(SRate*DvbS2Modulator.s2_get_efficiency());
}

int Dvbs2AddTsPacket(uint8_t *Packet)
{
	DVBS2Length=DvbS2Modulator.s2_add_ts_frame(Packet);
	return DVBS2Length*m_upsample;
	
}

sfcmplx *Dvbs2_get_IQ(void)
{
	if(m_upsample>1)
	{
		static sfcmplx Zero;
		Zero.im=0;
		Zero.re=0;
		sfcmplx *FrameS2IQ=(sfcmplx*)DvbS2Modulator.pl_get_frame();

		for(size_t i=0;i<DVBS2Length;i++)
		{
			for(size_t j=0;j<m_upsample;j++)
			{
				dvbs_symbols_short[i*m_upsample+j]=(j==0)?FrameS2IQ[i]:Zero;
				//dvbs_symbols_short[i*m_upsample+j].im*=m_upsample;
				//dvbs_symbols_short[i*m_upsample+j].re*=m_upsample;
			}
		}
		return dvbs_symbols_short;
	}
	else
	{	
		
		return (sfcmplx*)DvbS2Modulator.pl_get_frame();

	}
	
}

//******************* DVB-S **********************************
#define SMAG 0x3FFF

sfcmplx m_qpsk[4];
sfcmplx m_8psk[8];

void InitConstellation(void)
{
	#define CP 0x7FFF
	double r0, r1, r2, r3;
	double m = 1.0;
	r0 = 0.9;// I am not sure why this needs to be 0.9 but 32APSK does not work if == 1.0
	r1 = m;

	// QPSK
	m_qpsk[0].re = (short)((r1*cos(M_PI / 4.0))*CP);
	m_qpsk[0].im = (short)((r1*sin(M_PI / 4.0))*CP);
	m_qpsk[1].re = (short)((r1*cos(7 * M_PI / 4.0))*CP);
	m_qpsk[1].im = (short)((r1*sin(7 * M_PI / 4.0))*CP);
	m_qpsk[2].re = (short)((r1*cos(3 * M_PI / 4.0))*CP);
	m_qpsk[2].im = (short)((r1*sin(3 * M_PI / 4.0))*CP);
	m_qpsk[3].re = (short)((r1*cos(5 * M_PI / 4.0))*CP);
	m_qpsk[3].im = (short)((r1*sin(5 * M_PI / 4.0))*CP);

	//8PSK
	m_8psk[0].re = (short)((r1*cos(M_PI / 4.0))*CP);
	m_8psk[0].im = (short)((r1*sin(M_PI / 4.0))*CP);
	m_8psk[1].re = (short)((r1*cos(0.0))*CP);
	m_8psk[1].im = (short)((r1*sin(0.0))*CP);
	m_8psk[2].re = (short)((r1*cos(4 * M_PI / 4.0))*CP);
	m_8psk[2].im = (short)((r1*sin(4 * M_PI / 4.0))*CP);
	m_8psk[3].re = (short)((r1*cos(5 * M_PI / 4.0))*CP);
	m_8psk[3].im = (short)((r1*sin(5 * M_PI / 4.0))*CP);
	m_8psk[4].re = (short)((r1*cos(2 * M_PI / 4.0))*CP);
	m_8psk[4].im = (short)((r1*sin(2 * M_PI / 4.0))*CP);
	m_8psk[5].re = (short)((r1*cos(7 * M_PI / 4.0))*CP);
	m_8psk[5].im = (short)((r1*sin(7 * M_PI / 4.0))*CP);
	m_8psk[6].re = (short)((r1*cos(3 * M_PI / 4.0))*CP);
	m_8psk[6].im = (short)((r1*sin(3 * M_PI / 4.0))*CP);
	m_8psk[7].re = (short)((r1*cos(6 * M_PI / 4.0))*CP);
	m_8psk[7].im = (short)((r1*sin(6 * M_PI / 4.0))*CP);
}

static uint8_t dvbs_dibit[DVBS_RS_BLOCK*16];
static sfcmplx dvbs_symbol[DVBS_RS_BLOCK*16*4];
static short *dvbs_symbols_map;



static uint8_t InterMedBuffer[DVBS_RS_BLOCK * 16 * 4];


static int LenFrame = 0;
static int m_Constellation = M_QPSK;
int ConstellationEffiency[] = { 2,3,4,5 }; //Nb Bit per Symbol


int DvbsInit(int SRate, int CodeRate, int Constellation ,int Upsample)
{
	InitConstellation ();
	m_Constellation = Constellation;
	dvb_encode_init(CodeRate);
	int NetBitrate = SRate * 188 * CoefFec[CodeRate] / 204 * ConstellationEffiency[Constellation];
	m_upsample=Upsample;
	dvbs_symbols_short=(sfcmplx*)malloc(DVBS_RS_BLOCK*16*m_upsample*sizeof(sfcmplx));
	dvbs_symbols_map=(short*)malloc(DVBS_RS_BLOCK*16*m_upsample*sizeof(short));
	return NetBitrate;
}

int DvbsAddTsPacket(uint8_t *Packet)
{
	
	
	LenFrame = dvb_encode_frame(Packet, dvbs_dibit);
		
	return LenFrame*m_upsample;
}

sfcmplx *Dvbs_get_IQ(void)
{
	int psklen = 0;
	static sfcmplx Zero;
	Zero.im=0;
	Zero.re=0;
	
			for (size_t i = 0; i < (size_t)LenFrame; i++)
			{
				for(size_t j=0;j<m_upsample;j++)
				{
					dvbs_symbols_short[i*m_upsample+j] =(j==0)?m_qpsk[dvbs_dibit[i]]:Zero;
					//dvbs_symbols_short[i*m_upsample+j].im*=m_upsample;
 					//dvbs_symbols_short[i*m_upsample+j].re*=m_upsample;
					psklen++;
				}
			}
			
		LenFrame = 0;
		return dvbs_symbols_short;
}

short *Dvbs_get_MapIQ(int *Len)
{
	int psklen=0;

	
			
			#define MAX_SYMBOLS_REMAINING 12
			static short RemainingSymbolTab[MAX_SYMBOLS_REMAINING];
			static int Remaining=0;
			int shift=0;
			int index=0;
			//Fill the remaining first
			short package=0;

			for (size_t i = 0; i < Remaining; i++)
			{
					package|=RemainingSymbolTab[i]<<(shift*2+4);
					shift++;
					if(shift==6)
					{
						dvbs_symbols_map[index] =package;
						 shift=0;
						 index++;
						 package=0;
					}	 
			}

			//Fill with the current Frame

			for (size_t i = 0; i < (size_t)LenFrame; i++)
			{
				short dibit=dvbs_dibit[i];
				package|=(dibit)<<(shift*2+4);
				shift++;
				if(shift==6)
				{
					dvbs_symbols_map[index] =package;
					 shift=0;
					 index++;
					 package=0;
				}
			}

			psklen=(index/2)*2; // index should be "pair"
			//Fill Remaining
			
			Remaining=(index%2)*6+shift;
			
			for (size_t i = 0; i < Remaining; i++)
			{
				RemainingSymbolTab[i]=dvbs_dibit[LenFrame-Remaining+i];
			}		


			//DEBUG
			//short patern=0xC840 ;//110010000100
			//short patern=0xFFF0 ;//110010000100
			//short patern2=0x0000 ;//110010000100

			static short patern=0x5550 ;
			static short patern2=0x7770;
		//0xFFF0 ; //0x6660 zarb,en fetch12 semble perdre bit

			static int debug=0;

			/*for(int i=0;i<psklen/2;i++)
			{
				
				dvbs_symbols_map[i*2]=patern;
				dvbs_symbols_map[i*2+1]=patern2;
			}*/

			/*for(int i=0;i<psklen/2;i++)
			{
				
				dvbs_symbols_map[i*2]=(debug%2==0)?patern:patern2;
				dvbs_symbols_map[i*2+1]=(debug%2==0)?patern:patern2;
			}
			*/
			debug++;
			//fprintf(stderr,"LenFrame %d, psklen %d remaining %d Index=%d shift=%d\n",LenFrame,psklen,Remaining,index,shift);	
			

			
			/*for (size_t i = 0; i < (size_t)LenFrame/6; i++)
			{
				short package=0;
				for(size_t j=0;j<6;j++)
				{
					short dibit=dvbs_dibit[i*6+j];
					dibit=(dibit<<4<<j);
					package|=dibit;
					
					
				}
				dvbs_symbols_map[i] =package;
				psklen++;
			}*/
			
		
	*Len=psklen/2;
		LenFrame = 0;
		return dvbs_symbols_map;
}



// **************** DVB-S ARM **********************************
#ifdef WITH_ARM



extern void viterbi_init(int SetFEC);
extern uint16_t viterbi(uint8_t *in, uint8_t *out);
extern void 	dvbsenco_init(void);
extern uint8_t*	dvbsenco(uint8_t*);
uint8_t BuffIQ[204 * 2]; 
int NbIQOutput = 0;
sfcmplx sBuffIQ[204 * 8];


int DvbsInit(int SRate, int CodeRate)
{
	dvbsenco_init();
	viterbi_init(CodeRate);
	
	return 0;
}


int DvbsAddTsPacket(uint8_t *Packet)
{
	uint8_t *p = dvbsenco(Packet);
	NbIQOutput = viterbi(p, BuffIQ);
	return NbIQOutput;
}

extern sfcmplx *Dvbs_get_IQ(void)
{
	for (int i = 0; i < NbIQOutput;i++)
	{
		for (int j = 0; j < 4; j += 2)
		{
			sBuffIQ[i * 4 + j] = tx_lookup[((BuffIQ[i] >> j) & 0x3)];
		}
	}
}
#endif
