#include "libdvbmod.h"
#include "DVB-S2\DVBS2.h"

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


