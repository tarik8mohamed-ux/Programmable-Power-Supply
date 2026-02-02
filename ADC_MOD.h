#ifndef ADC_MOD_H
#define ADC_MOD_H

enum ADC_bm{
	VMON_bm = 0x10,
	CMON_bm = 0x11
	};

void ADC_init(uint8_t mode);
void ADC_channelSEL(uint8_t AIN_POS);
float ADC_read(void);



#endif