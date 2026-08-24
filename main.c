#include "hal.h"

static volatile uint32_t s_ticks; 
void SysTick_Handler(void) {
    s_ticks++;
}

int main(void) {
    uint16_t led = PIN('A', 5);
    gpio_set_mode(led, GPIO_MODE_OUTPUT);
    uart_init(UART2, 115200);
    uint32_t timer = 0, period = 500;

    for(;;) {
        if (timer_expired(&timer, period, s_ticks)) {
            static bool on;         // This block is executed
            gpio_write(led, on);    // Every 'period' milliseconds
            on = !on;               // Toggle LED state
            printf("LED: %d, tick: %lu\r\n", on, s_ticks);
        }

        // Here we could perform other activities
    }
    return 0;
}
