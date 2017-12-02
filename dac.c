#include <dac.h>
#include <msp430g2553.h>
#include <controller.h>

#define SCL_L       P1OUT&=~BIT5
#define SCL_H       P1OUT|=BIT5
#define SDA_L       P1OUT&=~BIT4
#define SDA_H       P1OUT|=BIT4
#define SDA_IN      P1OUT|=BIT4; P1DIR&=~BIT4; P1REN|=BIT4
#define SDA_OUT     P1DIR|=BIT4; P1REN&=~BIT4
#define DAC6571_voltage_max      493  //493x10mV
#define DAC6571_address         0x98  // 1001 10 A0 0  A0=0

extern unsigned int dac6571_code=0x0000;
extern int ampere=000;//10mA
void dac6571_byte_transmission(unsigned char byte_data){
    unsigned char i,shelter;
    shelter = 0x80;

    for (i=1; i<=8; i++){
        if ((byte_data & shelter)==0) SDA_L;
        else SDA_H;
        SCL_H; SCL_L;
        shelter >>= 1;
    }
    SDA_IN;
    SCL_H; SCL_L;
    SDA_OUT;
}

void dac6571_fastmode_operation(void){
    unsigned char msbyte,lsbyte;

    SCL_H;SDA_H;SDA_L;SCL_L;
    dac6571_byte_transmission(DAC6571_address);
    msbyte = dac6571_code/256;
    lsbyte = dac6571_code - msbyte * 256;
    dac6571_byte_transmission(msbyte);
    dac6571_byte_transmission(lsbyte);

    SDA_L;SCL_H;SDA_H;        // STOP condition
}

void init_dac(void){
    P1DIR |= BIT4 + BIT5;
    P1OUT |= BIT4 + BIT5;
}

bool lock_level=false;
bool lock_manual=false;
bool lock_display=false;
bool manual=true;

void update_dac(void){
    if(!lock_manual && key_code==TOOGLE_MANUAL){
        manual=!manual;
        lock_manual=true;
    } else if(lock_manual && key_code==0){
        lock_manual=false;
    }
    led[7]= manual? 1:2;

    if(!lock_display && key_code==TOOGLE_DAC){
        display_state=DAC;
        lock_display=true;
    } else if(lock_display && key_code==0){
        lock_display=false;
    }

    if(!lock_level){
        if(key_code==ADD_100mA){
            ampere+=10;
            if(ampere>=100)
                ampere=100;
            lock_level=true;
        } else if(key_code==ADD_10mA){
            ampere+=1;
            if(ampere>=100)
                ampere=100;
            lock_level=true;
        } else if(key_code==SUB_100mA){
            ampere-=10;
            if(ampere<=0)
                ampere=0;
            lock_level=true;
        } else if(key_code==SUB_10mA){
            ampere-=1;
            if(ampere<=0)
                ampere=0;
            lock_level=true;
        }
        if(lock_level){
            //_DINT();
            float temp=(6.992*ampere-9.37);
            if(temp<0)
                temp=0;
            else if(temp>1023)
                temp=1023;
            if(temp-(int)temp >0.5)
                temp+=1;
            dac6571_code=((unsigned int)temp)*4;
            dac6571_fastmode_operation();
            //_EINT();
        }
    }else if(lock_level && key_code==0){
        lock_level=false;
    }
}
