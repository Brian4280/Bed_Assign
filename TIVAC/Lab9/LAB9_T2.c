#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846 // value of pi
#endif

#define SERIES_LENGTH 200   // amount of values to store in buffer

float gSeriesData[SERIES_LENGTH];  // will store all values of sine wave
int32_t i32DataCount = 0;

int main(void)
{
    float fRadians;
    FPULazyStackingEnable(); // enable lazy stacking which prevents stacking floating point
                             // values to lower latency time between interrupts
    FPUEnable();     // turn on floating point

    // set the clock for 50MHz
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
    fRadians = ((2 * M_PI) / 200); // get 2*PI value
    while(i32DataCount < SERIES_LENGTH)
    {
        // get the 100 sine values 1 at a time and store them into the array
        gSeriesData[i32DataCount] = sinf(fRadians *  i32DataCount) + 0.5 * cosf(fRadians * 4 * i32DataCount);
        i32DataCount++;
    }
    while(1)
    {
        // once done, go into infinite loop...
    }
}
