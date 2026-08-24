#include "hal.h"


uint32_t SystemCoreClock = SYS_FREQUENCY;

void SystemInit(void) {
    FLASH->ACR |= FLASH_LATENCY | BIT(8) | BIT(9);
    RCC->PLLCFGR &= ~((BIT(17) - 1));
    RCC->PLLCFGR |= (((PLL_P - 2) / 2) & 3) << 16;
    RCC->PLLCFGR |= PLL_M | (PLL_N << 6);
    RCC->CR |= BIT(24);
    while ((RCC->CR & BIT(25)) == 0) spin(1);
    RCC->CFGR = (APB1_PRE << 10) | (APB2_PRE << 13);
    RCC->CFGR |= 2;
    while ((RCC->CFGR & 12) == 0) spin(1);

    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SysTick_Config(SystemCoreClock / 1000);
}
