#ifndef ADC_H_
#define ADC_H_

void init_adc();
void update_adc_inter();
void update_adc();

extern float volt0[];
extern float ampere0[];
extern float volt1[];
extern float ampere1[];
extern float sum_volt0,sum_ampere0,sum_volt1,sum_ampere1;

#define TOOGLE_V_SOURCE 3
#define TOOGLE_I_SOURCE 4
#endif /* ADC_H_ */
