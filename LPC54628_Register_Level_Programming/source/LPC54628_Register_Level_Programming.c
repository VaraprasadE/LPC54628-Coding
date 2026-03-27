#include "LPC54628.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>                // for fixed width types

/* accelerometer definitions */
#define ACCEL_I2C_ADDR           0x1DU
#define ACCEL_REG_OUT_X_MSB      0x01U
#define ACCEL_REG_WHO_AM_I       0x0DU
#define ACCEL_REG_XYZ_DATA_CFG   0x0EU
#define ACCEL_REG_CTRL_REG1      0x2AU
#define ACCEL_WHO_AM_I_VALUE     0x4AU

void UART0_Init(uint32_t baudrate) {
    /* STEP 1: Enable Peripheral Clocks */
    SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL_IOCON_MASK;
    SYSCON->AHBCLKCTRLSET[1] = SYSCON_AHBCLKCTRL_FLEXCOMM0_MASK;

    /* STEP 2: Select Functional Clock Source (Index [0] for FC0) */
    SYSCON->FCLKSEL[0] = 0; // 0 = FRO 12MHz

    /* STEP 3: Release Peripheral from Reset (Index [1] for FC0) */
    SYSCON->PRESETCTRLCLR[1] = SYSCON_PRESETCTRL_FC0_RST_MASK;

    /* STEP 4: Configure Pin Multiplexing (IOCON) */
    // PIO0_29 = RX, PIO0_30 = TX
    IOCON->PIO[0][29] = (1 << 0) | (1 << 8);
    IOCON->PIO[0][30] = (1 << 0) | (1 << 8);

    /* STEP 5: Select Flexcomm Function (1 = USART) */
    FLEXCOMM0->PSELID = 1;

    /* STEP 6: Enable the USART */
    USART0->CFG = USART_CFG_ENABLE_MASK | USART_CFG_DATALEN(1);

    /* STEP 7: Configure Baud Rate */
    USART0->BRG = (12000000 / (16 * baudrate)) - 1;

    /* STEP 8: Configure and Enable FIFOs */
    USART0->FIFOCFG |= (USART_FIFOCFG_ENABLETX_MASK | USART_FIFOCFG_ENABLERX_MASK);
}

void UART0_SendChar(char c) {
    while (!(USART0->FIFOSTAT & USART_FIFOSTAT_TXNOTFULL_MASK));
    USART0->FIFOWR = (uint32_t)c;
}

char UART0_ReceiveChar(void) {
    while (!(USART0->FIFOSTAT & USART_FIFOSTAT_RXNOTEMPTY_MASK));
    return (char)(USART0->FIFORD & 0xFF);
}

