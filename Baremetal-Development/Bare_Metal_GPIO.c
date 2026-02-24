#include "LPC54628.h"
#include <stdbool.h>

volatile uint32_t msTicks = 0;

// CMSIS standard SysTick Handler
void SysTick_Handler(void) {
   msTicks++;
}

void UserButton_Init(void) {
    /* 1. Enable Clocks (Index [0] for both IOCON and GPIO1) */
    // Bit 13: IOCON, Bit 15: GPIO1
    SYSCON->AHBCLKCTRLSET[0] = (1UL << 13) | (1UL << 15);

    /* 2. Clear Peripheral Reset for GPIO Port 1 (Index [0]) */
    SYSCON->PRESETCTRLCLR[0] = (1UL << 15);

    /* 3. Configure IOCON for PIO1_1 */
    // PIO array is [Port][Pin]
    // FUNC = 0, MODE = 2 (Pull-up), DIGIMODE = 1 (Bit 8)
    IOCON->PIO[1][1] = (0x0 << 0) | (0x2 << 4) | (1 << 8);

    /* 4. Set GPIO Direction to Input */
    // Port 1, Pin 1. Writing 0 to DIR sets it as input.
    GPIO->DIR[1] &= ~(1UL << 1);
}

bool UserButton_IsPressed(void) {
    /* Read from PIN register for Port 1.
       SW5 is Active-Low: 0 = Pressed, 1 = Released */
    return ((GPIO->PIN[1] & (1UL << 1)) == 0);
}

void LED_Init(void) {
   /* 1. Enable Clocks for GPIO2 and GPIO3 - Index [0] */
   SYSCON->AHBCLKCTRLSET[0] = (1UL << 16) | (1UL << 17);

   /* 2. Clear Peripheral Resets for GPIO2 and GPIO3 - Index [0] */
   SYSCON->PRESETCTRLCLR[0] = (1UL << 16) | (1UL << 17);

   /* 3. Configure Pin Multiplexing (IOCON) */
   /* PIO is a 2D array [Port][Pin] */
   IOCON->PIO[2][2]  = (0x0 << 0) | (1 << 7); // Green LED
   IOCON->PIO[3][3]  = (0x0 << 0) | (1 << 8); // Red LED
   IOCON->PIO[3][14] = (0x0 << 0) | (1 << 8); // Blue LED

   /* 4. Set Direction to Output */
   GPIO->DIR[2] |= (1UL << 2);
   GPIO->DIR[3] |= (1UL << 3) | (1UL << 14);

   /* 5. Start with LEDs OFF (Assuming Active Low) */
   GPIO->SET[2] = (1UL << 2);
   GPIO->SET[3] = (1UL << 3) | (1UL << 14);
}

void UART9_Init(uint32_t baudrate) {
   /* STEP 1: Enable Peripheral Clocks */
   // IOCON clock is in index [0]
   SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL_IOCON_MASK;
   // Flexcomm 9 clock is in AHBCLKCTRL2 (Index [2])
   SYSCON->AHBCLKCTRLSET[2] = SYSCON_AHBCLKCTRL_FLEXCOMM9_MASK;

   /* STEP 2: Select Functional Clock Source (Index [9] for FC9) */
   SYSCON->FCLKSEL[9] = 0; // 0 = FRO 12MHz

   /* STEP 3: Release Peripheral from Reset (Index [2] for FC9) */
   SYSCON->PRESETCTRLCLR[2] = SYSCON_PRESETCTRL_FC9_RST_MASK;

   /* STEP 4: Configure Pin Multiplexing (IOCON) */
   // PIO3_21 = RX, PIO3_22 = TX
   IOCON->PIO[3][21] = (1 << 0) | (1 << 8);
   IOCON->PIO[3][22] = (1 << 0) | (1 << 8);

   /* STEP 5: Select Flexcomm Function (1 = USART) */
   FLEXCOMM9->PSELID = 1;

   /* STEP 6: Enable the USART */
   USART9->CFG = USART_CFG_ENABLE_MASK | USART_CFG_DATALEN(1);

   /* STEP 7: Configure Baud Rate */
   USART9->BRG = (12000000 / (16 * baudrate)) - 1;

   /* STEP 8: Configure and Enable FIFOs */
   USART9->FIFOCFG |= (USART_FIFOCFG_ENABLETX_MASK | USART_FIFOCFG_ENABLERX_MASK);

}

void UART9_SendChar(char c) {
   while (!(USART9->FIFOSTAT & USART_FIFOSTAT_TXNOTFULL_MASK));
   USART9->FIFOWR = (uint32_t)c;
}

char UART9_ReceiveChar(void) {
   while (!(USART9->FIFOSTAT & USART_FIFOSTAT_RXNOTEMPTY_MASK));
   return (char)(USART9->FIFORD & 0xFF);
}

int main() {
    char rx_byte = 0; // Initialize to avoid junk logic
    uint32_t last_press_time = 0;
    uint32_t last_dot_time = 0; // NEW: To track heartbeat timing
    bool button_ready = true;

    LED_Init();
    UserButton_Init();
    UART9_Init(9600);
    SysTick_Config(SystemCoreClock / 1000);

    while (1) {
        /* 1. BUTTON LOGIC (Now checked constantly) */
        if (UserButton_IsPressed()) {
            uint32_t current_time = msTicks;
            if (button_ready && (current_time - last_press_time) > 200) {
                GPIO->NOT[2] = (1UL << 2);
                // Send a message over UART when the button is pressed
                const char* msg = "Button Pressed!\r\n";
               for (const char* p = msg; *p != '\0'; p++) {
                  UART9_SendChar(*p);
               }
                last_press_time = current_time;
                button_ready = false;
            }
        } else {
            button_ready = true;
        }

        /* 2. UART RECEIVE (Now checked constantly) */
        if (USART9->FIFOSTAT & USART_FIFOSTAT_RXNOTEMPTY_MASK) {
            rx_byte = UART9_ReceiveChar();
            UART9_SendChar(rx_byte);

            // Move COMMAND LOGIC inside this IF so it only runs on NEW data
            if (rx_byte == 'r' || rx_byte == 'R') {
                GPIO->NOT[2] = (1UL << 2);
            }
            else if (rx_byte == 'g' || rx_byte == 'G') {
                GPIO->NOT[3] = (1UL << 3);
            }
            else if (rx_byte == 'b' || rx_byte == 'B') {
                GPIO->NOT[3] = (1UL << 14);
            }
        }

        /* 3. NON-BLOCKING HEARTBEAT (The secret sauce) */
        // Check if 1000ms have passed since the last dot
        if ((msTicks - last_dot_time) >= 1000) {
            UART9_SendChar('.');
            last_dot_time = msTicks; // Reset timer for next dot
        }

        // NO DELAY_MS(1000) HERE!
        // The loop now spins at full speed.
    }
}


