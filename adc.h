#ifndef ADC_H_
#define ADC_H_

void init_adc();
void update_adc_inter();
void update_adc();

extern float volt0[];
extern float ampere0[];
extern float sum_volt0,sum_ampere0;
#endif /* ADC_H_ */
