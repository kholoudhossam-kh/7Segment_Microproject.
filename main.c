/* Main Code.c: controlling a 7-segment display using interrupts.
-------------------------------------------------------------------------
This C-language program implements a single digit counter. controlling: 
a 7-segment display:
	which is interfaced with port:
		- B (pin0-_a, .... pin7-_dp): common cathode type
and on-board user switches SW1 & SW2:
	which will be used to control the coounter as follows:
		- SW1 increases the counter by 1
		- SW2 decreases the counter by 1
-------------------------------------------------------------------------
The counter will keep the value limited between (0 & 8)
	- if while increasing the value reaches 8 and then the user presses SW1
	counter will reset to 0.
	- if while decreasing the value reaches 0 and then the user presses SW1
	counter will reset to 8.
-------------------------------------------------------------------------
Connections of the 7-segment display to board:
	- GPIO PB5: f
	- GPIO PB0: a
	- GPIO PB1: b
	- GPIO PB4: e
	- GND     : com
	- GPIO PB2: c
	- GPIO PB3: d
	- GPIO PB7: DP
	- GPIO PB6: g
-------------------------------------------------------------------------
Submitted By:
	- Muhammad Mahmoud Ali  1701282
	- Kholoud Hossam Othman 1600514
	- Jama Abdi Hussin      17W0025

Board Used: 
	- Tiva - C series Launchbad
	- Micro-Controller: ARM Cortex-M4F */	
	
#include "TM4C123.h"
	#define GPIO_PORTB_Pin0_7 0xFF // Mask for PB0 _ PB07
#define GPIO_PORTF_PRIORITY_MASK      0xFF1FFFFF
#define GPIO_PORTF_PRIORITY_BITS_POS  21
#define GPIO_PORTF_INTERRUPT_PRIORITY 2
	/*//GPIO Port B Configuration Registers
		#define GPIO_PORTB_DATA_R *((volatile unsigned long *)0x400053FC)
		#define GPIO_PORTB_DIR_R *((volatile unsigned long *)0x40005400)
		#define GPIO_PORTB_DEN_R *((volatile unsigned long *)0x4000551C)
		
	
	//GPIO Port F Configuration Registers
		#define GPIO_PORTF_DATA_R *((volatile unsigned long *)0x400253FC)
		#define GPIO_PORTF_DIR_R *((volatile unsigned long *)0x40025400)
		#define GPIO_PORTF_DEN_R *((volatile unsigned long *)0x4002551C)
		#define GPIO_PORTF_PUR_R *((volatile unsigned long *)0x40025510)
		#define GPIO_PORTF_Pin1_4 *((volatile unsigned long *)0x11) // Mask for PF0 & PF4

	//Deafault Clock Frequency*/
		#define SYSTEM_CLOCK_FREQUENCY 16000000
		#define DELAY_VALUE SYSTEM_CLOCK_FREQUENCY/16
		#define DELAY_DEBOUNCE SYSTEM_CLOCK_FREQUENCY/1000
		
	//Clock Gating Control Register
		//#define SYSCTL_RCGC2_R *((volatile unsigned long *)0X400FE108)
		#define SYSCTL_RCGC2_GPIOABF 0x23 // Enable clock on portA, portB and portF

	//Global Variable Array
unsigned char CC_7Seg_Display_Lookup_Table[10] = {0x3F, 0x06, 0x5B, 0x4F, 0X66, 0x6D, 0X7D, 0X07, 0X7F, 0X6F};

// flag for interrupts in the same press 
volatile char flag=0;
		
	//Global Variables
		unsigned long volatile J;
		volatile short I = 0;
		
