
// #include "uart.h"

// #include "relay.h"



// #define GPIOAEN (1U << 2)

// #define USART1EN (1U << 14)

// #define AFIOEN (1U << 0)

// #define DMA1EN (1U << 0)

// #define DMA_CCR5_EN (1U << 0)

// #define DMA_CCR5_MINC (1U << 7)

// #define DMA_CCR5_CIRC (1U << 5)

// #define DMA_CCR4_EN (1U << 0)

// #define DMA_CCR4_MINC (1U << 7)

// #define DMA_CCR4_DIR (1U << 4)



// #define RX_RING_SIZE 256



// volatile uint8_t rx_ring_buffer[RX_RING_SIZE];

// volatile uint16_t head_ptr = 0;

// uint16_t tail_ptr = 0;

// static char app_rx_buffer[RX_RING_SIZE];



// void UART1_DMA_Init(void) {

// RCC->APB2ENR |= GPIOAEN | USART1EN | AFIOEN;

// RCC->AHBENR |= DMA1EN;



// GPIOA->CRH &= ~((0xFU << (1 * 4)) | (0xFU << (2 * 4)));

// GPIOA->CRH |= (0xBU << (1 * 4));

// GPIOA->CRH |= (0x4U << (2 * 4));



// USART1->BRR = 0x271;





// DMA1_Channel5->CCR &= ~DMA_CCR5_EN;



// DMA1->IFCR = DMA_IFCR_CGIF4 | DMA_IFCR_CGIF5; // xóa cờ ngắt

// DMA1_Channel5->CPAR = (uint32_t)&USART1->DR; // gắn DATA vào PER

// DMA1_Channel5->CMAR = (uint32_t)rx_ring_buffer; // gắn buffer vào MEM

// DMA1_Channel5->CNDTR = RX_RING_SIZE; // length

// DMA1_Channel5->CCR = DMA_CCR5_MINC | DMA_CCR5_CIRC; // tự động tăng MEM và chế độ vòng



// DMA1_Channel5->CCR |= DMA_CCR5_EN;



// DMA1_Channel4->CPAR = (uint32_t)&USART1->DR;

// DMA1_Channel4->CCR = DMA_CCR4_MINC | DMA_CCR4_DIR; // MEM to PER



// USART1->CR3 |= USART_CR3_DMAR | USART_CR3_DMAT;

// USART1->CR1 |= USART_CR1_IDLEIE;

// USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;



// NVIC_SetPriority(USART1_IRQn, 1);

// NVIC_EnableIRQ(USART1_IRQn);

// }



// void UART1_DMA_Transmit(uint8_t *data, uint16_t len) {

// if (DMA1_Channel4->CCR & DMA_CCR4_EN) {

// uint32_t timeout = 0xFFFFF; // Timeout an toàn (khoảng vài ms tại 72MHz)

// while (!(DMA1->ISR & DMA_ISR_TCIF4) && --timeout) {} // Chờ truyền xong hoặc hết giờ

// }

// DMA1_Channel4->CCR &= ~DMA_CCR4_EN;



// DMA1->IFCR = DMA_IFCR_CGIF4; // xóa cờ ngắt

// DMA1_Channel4->CMAR = (uint32_t)data;

// DMA1_Channel4->CNDTR = len;



// DMA1_Channel4->CCR |= DMA_CCR4_EN;

// }


// void USART1_IRQHandler(void) {

// uint32_t sr = USART1->SR; // Đọc trạng thái 1 lần để tối ưu chu kỳ máy



// // 1. Xử lý ngắt IDLE (Nhận xong 1 frame dữ liệu)

// if (sr & USART_SR_IDLE) {

// volatile uint32_t dr = USART1->DR; // Đọc DR để xóa cờ IDLE

// (void)dr;
// }

// if (sr & (USART_SR_ORE | USART_SR_NE | USART_SR_FE | USART_SR_PE)) {

// volatile uint32_t dr = USART1->DR;

// (void)dr;

// }

// }

// bool UART1_Is_Data_Ready(void) {

// // Đọc trực tiếp phần cứng: Dữ liệu vào đến đâu, FSM biết ngay đến đó

// head_ptr = RX_RING_SIZE - DMA1_Channel5->CNDTR;



// return (head_ptr != tail_ptr);

// }





// static char app_rx_buffer[RX_RING_SIZE];

// // Thêm biến toàn cục tĩnh này để lưu vị trí nối chuỗi





// #include <string.h>



// static uint16_t app_index = 0;



// uint8_t* UART1_Get_Rx_Buffer(void) {

// // Cập nhật vị trí head_ptr mới nhất từ DMA trước khi xử lý

// head_ptr = RX_RING_SIZE - DMA1_Channel5->CNDTR;

// uint16_t current_head = head_ptr;



