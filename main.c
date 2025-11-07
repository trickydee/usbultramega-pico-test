// Simple RP2040 program that prints "Hello" over UART0 (GPIO 0/1).
#include "pico/stdlib.h"
#include "hardware/uart.h"

int main() {
    const uint UART_ID = uart0;
    const uint UART_TX_PIN = 0;
    const uint UART_RX_PIN = 1;
    const uint BAUD_RATE = 115200;

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_ID, true);

    uart_puts(UART_ID, "Hello from UART0 on GP0!\r\n");

    while (true) {
        tight_loop_contents();
    }
}

