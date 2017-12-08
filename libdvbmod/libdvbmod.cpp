#include "libdvbmod.h"
#include "./DVB-S2/DVBS2.h"
#include "./DVB-S/dvbs.h"

float CoefFec[12] = { 1 / 4.0,1 / 3.0,2 / 5.0,1 / 2.0,3 / 5.0,2 / 3.0,3 / 4.0,4 / 5.0,5 / 6.0,8 / 9.0,7 / 8.0,9 / 10.0 };
DVBS2 DvbS2Modulator;

int Dvbs2Init(int SRate,int CodeRate,int Constellation,int PilotesOn,int RollOff)
{
	DVB2FrameFormat S2Format;
	S2Format.frame_type = FRAME_NORMAL;
	S2Format.code_rate = CodeRate;
	S2Format.constellation = Constellation;
	S2Format.pilots = PilotesOn;
	S2Format.broadcasting = 1;
	S2Format.roll_off = RollOff;
	S2Format.null_deletion = 0;
	S2Format.dummy_frame = 0;
	DvbS2Modulator.s2_set_configure(&S2Format);
	
	return (int)(SRate*DvbS2Modulator.s2_get_efficiency());
}

int Dvbs2AddTsPacket(uint8_t *Packet)
{
	return(DvbS2Modulator.s2_add_ts_frame(Packet));
	
}

sfcmplx *Dvbs2_get_IQ(void)
{
	return (sfcmplx*)DvbS2Modulator.pl_get_frame();
}

//******************* DVB-S **********************************
#define SMAG 0x3FFF

const sfcmplx tx_lookup[4] = {
	{ SMAG,  SMAG },
	{ -SMAG,  SMAG },
	{ SMAG, -SMAG },
	{ -SMAG, -SMAG }
};


static uint8_t dvbs_dibit[DVBS_RS_BLOCK*16];
static sfcmplx dvbs_symbol[DVBS_RS_BLOCK*16*4];
static int LenFrame = 0;

int DvbsInit(int SRate, int CodeRate)
{
	dvb_encode_init(CodeRate);
	return SRate * 2 * 188 * CoefFec [CodeRate ]/ 204; //Fxme Add FEC Coef
}

int DvbsAddTsPacket(uint8_t *Packet)
{
	return (LenFrame=dvb_encode_frame(Packet, dvbs_dibit));
}

sfcmplx *Dvbs_get_IQ(void)
{
	for (int i = 0; i < LenFrame; i++)
	{
		dvbs_symbol[i] = tx_lookup[dvbs_dibit[i]];
	}
	return (sfcmplx*)dvbs_symbol;
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
