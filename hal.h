#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "stm32f411xe.h"

#define FREQ 16000000
#define BIT(x) (1UL << (x))
#define PIN(bank, num) ((((bank) - 'A') << 8) | (num))
#define PINNO(pin) (pin & 255)
#define PINBANK(pin) (pin >> 8)

enum { APB1_PRE = 4, APB2_PRE = 3 };
enum { PLL_HSI = 16, PLL_M = 8, PLL_N = 100, PLL_P = 2 };
#define FLASH_LATENCY 3 
#define SYS_FREQUENCY ((PLL_HSI / PLL_M * PLL_N / PLL_P) * 1000000)
#define APB1_FREQUENCY (SYS_FREQUENCY / (BIT(APB1_PRE - 3)))
#define APB2_FREQUENCY (SYS_FREQUENCY / (BIT(APB2_PRE - 3)))

static inline void spin(volatile uint32_t count) {
    while (count--) (void) 0;
}

#define GPIO(bank) ((GPIO_TypeDef*) (GPIOA_BASE + 0x400 * (bank)))

// Enum values are per datasheet: 0, 1, 2, 3
enum { GPIO_MODE_INPUT, GPIO_MODE_OUTPUT, GPIO_MODE_AF, GPIO_MODE_ANALOG };


static inline void gpio_set_mode(uint16_t pin, uint8_t mode) {
    GPIO_TypeDef* gpio = GPIO(PINBANK(pin));
    int n = PINNO(pin);
    RCC->AHB1ENR |= BIT(PINBANK(pin)); 
    gpio->MODER &= ~(3U << (n * 2));
    gpio->MODER |= (mode & 3U) << (n * 2);
}

static inline void gpio_set_af(uint16_t pin, uint8_t af_num) {
    GPIO_TypeDef* gpio = GPIO(PINBANK(pin));
    int n = PINNO(pin);
    gpio->AFR[n >> 3] &= ~(15UL << ((n & 7) * 4));
    gpio->AFR[n >> 3] |= ((uint32_t) af_num) << ((n & 7) * 4);
}

static inline void gpio_write(uint16_t pin, bool val) {
    GPIO_TypeDef* gpio = GPIO(PINBANK(pin));
    gpio->BSRR = (1U << PINNO(pin)) << (val ? 0 : 16);
}

#define UART1 USART1
#define UART2 USART2 
#define UART6 USART6 

static inline bool uart_init(USART_TypeDef* uart, unsigned long baud) {
    uint8_t af = 7;
    uint16_t rx = 0, tx = 0;
    uint32_t freq = 0;

    if (uart == UART1) {
        freq = APB2_FREQUENCY, RCC->APB2ENR |= BIT(4);
        tx = PIN('A', 9), rx = PIN('A', 10);
    } else if (uart == UART2) {
        freq = APB1_FREQUENCY, RCC->APB1ENR |= BIT(17);
        tx = PIN('A', 2), rx = PIN('A', 3);
    } else if (uart == UART6) {
        freq = APB2_FREQUENCY, RCC->APB2ENR |= BIT(5), af = 8;
        tx = PIN('C', 6), rx = PIN('C', 7);
    } else {
        return false;
    }
 
    gpio_set_mode(tx, GPIO_MODE_AF);
    gpio_set_af(tx, af);
    gpio_set_mode(rx, GPIO_MODE_AF);
    gpio_set_af(rx, af);
    uart->CR1 = 0;
    uart->BRR = freq / baud;
    uart->CR1 |= BIT(13) | BIT(2) | BIT(3);
    return true;
}

static inline void uart_write_byte(USART_TypeDef* uart, uint8_t byte) {
    while ((uart->SR & BIT(7)) == 0) spin(1);
    uart->DR = byte;
}

static inline void uart_write_buf(USART_TypeDef* uart, char* buf, size_t len) {
    while (len-- > 0) uart_write_byte(uart, *(uint8_t*) buf++);
}

static inline int uart_read_ready(USART_TypeDef* uart) {
    return uart->SR & BIT(5);
}

static inline uint8_t uart_read_byte(USART_TypeDef* uart) {
    return (uint8_t) (uart->DR & 255);
}

// t: expiration time, prd: period, now: current time. Return true if expired
static inline bool timer_expired(uint32_t* t, uint32_t prd, uint32_t now) {
    if (now + prd < *t) *t = 0;                     // Time wrapped? Reset timer
    if (*t == 0) *t = now + prd;                    // First poll? Set expiration
    if (*t > now) return false;                     // Not expired yet, return
    *t = (now - *t) > prd ? now + prd : *t + prd;   // Next expiration time
    return true;                                    // Expired, return true
}

