#include "uart.h"
#include "relay.h"

#define GPIOAEN 				(1U << 2)
#define USART1EN				(1U << 14)
#define AFIOEN					(1U << 0)
#define DMA1EN					(1U << 0)
#define DMA_CCR5_EN				(1U << 0)
#define DMA_CCR5_MINC   		(1U << 7)
#define DMA_CCR5_CIRC   		(1U << 5)
#define DMA_CCR4_EN				(1U << 0)
#define DMA_CCR4_MINC   		(1U << 7)
#define DMA_CCR4_DIR			(1U << 4)

#define RX_RING_SIZE 256

volatile uint8_t rx_ring_buffer[RX_RING_SIZE];
volatile uint16_t head_ptr = 0;
uint16_t tail_ptr = 0;
static char app_rx_buffer[RX_RING_SIZE];

void UART1_DMA_Init(void) {
    RCC->APB2ENR |= GPIOAEN | USART1EN | AFIOEN;
    RCC->AHBENR  |= DMA1EN;

    GPIOA->CRH &= ~((0xFU << (1 * 4)) | (0xFU << (2 * 4)));
    GPIOA->CRH |=  (0xBU << (1 * 4));
    GPIOA->CRH |=  (0x4U << (2 * 4));

    USART1->BRR = 0x271;


    DMA1_Channel5->CCR &= ~DMA_CCR5_EN;

    DMA1->IFCR = DMA_IFCR_CGIF4 | DMA_IFCR_CGIF5;   // xóa cờ ngắt
    DMA1_Channel5->CPAR = (uint32_t)&USART1->DR;    // gắn DATA vào PER
    DMA1_Channel5->CMAR = (uint32_t)rx_ring_buffer; // gắn buffer vào MEM
    DMA1_Channel5->CNDTR = RX_RING_SIZE;            // length
    DMA1_Channel5->CCR = DMA_CCR5_MINC | DMA_CCR5_CIRC; // tự động tăng MEM và chế độ vòng

    DMA1_Channel5->CCR |= DMA_CCR5_EN;

    DMA1_Channel4->CPAR = (uint32_t)&USART1->DR;
    DMA1_Channel4->CCR = DMA_CCR4_MINC | DMA_CCR4_DIR;  // MEM to PER

    USART1->CR3 |= USART_CR3_DMAR | USART_CR3_DMAT;
    USART1->CR1 |= USART_CR1_IDLEIE;
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

    NVIC_SetPriority(USART1_IRQn, 1);
    NVIC_EnableIRQ(USART1_IRQn);
}

void UART1_DMA_Transmit(uint8_t *data, uint16_t len) {
	if (DMA1_Channel4->CCR & DMA_CCR4_EN) {
	        while (!(DMA1->ISR & DMA_ISR_TCIF4)) {}   //chờ truyền byte trước xong
	}
    DMA1_Channel4->CCR &= ~DMA_CCR4_EN;

    DMA1->IFCR = DMA_IFCR_CGIF4; // xóa cờ ngắt
    DMA1_Channel4->CMAR = (uint32_t)data;
    DMA1_Channel4->CNDTR = len;

    DMA1_Channel4->CCR |= DMA_CCR4_EN;
}


void USART1_IRQHandler(void) {
    if (USART1->SR & USART_SR_IDLE) {
        volatile uint32_t clear = USART1->SR;
        clear = USART1->DR;
        (void)clear;


        head_ptr = RX_RING_SIZE - DMA1_Channel5->CNDTR;
    }
}






bool UART1_Is_Data_Ready(void) {

    return (head_ptr != tail_ptr);
}

uint8_t* UART1_Get_Rx_Buffer(void) {
    uint16_t index = 0;
    uint16_t temp_tail = tail_ptr;


    while (temp_tail != head_ptr) {
        app_rx_buffer[index++] = rx_ring_buffer[temp_tail];
        temp_tail = (temp_tail + 1) % RX_RING_SIZE;
    }

    app_rx_buffer[index] = '\0';
    return (uint8_t*)app_rx_buffer;
}

void UART1_Clear_Data_Flag(void) {

    tail_ptr = head_ptr;
}

void UART1_Clear_Rx_Buffer(void) {

    memset(app_rx_buffer, 0, RX_RING_SIZE);
    tail_ptr = head_ptr;
}

