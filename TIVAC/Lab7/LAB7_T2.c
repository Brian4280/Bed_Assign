#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "inc/hw_ints.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"

void sendString(char text[])
{
    int i = 0;
    while(text[i] != '\0')
    {
        UARTCharPut(UART0_BASE, text[i]);
        i++;
    }
}

void UARTIntHandler(void)
{
    uint32_t ui32Status;
    ui32Status = UARTIntStatus(UART0_BASE, true); //get interrupt status
    UARTIntClear(UART0_BASE, ui32Status); //clear the asserted interrupts

    uint32_t Vals[4];
    uint32_t Avg;
    uint32_t TempC, TempF;

    char Cel[10];
    char Far[10];
    char command;
    command = UARTCharGet(UART0_BASE);
    if (command == 'R') {
        UARTCharPut(UART0_BASE, command);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1); }
    else if (command == 'B'){
        UARTCharPut(UART0_BASE, command);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); }
    else if (command == 'G') {
        UARTCharPut(UART0_BASE, command);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); }
    else if (command == 'r') {
        UARTCharPut(UART0_BASE, command);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0); }
    else if (command == 'b') {
        UARTCharPut(UART0_BASE, command);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); }
    else if (command == 'g') {
        UARTCharPut(UART0_BASE, command);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0); }
    else if (command == 'T') {
        UARTCharPut(UART0_BASE, command);
        ADCIntClear(ADC0_BASE, 1);
        ADCSequenceEnable(ADC0_BASE, 1);
        ADCProcessorTrigger(ADC0_BASE, 1);
        while(!ADCIntStatus(ADC0_BASE, 1, false))
        {

        }
        ADCSequenceDataGet(ADC0_BASE, 1, Vals);
        Avg = (Vals[0] + Vals[1] + Vals[2] + Vals[3] + 2) / 4;

        TempC = (1475 - ((2475 * Avg)) / 4096) / 10;
        TempF = ((TempC * 9) + 160) / 5;
        ltoa(TempF, Far);
        ltoa(TempC, Cel);

        sendString("\r\nTemp = ");
        sendString(Cel);
        sendString("C = ");
        sendString(Far);
        UARTCharPut(UART0_BASE, 'F');
    }
    else {
        UARTCharPut(UART0_BASE, command);
        sendString("\r\nR: red, G: green, B: blue, T: temperature\r\n");
    }

    sendString("\r\nEnter command: ");

}

void task1Setup(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); //enable GPIO port for LED
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2); //enable pin for LED PF2
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);

    IntMasterEnable(); //enable processor interrupts
    IntEnable(INT_UART0); //enable the UART interrupt
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT); //only enable RX and TX interrupts
}




int main(void) {
    task1Setup();

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0);
    sendString("Enter Command: ");


    while (1) //let interrupt handler do the UART echo function
    {
        // i
    }
}


