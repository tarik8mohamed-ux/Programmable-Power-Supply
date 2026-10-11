#ifndef ROT_ENCODER_H
#define ROT_ENCODER_H

/* setpoint change per encoder click (matches the display's 0.01 resolution) */
#define RESOLUTION 0.01f

/* setpoint limits */
#define VSET_MAX 24.0f   /* DAC full scale x CV gain = 5 V x 4.8; about 22 V is reachable from a 24 V input */
#define CSET_MAX 3.0f    /* two LT3081s at 1.5 A each */

/* Values of displayed vars (defined in ROT_ENCODER_MOD.c) */
extern float Vset;
extern float Cset;
extern float Vmon;
extern float Cmon;

void encoder_init(void);
void Vencoder_poll(void);
void Cencoder_poll(void);
void vals_reset(void);

#endif