// while (tail_ptr != current_head) {

// char c = (char)rx_ring_buffer[tail_ptr];



// // 1. BỘ LỌC CHỐNG NHIỄU: Ép bỏ qua các byte Null (0x00) và rác phần cứng (0xFF)

// if (c != '\0' && c != (char)0xFF) {



// // 2. CƠ CHẾ SLIDING WINDOW:

// if (app_index >= RX_RING_SIZE - 1) {

// memmove(app_rx_buffer, app_rx_buffer + 1, RX_RING_SIZE - 2);

// app_index = RX_RING_SIZE - 2;

// }

// app_rx_buffer[app_index++] = c;

// }

// tail_ptr = (tail_ptr + 1) % RX_RING_SIZE;

// }



// app_rx_buffer[app_index] = '\0'; // Đóng chuỗi chuẩn C

// return (uint8_t*)app_rx_buffer;

// }



// void UART1_Clear_Rx_Buffer(void) {

// memset(app_rx_buffer, 0, RX_RING_SIZE);

// app_index = 0;



// // Ép đuôi đuổi kịp đầu DMA hiện tại

// head_ptr = RX_RING_SIZE - DMA1_Channel5->CNDTR;

// tail_ptr = head_ptr;

// }

// // Bỏ trống hoàn toàn

// void UART1_Clear_Data_Flag(void) {

// }
#include "uart.h"

static uint8_t rx_dma_buffer[RX_RING_SIZE];
static volatile bool data_ready_flag = false;

void UART1_DMA_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    /* PA9 (TX): AF Push-Pull 50MHz, PA10 (RX): Input Pull-up */
    GPIOA->CRH &= ~((0xFU << 4) | (0xFU << 8));
    GPIOA->CRH |= ((0xBU << 4) | (0x8U << 8));
    GPIOA->ODR |= (1U << 10);
    /* Baudrate = 115200 @ 72MHz */
    USART1->BRR = 0x0271;
    /* DMA1_Channel5 cho USART1_RX */
    DMA1_Channel5->CCR &= ~DMA_CCR_EN;
    DMA1_Channel5->CPAR = (uint32_t)&USART1->DR;
    DMA1_Channel5->CMAR = (uint32_t)rx_dma_buffer;
    DMA1_Channel5->CNDTR = RX_RING_SIZE - 1U;
    DMA1_Channel5->CCR = DMA_CCR_MINC | DMA_CCR_CIRC | DMA_CCR_EN;
    /* DMA1_Channel4 cho USART1_TX */
    DMA1_Channel4->CCR &= ~DMA_CCR_EN;
    DMA1_Channel4->CPAR = (uint32_t)&USART1->DR;
    DMA1_Channel4->CCR = DMA_CCR_MINC | DMA_CCR_DIR;
    USART1->CR3 |= USART_CR3_DMAT | USART_CR3_DMAR;
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_IDLEIE | USART_CR1_UE;
    NVIC_SetPriority(USART1_IRQn, 0);
    NVIC_EnableIRQ(USART1_IRQn);
}

void UART1_DMA_Transmit(uint8_t *data, uint16_t len) {
    if (len == 0) {
        return;
    }
    if (DMA1_Channel4->CCR & DMA_CCR_EN) {
        while (!(DMA1->ISR & DMA_ISR_TCIF4)) {}
        DMA1->IFCR = DMA_IFCR_CGIF4;
        DMA1_Channel4->CCR &= ~DMA_CCR_EN;
    }
    DMA1_Channel4->CMAR = (uint32_t)data;
    DMA1_Channel4->CNDTR = len;
    DMA1_Channel4->CCR |= DMA_CCR_EN;
}

void USART1_IRQHandler(void) {
    if (USART1->SR & USART_SR_IDLE) {
        (void)USART1->SR;
        (void)USART1->DR;
        uint16_t received_len = (RX_RING_SIZE - 1U) - (uint16_t)DMA1_Channel5->CNDTR;
        if (received_len > 0) {
            rx_dma_buffer[received_len] = '\0';
            data_ready_flag = true;
        }
    }
}

bool UART1_Is_Data_Ready(void) {
    return data_ready_flag;
}

void UART1_Clear_Data_Flag(void) {
    data_ready_flag = false;
}

uint8_t* UART1_Get_Rx_Buffer(void) {
    return rx_dma_buffer;
}

void UART1_Clear_Rx_Buffer(void) {
    DMA1_Channel5->CCR &= ~DMA_CCR_EN;
    memset(rx_dma_buffer, 0, RX_RING_SIZE);
    DMA1_Channel5->CNDTR = RX_RING_SIZE - 1U;
    data_ready_flag = false;
    DMA1_Channel5->CCR |= DMA_CCR_EN;
}
