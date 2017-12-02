#ifndef DAC_H_
#define DAC_H_

extern unsigned int dac6571_code;
extern int ampere;
void init_dac(void);
void update_dac(void);
#define TOOGLE_MANUAL 8
#define TOOGLE_DAC 7
#define SHOW_V_SOURCE 3
#define SHOW_I_SOURCE 4
#define ADD_100mA 1
#define ADD_10mA 2
#define SUB_100mA 5
#define SUB_10mA 6

#endif /* DAC_H_ */
