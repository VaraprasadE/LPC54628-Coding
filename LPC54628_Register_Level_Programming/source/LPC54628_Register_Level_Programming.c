#include "LPC54628.h"

volatile uint32_t msTicks = 0;
// The compiler sees this and "replaces" the weak one in the startup file
void SysTick_Handler(void) {
    msTicks++; // Increment your global counter
}

void delay_ms(uint32_t ms) {
    uint32_t startTicks = msTicks;
    while ((msTicks - startTicks) < ms);
}

int main(void) {
    /* STEP 1: Enable Clocks (SYSCON) */
    // Bit 13: IOCON Clock, Bit 16: GPIO2 Clock, Bit 17: GPIO3 Clock
    SYSCON->AHBCLKCTRLSET[0] = (1UL << 13) | (1UL << 16) | (1UL << 17);

    /* STEP 2: Clear Peripheral Resets (SYSCON) */
    // Clear reset for GPIO2 and GPIO3
    SYSCON->PRESETCTRLCLR[0] = (1UL << 16) | (1UL << 17);

    /* STEP 3: Pin Multiplexing (IOCON) */
    // Set FUNC to 0 (GPIO) and ensure digital mode is on
    // Mask 0x80 usually enables digital mode on these pins
    IOCON->PIO[2][2]  = (0x0 << 0) | (1 << 7);
    IOCON->PIO[3][3]  = (0x0 << 0) | (1 << 7);
    IOCON->PIO[3][14] = (0x0 << 0) | (1 << 7);

    /* STEP 4: Set Direction (GPIO) */
    GPIO->DIR[2] |= (1UL << 2);   // Green LED Output
    GPIO->DIR[3] |= (1UL << 3);   // Red LED Output
    GPIO->DIR[3] |= (1UL << 14);  // Blue LED Output

    SysTick_Config(SystemCoreClock / 1000);

    while(1) {
        // Toggle LEDs
        GPIO->NOT[2] = (1UL << 2);
        GPIO->NOT[3] = (1UL << 3) | (1UL << 14);

//        for (volatile int i = 0; i < 1000000; i++);
        delay_ms(1000);
    }
}
