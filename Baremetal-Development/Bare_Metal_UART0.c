#include "LPC54628.h"

volatile uint32_t msTicks = 0;

// CMSIS standard SysTick Handler
void SysTick_Handler(void) {
    msTicks++;
}

void delay_ms(uint32_t ms) {
    uint32_t startTicks = msTicks;
    while ((msTicks - startTicks) < ms);
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

void UART0_Init(uint32_t baudrate) {
    /* 1. Enable Peripheral Clocks */
    // IOCON is in AHBCLKCTRL[0], Flexcomm 0 is in AHBCLKCTRL[1]
    SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL_IOCON_MASK;
    SYSCON->AHBCLKCTRLSET[1] = SYSCON_AHBCLKCTRL_FLEXCOMM0_MASK;

    /* 2. Select Clock Source for FC0 (0 = FRO 12MHz) */
    SYSCON->FCLKSEL[0] = 0;

    /* 3. Clear Reset for Flexcomm 0 */
    // Note: Use the FC0 reset mask in PRESETCTRLCLR[1]
    SYSCON->PRESETCTRLCLR[1] = SYSCON_PRESETCTRL_FC0_RST_MASK;

    /* 4. Configure Pins for Debug Console (PIO0_29 and PIO0_30) */
    // Func 1 is UART. Bit 7 is Digital Mode for Port 0.
    IOCON->PIO[0][29] = (1 << 0) | (1 << 8); // RX
    IOCON->PIO[0][30] = (1 << 0) | (1 << 8); // TX

    /* 5. Set Flexcomm 0 to USART mode */
    FLEXCOMM0->PSELID = 1;

    /* 6. USART Configuration */
    // Set 8-bit length and Enable the USART
    USART0->CFG = USART_CFG_ENABLE_MASK | USART_CFG_DATALEN(1);
    // Baud Rate: (12MHz / (16 * baudrate)) - 1
    USART0->BRG = (12000000 / (16 * baudrate)) - 1;

    /* 7. Enable both Transmit and Receive FIFOs */
    // Bit 0: Enable TX, Bit 1: Enable RX
    USART0->FIFOCFG |= (USART_FIFOCFG_ENABLETX_MASK | USART_FIFOCFG_ENABLERX_MASK);
}

void UART0_SendChar(char c) {
    // Wait until TX FIFO is not full
    while (!(USART0->FIFOSTAT & USART_FIFOSTAT_TXNOTFULL_MASK));
    // Write character to the FIFO Data Write register
    USART0->FIFOWR = (uint32_t)c;
}

char UART0_ReceiveChar(void) {
    /* Wait until the RX FIFO is NOT empty (Bit 0: RXNOTEMPTY) */
    while (!(USART0->FIFOSTAT & USART_FIFOSTAT_RXNOTEMPTY_MASK));

    /* Return the data from the FIFO data register */
    return (char)(USART0->FIFORD & 0xFF);
}

int main() {
   char rx_byte;

   /* Step 1: Initialize Peripherals */
   LED_Init();
   UART0_Init(9600);

   SysTick_Config(SystemCoreClock / 1000);

   while (1) {
       /* BLOCKING RECEIVE: Wait for a character to arrive */
       if (USART0->FIFOSTAT & USART_FIFOSTAT_RXNOTEMPTY_MASK) {
            rx_byte = UART0_ReceiveChar();
            /* ECHO: Send it back so you see it on your Saleae/Terminal */
            UART0_SendChar(rx_byte);
        }

       /* COMMAND LOGIC */
       if (rx_byte == 'r' || rx_byte == 'R') {
           GPIO->NOT[2] = (1UL << 2);    // Toggle Green
       }
       else if (rx_byte == 'g' || rx_byte == 'G') {
           GPIO->NOT[3] = (1UL << 3);    // Toggle Red
       }
       else if (rx_byte == 'b' || rx_byte == 'B') {
           GPIO->NOT[3] = (1UL << 14);
       }

       UART0_SendChar('.'); // Send a dot every second to show the board is alive
       delay_ms(1000);
   }
}