void UART0_SendString(const char *message) {
    while (*message != '\0') {
        UART0_SendChar(*message);
        message++;
    }
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

void UART9_SendString(const char *message) {
    while (*message != '\0') {
        UART9_SendChar(*message);
        message++;
    }
}

void UART_BroadcastString(const char *message) {
    UART0_SendString(message);
    UART9_SendString(message);
}

static void UART0_SendPrompt(void) {
    UART0_SendString("> ");
}

static void UART_BroadcastCommand(char command_byte) {
    UART0_SendString("CMD: ");
    UART0_SendChar(command_byte);
    UART0_SendString("\r\n");

    UART9_SendString("CMD: ");
    UART9_SendChar(command_byte);
    UART9_SendString("\r\n");
}

/* ---------------------------------------------------------------------------
   Accelerometer (MMA8652) I2C helpers using Flexcomm2 as master
   --------------------------------------------------------------------------- */

void Accel_I2C_Init(void) {
    /* 1. Enable Clocks */
    // IOCON is at AHBCLKCTRL0[13]
    SYSCON->AHBCLKCTRLSET[0] = (1UL << 13);          
    
    // Flexcomm 2 is at AHBCLKCTRL1[13]
    SYSCON->AHBCLKCTRLSET[1] = (1UL << 13);          

    /* 2. Select Clock Source for FC2 */
    // FCLKSEL index matches the Flexcomm number: FC2 = index 2
    SYSCON->FCLKSEL[2] = 0; /* 0 = FRO 12MHz */

    /* 3. Clear Peripheral Reset */
    // PRESETCTRL1 index 1, Bit 13 is FC2
    SYSCON->PRESETCTRLCLR[1] = (1UL << 13);

    /* 4. Configure I2C pins PIO3_23 SDA & PIO3_24 SCL */
    // port 3, pin 23/24
    // FUNC 1 = I2C, DIGIMODE = 1 (Bit 8). 
    // Note: I2C pins on P3 often need the I2CSLEW/FILTER bits in 11:10
    IOCON->PIO[3][23] = (1 << 0) | (1 << 8) | (1 << 11); // SDA
    IOCON->PIO[3][24] = (1 << 0) | (1 << 8) | (1 << 11); // SCL

    /* 5. Select I2C function in Flexcomm block */
    FLEXCOMM2->PSELID = 3;

    /* 6. Configure I2C clock rate (~400kHz) */
    // 12MHz / (2 * 15) = 400kHz roughly. Adjust CLKDIV based on MSTTIME.
    I2C2->CLKDIV = 2;                         
    I2C2->MSTTIME = (3 << 4) | (3 << 0); // SCL High/Low time      

    /* 7. Enable master mode */
    I2C2->CFG = (1 << 0);
}

static uint32_t Accel_I2C_WaitPending(void) {
    while ((I2C2->STAT & I2C_STAT_MSTPENDING_MASK) == 0U) {
    }

    return (I2C2->STAT & I2C_STAT_MSTSTATE_MASK);
}

static void Accel_I2C_Stop(void) {
    I2C2->MSTCTL = I2C_MSTCTL_MSTSTOP_MASK;
    (void)Accel_I2C_WaitPending();
}

bool Accel_WriteRegister(uint8_t reg_addr, uint8_t data) {
    uint32_t state;

    /* wait for idle */
    (void)Accel_I2C_WaitPending();

    I2C2->MSTDAT = (ACCEL_I2C_ADDR << 1) | 0U;
    I2C2->MSTCTL = I2C_MSTCTL_MSTSTART_MASK;

    state = Accel_I2C_WaitPending();
    if (state != I2C_STAT_MSTSTATE(0x2)) {
        Accel_I2C_Stop();
        return false;
    }

    I2C2->MSTDAT = reg_addr;
    I2C2->MSTCTL = I2C_MSTCTL_MSTCONTINUE_MASK;

    state = Accel_I2C_WaitPending();
    if (state != I2C_STAT_MSTSTATE(0x2)) {
        Accel_I2C_Stop();
        return false;
    }

    I2C2->MSTDAT = data;
    I2C2->MSTCTL = I2C_MSTCTL_MSTCONTINUE_MASK;

    state = Accel_I2C_WaitPending();
    if (state != I2C_STAT_MSTSTATE(0x2)) {
        Accel_I2C_Stop();
        return false;
    }

    Accel_I2C_Stop();
    return true;
}

bool Accel_ReadRegisters(uint8_t start_reg, uint8_t *data, uint32_t length) {
    uint32_t index;
    uint32_t state;

    if ((data == 0) || (length == 0U)) {
        return false;
    }

    /* wait for idle */
    (void)Accel_I2C_WaitPending();

    I2C2->MSTDAT = (ACCEL_I2C_ADDR << 1) | 0U;
    I2C2->MSTCTL = I2C_MSTCTL_MSTSTART_MASK;

    state = Accel_I2C_WaitPending();
    if (state != I2C_STAT_MSTSTATE(0x2)) {
        Accel_I2C_Stop();
        return false;
    }

    I2C2->MSTDAT = start_reg;
    I2C2->MSTCTL = I2C_MSTCTL_MSTCONTINUE_MASK;

    state = Accel_I2C_WaitPending();
    if (state != I2C_STAT_MSTSTATE(0x2)) {
        Accel_I2C_Stop();
        return false;
    }

    I2C2->MSTDAT = (ACCEL_I2C_ADDR << 1) | 1U;
    I2C2->MSTCTL = I2C_MSTCTL_MSTSTART_MASK;

    for (index = 0; index < length; index++) {
        state = Accel_I2C_WaitPending();
        if (state != I2C_STAT_MSTSTATE(0x1)) {
            Accel_I2C_Stop();
            return false;
        }

        data[index] = (uint8_t)(I2C2->MSTDAT & I2C_MSTDAT_DATA_MASK);

        if (index + 1U < length) {
            I2C2->MSTCTL = I2C_MSTCTL_MSTCONTINUE_MASK;
        } else {
            I2C2->MSTCTL = I2C_MSTCTL_MSTSTOP_MASK;
        }
    }

    (void)Accel_I2C_WaitPending();
    return true;
}

uint8_t Accel_ReadRegister(uint8_t reg_addr) {
    uint8_t value = 0;

    if (!Accel_ReadRegisters(reg_addr, &value, 1U)) {
        return 0;
    }

    return value;
}

bool Accel_ReadAxes(int16_t *x_axis, int16_t *y_axis, int16_t *z_axis) {
    uint8_t raw_data[6];

    if ((x_axis == 0) || (y_axis == 0) || (z_axis == 0)) {
        return false;
    }

    if (!Accel_ReadRegisters(ACCEL_REG_OUT_X_MSB, raw_data, 6U)) {
        return false;
    }

    *x_axis = (int16_t)((((uint16_t)raw_data[0]) << 8) | raw_data[1]) >> 4;
    *y_axis = (int16_t)((((uint16_t)raw_data[2]) << 8) | raw_data[3]) >> 4;
    *z_axis = (int16_t)((((uint16_t)raw_data[4]) << 8) | raw_data[5]) >> 4;

    return true;
}

bool Accel_DeviceInit(void) {
    uint8_t who_am_i = Accel_ReadRegister(ACCEL_REG_WHO_AM_I);

    if (who_am_i != ACCEL_WHO_AM_I_VALUE) {
        return false;
    }

    if (!Accel_WriteRegister(ACCEL_REG_CTRL_REG1, 0x00U)) {
        return false;
    }

    if (!Accel_WriteRegister(ACCEL_REG_XYZ_DATA_CFG, 0x00U)) {
        return false;
    }

    return Accel_WriteRegister(ACCEL_REG_CTRL_REG1, 0x01U);
}

int main() {
    bool accel_ready;
    char rx_byte;
    char message[96];
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;

    UART0_Init(9600);
    UART9_Init(9600);

    Accel_I2C_Init();
    accel_ready = Accel_DeviceInit();

    UART_BroadcastString("UART0 command interface ready. Send 'a' to read accelerometer.\r\n");
    if (accel_ready) {
        UART_BroadcastString("Accelerometer detected on I2C.\r\n");
    } else {
        UART_BroadcastString("Accelerometer init failed.\r\n");
    }
    UART0_SendPrompt();

    while (1) {
        if (USART0->FIFOSTAT & USART_FIFOSTAT_RXNOTEMPTY_MASK) {
            rx_byte = UART0_ReceiveChar();

            if ((rx_byte == '\r') || (rx_byte == '\n')) {
                continue;
            }

            UART_BroadcastCommand(rx_byte);

            if ((rx_byte == 'a') || (rx_byte == 'A')) {
                if (!accel_ready) {
                    accel_ready = Accel_DeviceInit();
                }

                if (accel_ready && Accel_ReadAxes(&accel_x, &accel_y, &accel_z)) {
                    (void)snprintf(message,
                                   sizeof(message),
                                   "ACCEL X=%d Y=%d Z=%d\r\n",
                                   accel_x,
                                   accel_y,
                                   accel_z);
                    UART_BroadcastString(message);
                } else {
                    UART_BroadcastString("ACCEL READ FAILED\r\n");
                }
            } else if ((rx_byte == '?') || (rx_byte == 'h') || (rx_byte == 'H')) {
                UART_BroadcastString("Commands: a=read accel, h/?=help\r\n");
            } else {
                (void)snprintf(message, sizeof(message), "Unknown command '%c'\r\n", rx_byte);
                UART_BroadcastString(message);
            }

            UART0_SendPrompt();
        }
    }
}
