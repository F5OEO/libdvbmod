#include "memory.h"
#include "../dvb.h"
extern void dvb_rs_encode(uint8_t *inout);
extern void dvb_convolutional_interleave(uint8_t *inout);
extern int dvb_conv_encode_frame(uint8_t *in, uint8_t *out, int len);

#define MP_T_SYNC 0x47
#define DVBS_T_ISYNC 0xB8
#define DVBS_T_PAYLOAD_LEN 187
#define MP_T_FRAME_LEN 188
#define DVBS_RS_BLOCK_DATA 239
#define DVBS_RS_BLOCK_PARITY 16
#define DVBS_RS_BLOCK (DVBS_RS_BLOCK_DATA+DVBS_RS_BLOCK_PARITY)
#define DVBS_PARITY_LEN 16
#define DVBS_T_CODED_FRAME_LEN (MP_T_FRAME_LEN+DVBS_PARITY_LEN)
#define DVBS_T_FRAMES_IN_BLOCK 8
#define DVBS_T_BIT_WIDTH 8
#define DVBS_T_SCRAM_SEQ_LENGTH 1503

unsigned int s_reg;// Register used for scrambler
unsigned char dvbs_s_table[DVBS_T_SCRAM_SEQ_LENGTH];//Scrambler table
static int m_fc;
static int m_sc;

//
// Called once
//

int dvb_scramble_bit( void )
{
	int out = s_reg&0x01;
	//Shift
	s_reg >>= 1;

	out ^= (s_reg&1);
 
	if( out ) s_reg |= 0x4000;

	return out;
}
void dvb_scrambler_init( void )
{
	int i,j;
	s_reg = 0x4A80;// prime value
	uint8_t *p = &dvbs_s_table[0];

	for( i = 0; i < DVBS_T_SCRAM_SEQ_LENGTH; i++ )
	{
		p[i]=0;
		for( j = 0; j < 8; j++ )
		{
			p[i]<<=1;
			p[i] |= dvb_scramble_bit();
		}
	}
    //print_scramble_table();
}
//
// Called many times
// In place operation,starts at byte after the sync byte
//
void dvb_scramble_transport_packet( uint8_t *in, uint8_t *out )
{
	int i;

    for( i = 1; i <= DVBS_T_PAYLOAD_LEN; i++ )
	{
        out[i] = in[i]^dvbs_s_table[m_sc++];
	}
	// The scramber carries on but the SYNC byte is not 
	// scrambled so we need to increment the pointer by one
    m_sc++;
}
void dvb_encode_init( void )
{
	dvb_scrambler_init();
	m_fc = 0;
	m_sc = 0;
}
void dvb_reset_scrambler(void)
{
    m_sc  = 0;
}

//
// Input the transport packets
// Output the baeband modulated samples
//
//	tp = transport packet
//
int dvb_encode_frame( uint8_t *tp, uint8_t *dibit  )
{
	int len;
    uint8_t pkt[DVBS_RS_BLOCK];

    if( m_fc == 0 )
	{
		// Add the inverted sync byte and do the first frame
		// Transport multiplex adaption
        pkt[0] = DVBS_T_ISYNC;
		// Reset the scrambler
        dvb_reset_scrambler();
	}
    else
    {
         pkt[0] = MP_T_SYNC;
    }
    m_fc = (m_fc+1)%DVBS_T_FRAMES_IN_BLOCK;

	// Apply the scrambler
    dvb_scramble_transport_packet( tp, pkt );

	// Reed Solomon Encode the whole frame
    dvb_rs_encode( pkt );

	// Apply the Interleaver
    dvb_convolutional_interleave( pkt );

	// Apply the convolutional encoder and produce the dibit array
    len = dvb_conv_encode_frame( pkt, dibit, DVBS_T_CODED_FRAME_LEN );
	return len;
}

