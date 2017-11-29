#include <msp430g2553.h>
#include <controller.h>

typedef short bool;
#define false 0
#define true 1

bool enadc=true;
bool update=false;
extern float volt0[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
extern float ampere0[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
extern float sum_volt0=0,sum_ampere0=0;
//P1.5 A5
//key 5
void init_adc(){
    ADC10CTL0 = ADC10SHT_3 + ADC10ON + ADC10IE + MSC;
    ADC10CTL0|= SREF_0 ;                        // ADC10ON, interrupt enabled,16*ADC41CLKs
    ADC10CTL1 = INCH_1 + CONSEQ_1;                       // input A1
    ADC10AE0 |= BIT1 + BIT0;                         // PA.1 ADC option select
    P1DIR &= ~0x03;
}

void update_adc_inter(){
    ADC10CTL0 |= ENC + ADC10SC;
//    int volt_str=(int)(1000*volt0);
//    int ampere_str=(int)(1000*ampere0);
//    digit[3] = (digit[3]&0x80) + volt_str%10;
//    digit[2] = (digit[2]&0x80) + (volt_str/10)%10;
//    digit[1] = (digit[1]&0x80) + (volt_str/100)%10;
//    digit[0] = (digit[0]&0x80) + (volt_str/1000)%10;
//    digit[7] = (digit[7]&0x80) + ampere_str%10;
//    digit[6] = (digit[6]&0x80) + (ampere_str/10)%10;
//    digit[5] = (digit[5]&0x80) + (ampere_str/100)%10;
//    digit[4] = (digit[4]&0x80) + (ampere_str/1000)%10;
}

void update_adc(){
    return;
}

unsigned short ch_idx=1;
int spidx=0;
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void){
    float temp=0;
    switch(ch_idx){
    case 1://稳压电流
        temp=((((int)ADC10MEM)*3.55/1023)-4.925e-3)/10.216;
        sum_ampere0+=temp;
        sum_ampere0-=ampere0[spidx];
        ampere0[spidx]=temp;
        if(spidx>=19)
            spidx=0;
        else
            ++spidx;
        ch_idx=0;
        break;
    case 0://稳压电压
        temp=((int)ADC10MEM)*3.55/1023*2;
        sum_volt0+=temp;
        sum_volt0-=volt0[spidx];
        volt0[spidx]=temp;
        ch_idx=1;
    }
}
