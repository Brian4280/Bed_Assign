
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
#include "driverlib/timer.h"
#include "driverlib/debug.h"
#define NUM_SSI_DATA     2

void enableADC()
{
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ); //40mhz clock
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);   // enable ADC0
    ADCHardwareOversampleConfigure(ADC0_BASE, 32);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // enable portF for LEDs
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);  // sequence 3 for 8 values
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 3, ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);
    // on last step, signal end and enable interrupt
}

int main(void)
{
    enableADC();  // turn on clock and ADC
    // same as every other ADC related assignment

    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);  // enable SSI0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); // enable Ports A

    GPIOPinConfigure(GPIO_PA2_SSI0CLK);  // configure each element of SSI
    GPIOPinConfigure(GPIO_PA3_SSI0FSS);
    GPIOPinConfigure(GPIO_PA4_SSI0RX);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);
    GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_3 | GPIO_PIN_2);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4, GPIO_PIN_4);

    // configure an enable the SSI port for SPI master mode
    SSIClockSourceSet(SSI0_BASE, SSI_CLOCK_SYSTEM);
    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 1000000, 8);

    SSIEnable(SSI0_BASE); // enable SSI

    //enable UART
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0, 115200, 16000000);

    UARTprintf("SSI ->\n");
    UARTprintf("  Mode: SPI\n");
    UARTprintf("  Data: 8-bit\n\n");
    UARTprintf("Send: \n  ");



    uint32_t ui32index;
    uint32_t pui32DataTx[NUM_SSI_DATA];
    uint32_t pui32DataRx[NUM_SSI_DATA];

    // for obtaining ADC values
    uint32_t ADCVals[4];
    uint32_t TempF, TempC, Avg;
    char Far[10];

    while(1)
    {
        while(SSIDataGetNonBlocking(SSI0_BASE, &pui32DataRx[0]))
        {
        }
        // turn on ADC
        ADCIntClear(ADC0_BASE, 1);
        ADCSequenceEnable(ADC0_BASE, 1);
        ADCProcessorTrigger(ADC0_BASE, 1);

        while(!ADCIntStatus(ADC0_BASE, 1, false))
        {
            // poll until ADC is complete
        }

        // grab ADC values and convert to F
        ADCSequenceDataGet(ADC0_BASE, 1, ADCVals);
        Avg = (ADCVals[0] + ADCVals[1] + ADCVals[2] + ADCVals[3] + 2) / 4;
        TempC = (1475 - ((2475 * Avg)) / 4096) / 10;
        TempF = ((TempC * 9) + 160) / 5;
        // convert the int to a string
        ltoa(TempF, Far);
        ADCSequenceDisable(ADC0_BASE, 0);


        UARTprintf("\nSent:\n ");

        for(ui32index = 0; ui32index < NUM_SSI_DATA; ui32index++)
        {
            // send the 2 numbers from the temperature
            //NUM_SSI_DATA was changed to 2 for this task
            UARTprintf("'%c' ", Far[ui32index]);
            SSIDataPut(SSI0_BASE, Far[ui32index]);
        }

        // Wait until SSI0 is done transferring all the data in the transmit FIFO.
        while(SSIBusy(SSI0_BASE))
        {
        }
        // Display indication that the SSI is receiving data.
        UARTprintf("\nReceived:\n ");
        for(ui32index = 0; ui32index < NUM_SSI_DATA; ui32index++)
        {
            // unchanged from task 0
            SSIDataGet(SSI0_BASE, &pui32DataRx[ui32index]);
            pui32DataRx[ui32index] &= 0x00FF;
            UARTprintf("'%c' ", pui32DataRx[ui32index]);
        }

        SysCtlDelay(13333333); // add a 1 sec delay
    }
}

