//本程序时钟采用内部RC振荡器。     DCO：8MHz,供CPU时钟;  SMCLK：1MHz,供定时器时钟
#include <msp430g2553.h>
#include <tm1638.h>
#include <controller.h>
#include <adc.h>
#include <dac.h>
/////////////////////////////
//         常量定义         //
//////////////////////////////

// 0.1s软件定时器溢出值，5个20ms
#define V_T100ms	5

//////////////////////////////
//       变量定义           //
//////////////////////////////

// 软件定时器计数
unsigned char clock100ms=0;
// 软件定时器溢出标志
unsigned char clock100ms_flag=0;
// 测试用计数器
unsigned int test_counter=0;
// 8位数码管显示的数字或字母符号
// 注：板上数码位从左到右序号排列为4、5、6、7、0、1、2、3
extern unsigned char digit[8]={' ',' ',' ',' ',' ',' ',' ',' '};
// 8位小数点 1亮  0灭
// 注：板上数码位小数点从左到右序号排列为4、5、6、7、0、1、2、3
unsigned char pnt=0x11;
extern unsigned char led[]={0,0,0,0,0,0,0,0};
// 当前按键值
extern unsigned char key_code=0;
bool upgraded=false;

extern enum DISPLAY display_state=V_SOURCE;
//enum PIN {P1,P2};
//////////////////////////////
//       系统初始化         //
//////////////////////////////
void set_input(enum PIN pin,unsigned char port){
    unsigned char pinOUT,pinDIR,pinREN;
    if(pin==P1){
        pinOUT=P1OUT;
        pinDIR=P1DIR;
        pinREN=P1REN;
    } else{
        pinOUT=P2OUT;
        pinDIR=P2DIR;
        pinREN=P2REN;
    }
    pinDIR &= !port;
    pinOUT |= port;
    pinREN |= port;
}

void set_output(enum PIN pin,unsigned char port){
    if(pin==P1){
        P1DIR |= port;
    } else{
        P2DIR |= port;
    }
}
//  I/O端口和引脚初始化
void Init_Ports(void)
{
	P2SEL &= ~(BIT7+BIT6);       //P2.6、P2.7 设置为通用I/O端口
	//因两者默认连接外晶振，故需此修改

	P2DIR |= BIT7 + BIT6 + BIT5; //P2.5、P2.6、P2.7 设置为输出
	//set_output(P1,0xF);

}

//  定时器TIMER0初始化，循环定时20ms
void Init_Timer0(void)
{
	TA0CTL = TASSEL_2 + MC_1 ;      // Source: SMCLK=1MHz, UP mode,
	TA0CCR0 = 20000;                // 1MHz时钟,计满20000次为 20ms
	TA0CCTL0 = CCIE;                // TA0CCR0 interrupt enabled
}

//  MCU器件初始化，注：会调用上述函数
void Init_Devices(void)
{
	WDTCTL = WDTPW + WDTHOLD;     // Stop watchdog timer，停用看门狗
	if (CALBC1_8MHZ ==0xFF || CALDCO_8MHZ == 0xFF)
	{
		while(1);            // If calibration constants erased, trap CPU!!
	}

    //设置时钟，内部RC振荡器。     DCO：8MHz,供CPU时钟;  SMCLK：1MHz,供定时器时钟
	BCSCTL1 = CALBC1_8MHZ; 	 // Set range
	DCOCTL = CALDCO_8MHZ;    // Set DCO step + modulation
	BCSCTL3 |= LFXT1S_2;     // LFXT1 = VLO
	IFG1 &= ~OFIFG;          // Clear OSCFault flag
	BCSCTL2 |= DIVS_3;       //  SMCLK = DCO/8

    Init_Ports();           //调用函数，初始化I/O口
    Init_Timer0();          //调用函数，初始化定时器0
    _BIS_SR(GIE);           //开全局中断
   //all peripherals are now initialized
}

//////////////////////////////
//      中断服务程序        //
//////////////////////////////

bool marker=false;
// Timer0_A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void)
{
    _BIS_SR(GIE);
	// 0.1秒钟软定时器计数
	if (++clock100ms>=V_T100ms)
	{
		clock100ms_flag = 1; //当0.1秒到时，溢出标志置1
		clock100ms = 0;
	}
	// 刷新全部数码管和LED指示灯
	TM1638_RefreshDIGIandLED(digit,pnt,led);

	// 检查当前键盘输入，0代表无键操作，1-16表示有对应按键
	key_code=TM1638_Readkeyboard();

	//调用扩展模块
	update_adc_inter();
//	update_level();
}



void update_display(void){
    int volt_str;
    int ampere_str;
    switch(display_state){
    case V_SOURCE:
        volt_str=(int)(100*sum_volt0);
        ampere_str=(int)(100*sum_ampere0);

        digit[3] = volt_str%10;
        digit[2] = (volt_str/10)%10;
        digit[1] = (volt_str/100)%10;
        digit[0] = (volt_str/1000)%10;
        digit[7] = ampere_str%10;
        digit[6] = (ampere_str/10)%10;
        digit[5] = (ampere_str/100)%10;
        digit[4] = (ampere_str/1000)%10;
        break;
    case I_SOURCE:
        volt_str=(int)(100*sum_volt1);
        ampere_str=(int)(100*sum_ampere1);

        digit[3] = volt_str%10;
        digit[2] = (volt_str/10)%10;
        digit[1] = (volt_str/100)%10;
        digit[0] = (volt_str/1000)%10;
        digit[7] = ampere_str%10;
        digit[6] = (ampere_str/10)%10;
        digit[5] = (ampere_str/100)%10;
        digit[4] = (ampere_str/1000)%10;
        break;
    case DAC:
        digit[2] = ampere%10;
        digit[1] = (ampere/10)%10;
        digit[0] = (ampere/100)%10;
        digit[3] = ' ';
    }
}

//////////////////////////////
//         主程序           //
//////////////////////////////
int main(void)
{
   	Init_Devices();
	init_adc();
	init_dac();
	while (clock100ms<3);   // 延时60ms等待TM1638上电完成
	init_TM1638();	    //初始化TM1638

	while(1)
	{
		if (clock100ms_flag==1)   // 检查0.1秒定时是否到
		{
			clock100ms_flag=0;
            //更新增益等级
		}
		//调用扩展模块
		update_adc();
		update_dac();
		update_display();
	}
}
