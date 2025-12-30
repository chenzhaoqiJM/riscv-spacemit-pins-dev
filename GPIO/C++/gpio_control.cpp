#include <stdio.h>
#include <unistd.h>
#include <lgpio.h>

#define GPIO_PIN 71
#define GPIO_CHIP 0

int main(int argc, char *argv[])
{
    int h;
    int lFlags = 0; // default flags

    // Open GPIO chip 0
    h = lgGpiochipOpen(GPIO_CHIP);
    if (h < 0)
    {
        printf("Error opening GPIO chip %d\n", GPIO_CHIP);
        return 1;
    }

    // Claim GPIO pin 71 as output
    if (lgGpioClaimOutput(h, lFlags, GPIO_PIN, 0) < 0)
    {
        printf("Error claiming GPIO pin %d as output\n", GPIO_PIN);
        lgGpiochipClose(h);
        return 1;
    }

    printf("Blinking GPIO %d on chip %d\n", GPIO_PIN, GPIO_CHIP);
    
    // Toggle the pin for a few cycles
    for (int i = 0; i < 10; i++)
    {
        lgGpioWrite(h, GPIO_PIN, 1);
        printf("ON\n");
        sleep(1);
        lgGpioWrite(h, GPIO_PIN, 0);
        printf("OFF\n");
        sleep(1);
    }

    // Release the chip
    lgGpiochipClose(h);
    return 0;
}
