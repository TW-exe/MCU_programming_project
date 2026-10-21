#include <stdint.h>
#define STM32G474xx
#include "stm32g4xx.h"
#include "uart.h"

void UART_Init(void) {
	// 1. Enable peripheral clocks
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
	RCC->APB1ENR1 |= RCC_APB1ENR1_USART3EN;
	// Route HSI16 clock to USART3
	RCC->CCIPR &= ~RCC_CCIPR_USART3SEL;
	RCC->CCIPR |= (2 << RCC_CCIPR_USART3SEL_Pos);

	// 2. Configure PC10 (TX) and PC11 (RX)
	GPIOC->MODER &= ~(GPIO_MODER_MODE10 | GPIO_MODER_MODE11);
	GPIOC->MODER |= (GPIO_MODER_MODE10_1 | GPIO_MODER_MODE11_1); // Alternate function mode
	// Route Alternate Function 7 (AF7) to PC10 and PC11
	GPIOC->AFR[1] &= ~((0xF << GPIO_AFRH_AFSEL10_Pos) | (0xF << GPIO_AFRH_AFSEL11_Pos));
	GPIOC->AFR[1] |= ((7 << GPIO_AFRH_AFSEL10_Pos) | (7 << GPIO_AFRH_AFSEL11_Pos));

	// 3. Configure USART3
	USART3->CR1 &= ~USART_CR1_UE; // Turn off to allow config
	// Set Baud Rate to 115200 (Assuming 16 MHz HSI clock source)
	USART3->BRR = 139;
	// Enable Transmitter (TE) and Receiver (RE)
	USART3->CR1 |= (USART_CR1_TE | USART_CR1_RE);
	// Enable USART3 (UE)
	USART3->CR1 |= USART_CR1_UE;
}


void UART_Tx(uint8_t byte) {
	// Wait until Transmit Data Register is Empty (TXE) flag is set
	while (!(USART3->ISR & USART_ISR_TXE));
	// Write the byte to the Transmit Data Register
	USART3->TDR = byte;
}

uint8_t UART_Rx(void) {
	// If an Overrun Error (ORE) occurs, clear it so we don't get permanently stuck
	if (USART3->ISR & USART_ISR_ORE) {
		USART3->ICR = USART_ICR_ORECF; // Write 1 to clear the Overrun flag
	}
	// Wait until the Read Data Register Not Empty (RXNE) flag is set
	while (!(USART3->ISR & USART_ISR_RXNE));
	// Reading RDR automatically clears the RXNE flag
	return (uint8_t)(USART3->RDR & 0xFF);
}


