#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"


#define PWM_FREQUENCY 55

int main(void)
{
    volatile uint32_t ui32Load;
    volatile uint32_t ui32PWMClock;
    volatile uint8_t ui8Adjust;
    ui8Adjust = 83;

    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1); // enable LED
    GPIOPinConfigure(GPIO_PF1_M1PWM5); // set LED as PWM output

    ui32PWMClock = SysCtlClockGet() / 64;
    ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
    PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN);
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ui32Load);

    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, 50 * ui32Load / 1000);  // start at leftmost pos
    PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true);
    PWMGenEnable(PWM1_BASE, PWM_GEN_2);

    volatile uint32_t direction = 1;  // used to direct which way the servo is sweeping

    while(1)
    {


        if (direction == 1)  // go from dim to bright
        {
            ui8Adjust++;
            if (ui8Adjust >= 250)
                direction = 0;  // change direction when end is reached

            PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8Adjust * ui32Load / 1000);
        }
        else if (direction == 0)  // go from bright to dim
        {
            ui8Adjust--;
            if (ui8Adjust <= 10)
                direction = 1;   // change direction.

            PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8Adjust * ui32Load / 1000);
        }
        SysCtlDelay(100000);
    }

}