int main(){
	//Enable Clocks For GPIO Ports
		SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOABF;
		
	//Configs For Port B
		GPIO_PORTB_DEN_R |= GPIO_PORTB_Pin0_7;
		GPIO_PORTB_DIR_R |= GPIO_PORTB_Pin0_7;
	
	//Configs For Port F
	//Setting PF0, PF4 Switches as External interrupts
	//Digitalize PF0, PF4
		GPIO_PORTF_DEN_R  = GPIO_PORTF_DEN_R  | (1<<0) | (1<<4);
		GPIO_PORTF_LOCK_R = 0x4C4F434B;
		GPIO_PORTF_CR_R = 0X01;
	
	//Config PF0, PF4 as Inputs
		GPIO_PORTF_DIR_R = GPIO_PORTF_DIR_R & (~(1<<0)) & (~(1<<4));
	
	//Enable Pull UPs for PF0, PF4
		GPIO_PORTF_PUR_R = GPIO_PORTF_PUR_R | (1<<0) | (1<<4);
	
	// Set Deafault Value for 7-Segment
		GPIO_PORTB_DATA_R = 0X00;
		
	//Write Data To PF0, PF4 For toggling The State Of LED's
		GPIO_PORTF_IS_R = GPIO_PORTF_IS_R & (~(1<<0)) & (~(1<<4)); 	//Edge Triggered Interrupter//1??
		GPIO_PORTF_IBE_R = GPIO_PORTF_IBE_R & (~(1<<0)) & (~(1<<4)); //Disabling Both Edge Triggered Interrupter//1??
		GPIO_PORTF_IEV_R = GPIO_PORTF_IEV_R & (~(1<<0)) & (~(1<<4)); //Selecting Falling Edge Triggered Interrupter//1??
		GPIO_PORTF_IM_R = GPIO_PORTF_IM_R | (1<<0) | (1<<4); 				//PF0 PF4 Are Not Masked;
		GPIO_PORTF_ICR_R = GPIO_PORTF_ICR_R | (1<<0) | (1<<4);        //Clearing Interruptors Flag For PF0 PF5
	
	//Set NVIC Register for enabling the EXIT For GPIO_PORTF
		//NVIC_ISER[0] = NVIC_ISER[0] | (1<<30); // IRQ No: GPIO_PORTF = 30 //??
                 NVIC_EN0_R         |= 0x40000000;   /* Enable NVIC Interrupt for GPIO PORTF by set bit number 30 in EN0 Register */
	
	//Set the Proproty For The GPIO_PORTF Interupt
		//NVIC_IP[30] = 0x60; //Priority 3
                NVIC_PRI7_R = (NVIC_PRI7_R & GPIO_PORTF_PRIORITY_MASK) | (GPIO_PORTF_INTERRUPT_PRIORITY<<GPIO_PORTF_PRIORITY_BITS_POS); 
	
	while(1) {	
		//Update 7-Segment
			//GPIO_PORTB_DATA_R = CC_7Seg_Display_Lookup_Table[I];
		
		//Delay
		//	for (J=0; J< DELAY_VALUE; J++);
			
		// Reset Counter
			GPIO_PORTB_DATA_R = CC_7Seg_Display_Lookup_Table[I];
			
                        if(((GPIO_PORTF_DATA_R &(1<<4)) ==(1<<4)) && flag==1)
                          flag=0;
                         if(((GPIO_PORTF_DATA_R &(1<<0) )==(1<<0)) && flag==2)
                          flag=0;
		}
}

void GPIO_PORTF_Handler()
{ 
	if((GPIO_PORTF_MIS_R & 0x10) == 0x10) //Interrupt at PF4
		{
		if(flag==0)	
                  I++;
			GPIO_PORTF_ICR_R = GPIO_PORTF_ICR_R | (1<<4);
                        flag=1;
                        if(I>8) {
			 I = 0;
			}
                      
		}
	
	if((GPIO_PORTF_MIS_R & (1<<0)) == (1<<0)) //Interrupt at PF0
	{
	if(flag==0)	
          I--;
		GPIO_PORTF_ICR_R = GPIO_PORTF_ICR_R | (1<<0);
                flag=2;
                if(I<0) {
			 I = 8;
		
                        }
	}
        //Update 7-Segment
			GPIO_PORTB_DATA_R = CC_7Seg_Display_Lookup_Table[I];
                        ;
}