#ifndef ROT_ENCODER_H
#define ROT_ENCODER_H

/*resolution of encoder turn*/
#define RESOLUTION (5/ (4096-1) )

/* Values of displayed vars*/
extern float Vset;
extern float Cset;
extern float Vmon;
extern float Cmon;

void Vencoder_poll(void);
void Cencoder_poll(void);
void vals_reset(void);

#endif