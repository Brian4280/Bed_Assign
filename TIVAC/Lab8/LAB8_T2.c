
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ssi.h"
#include "inc/hw_types.h"
#include "driverlib/ssi.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/adc.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "Nokia5110.h"

#define NUM_SSI_DATA     2

void enableADC()
{
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);
}



// The delay parameter is in units of the 16 MHz core clock.
void SysTick_Wait(unsigned long delay){
    NVIC_ST_RELOAD_R = delay-1;  // number of counts to wait
    NVIC_ST_CURRENT_R = 0;       // any value written to CURRENT clears
    while((NVIC_ST_CTRL_R&0x00010000)==0){ // wait for count flag
    }
}


void SysTick_Wait50ms(unsigned long delay){
    unsigned long i;
    for(i=0; i<delay; i++){
        SysTick_Wait(2000000);  // wait 50ms
    }
}

void SysTick_Init(void){
    NVIC_ST_CTRL_R = 0;               // disable SysTick during setup
    NVIC_ST_CTRL_R = 0x00000005;      // enable SysTick with core clock
}


int main(void)
{
    enableADC(); // set clock and turn on ADC

    // used for getting temperature from adc
    uint32_t Vals[4];
    uint32_t Avg, TempC, TempF;
    char Far[10];

    // starting ssi for lcd screen
    SysTick_Init();
    startSSI0();
    initialize_screen(BACKLIGHT_ON,SSI0);
    clear_screen(SSI0);
    SysTick_Wait50ms(100);
    while(1)
    {
        // clear screen and write name and 'Temp:'
        clear_screen(SSI0);
        screen_write("Brian Lopez",ALIGN_LEFT_TOP,SSI0);
        screen_write("Temp:", ALIGN_LEFT_CENTRE, SSI0);

        // enable ADC and get value
        ADCSequenceEnable(ADC0_BASE, 1);
        ADCIntClear(ADC0_BASE, 1);
        ADCProcessorTrigger(ADC0_BASE, 1);

        while(!ADCIntStatus(ADC0_BASE, 1, false))
        {
        }

        ADCSequenceDataGet(ADC0_BASE, 1, Vals);

        Avg = (Vals[0] + Vals[1] + Vals[2] + Vals[3] + 2) / 4;
        TempC = (1475 - ((2475 * Avg)) / 4096) / 10;
        TempF = ((TempC * 9) + 160) / 5;
        // convert integer to a string
        ltoa(TempF, Far);
        // Add the F for fahrenheit after int value
        Far[2] = 'F';
        Far[3] = '\0';  // need to update NULL for screen_write function
        screen_write(Far, ALIGN_CENTRE_CENTRE, SSI0);
        SysTick_Wait50ms(20);
    }
    return 0;
}

