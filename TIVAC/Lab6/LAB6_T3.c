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
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // for LEDs

    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinConfigure(GPIO_PF1_M1PWM5);    // for PF1
    GPIOPinConfigure(GPIO_PF2_M1PWM6);    // for PF2
    GPIOPinConfigure(GPIO_PF3_M1PWM7);    // for PF3

    ui32PWMClock = SysCtlClockGet() / 64;
    ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;

    // configure for PWM5 and 6
    PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN);
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ui32Load);
    // configure for PWM7
    PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, ui32Load);

    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, 50 * ui32Load / 1000);  // set all 3 PWMs
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, 50 * ui32Load / 1000);  // at starting pos
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, 50 * ui32Load / 1000);
    PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT | PWM_OUT_6_BIT | PWM_OUT_7_BIT, true);

    PWMGenEnable(PWM1_BASE, PWM_GEN_2);  // turn on both PWMs
    PWMGenEnable(PWM1_BASE, PWM_GEN_3);

    uint32_t i, j, k;

    while(1)
    {
        for (i = 1500; i >= 10; i--) {
            PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, i * ui32Load / 1000);
            SysCtlDelay(50000);
            for (j = 1500; j >= 10; j--) {
                PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, j * ui32Load / 1000);
                SysCtlDelay(50000);
                for (k = 1500; k >= 10; k--) {
                    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, k * ui32Load / 1000);
                    SysCtlDelay(50000);
                }

            }
        }
    }

}
