#include <msp430g2553.h>
#include <controller.h>
#include <adc.h>
bool enadc=true;
bool update=false;
extern float volt0[10]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
extern float ampere0[10]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
extern float volt1[10]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
extern float ampere1[10]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
extern float sum_volt0=0,sum_ampere0=0,sum_volt1=0,sum_ampere1=0;
//P1.5 A5
//key 5
void init_adc(){
    ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE;// + MSC;
    ADC10CTL0|= SREF_0 ;                        // ADC10ON, interrupt enabled,16*ADC41CLKs
    ADC10CTL1 = INCH_3 + CONSEQ_1;                   // input A1
    ADC10AE0 |= 0x0F;                         // PA.1 ADC option select
    P1DIR    &= ~0x0F;
}

void update_adc_inter(){
    ADC10CTL0 |= ENC + ADC10SC;
}

bool lock_source=false;
void update_adc(){
    if(!lock_source){
        if(key_code==TOOGLE_V_SOURCE){
            display_state=V_SOURCE;
            lock_source=true;
        } else if(key_code==TOOGLE_I_SOURCE){
            display_state=I_SOURCE;
            lock_source=true;
        }
    } else if(key_code==0){
        lock_source=false;
    }
}

unsigned short ch_idx=3;
int spidx=0;
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void){
    float temp=0;
    switch(ch_idx){
    case 3://恒流电流
        temp=((((int)ADC10MEM)*3.55/1023)/3.3);
        sum_ampere1+=temp;
        sum_ampere1-=ampere1[spidx];
        ampere1[spidx]=temp;
        ch_idx=2;
        break;
    case 2://恒流电压
        temp=((int)ADC10MEM)*3.55/1023*2;
        sum_volt1+=temp;
        sum_volt1-=volt1[spidx];
        volt1[spidx]=temp;
        ch_idx=1;
        break;
    case 1://稳压电流
        temp=((((int)ADC10MEM)*3.55/1023)-4.925e-3)/10.216;
        sum_ampere0+=temp;
        sum_ampere0-=ampere0[spidx];
        ampere0[spidx]=temp;
        ch_idx=0;
        break;
    case 0://稳压电压
        temp=((int)ADC10MEM)*3.55/1023*2;
        sum_volt0+=temp;
        sum_volt0-=volt0[spidx];
        volt0[spidx]=temp;
        ch_idx=3;
        if(spidx>=9)
            spidx=0;
        else
            ++spidx;
    }
    if(ch_idx!=3){
        ADC10CTL0&=~ADC10SC;
        ADC10CTL0|=ADC10SC;
    }
}
