#include "uart.h"
#include "relay.h" // Nhớ include thư viện relay để gọi relay_set_state()

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

void UART1_DMA_Init(void) {
    RCC->APB2ENR |= GPIOAEN | USART1EN | AFIOEN;
    RCC->AHBENR  |= DMA1EN;

    GPIOA->CRH &= ~((0xFU << (1 * 4)) | (0xFU << (2 * 4)));
    GPIOA->CRH |=  (0xBU << (1 * 4));
    GPIOA->CRH |=  (0x4U << (2 * 4));

    USART1->BRR = 0x271;

    // --- SỬA LỖI 1: Trỏ đúng vào Ring Buffer ---
    DMA1_Channel5->CCR &= ~DMA_CCR5_EN;
    DMA1->IFCR = DMA_IFCR_CGIF4 | DMA_IFCR_CGIF5;
    DMA1_Channel5->CPAR = (uint32_t)&USART1->DR;
    DMA1_Channel5->CMAR = (uint32_t)rx_ring_buffer; // Trỏ vào mảng vòng
    DMA1_Channel5->CNDTR = RX_RING_SIZE;            // Kích thước mảng vòng
    DMA1_Channel5->CCR = DMA_CCR5_MINC | DMA_CCR5_CIRC;
    DMA1_Channel5->CCR |= DMA_CCR5_EN;

    DMA1_Channel4->CPAR = (uint32_t)&USART1->DR;
    DMA1_Channel4->CCR = DMA_CCR4_MINC | DMA_CCR4_DIR;

    USART1->CR3 |= USART_CR3_DMAR | USART_CR3_DMAT;
    USART1->CR1 |= USART_CR1_IDLEIE;
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

    NVIC_SetPriority(USART1_IRQn, 1);
    NVIC_EnableIRQ(USART1_IRQn);
}

void UART1_DMA_Transmit(uint8_t *data, uint16_t len) {
	if (DMA1_Channel4->CCR & DMA_CCR4_EN) {
	        while (!(DMA1->ISR & DMA_ISR_TCIF4)) {}
	}
    DMA1_Channel4->CCR &= ~DMA_CCR4_EN;
    DMA1->IFCR = DMA_IFCR_CGIF4;
    DMA1_Channel4->CMAR = (uint32_t)data;
    DMA1_Channel4->CNDTR = len;
    DMA1_Channel4->CCR |= DMA_CCR4_EN;
}

// --- SỬA LỖI 2: ISR chỉ chốt con trỏ, KHÔNG khởi động lại DMA ---
void USART1_IRQHandler(void) {
    if (USART1->SR & USART_SR_IDLE) {
        volatile uint32_t clear = USART1->SR;
        clear = USART1->DR;
        (void)clear;

        // CNDTR tự động giảm từ 256 về 0. Trừ đi sẽ ra vị trí ghi hiện tại.
        head_ptr = RX_RING_SIZE - DMA1_Channel5->CNDTR;
    }
}

void UART_Process_MQTT_Payload(void) {
    char payload_cmd[50];
    uint8_t index = 0;

    while (tail_ptr != head_ptr) {
        char c = rx_ring_buffer[tail_ptr];

        if (index < 49) {
            payload_cmd[index++] = c;
        }
        tail_ptr = (tail_ptr + 1) % RX_RING_SIZE;
    }

    if (index > 0) {
        payload_cmd[index] = '\0';

        if (strstr(payload_cmd, "ON") != NULL) {
            relay_set_state(RELAY_ON);
            // Có thể thêm: manual_override = true;
        }
        else if (strstr(payload_cmd, "OFF") != NULL) {
            relay_set_state(RELAY_OFF);
        }
    }
}

// Bộ đệm tuyến tính trung gian trả về cho tầng Application
static char app_rx_buffer[RX_RING_SIZE];

bool UART1_Is_Data_Ready(void) {
    // Có dữ liệu khi con trỏ đọc (Tail) chưa đuổi kịp con trỏ ghi (Head)
    return (head_ptr != tail_ptr);
}

uint8_t* UART1_Get_Rx_Buffer(void) {
    uint16_t index = 0;
    uint16_t temp_tail = tail_ptr;

    // Rút toàn bộ dữ liệu từ Ring Buffer ra mảng tuyến tính để dùng hàm strstr()
    while (temp_tail != head_ptr) {
        app_rx_buffer[index++] = rx_ring_buffer[temp_tail];
        temp_tail = (temp_tail + 1) % RX_RING_SIZE;
    }

    app_rx_buffer[index] = '\0'; // Chốt chuỗi String (Null-terminated)
    return (uint8_t*)app_rx_buffer;
}

void UART1_Clear_Data_Flag(void) {
    // Ép con trỏ đọc nhảy thẳng đến con trỏ ghi -> Đánh dấu đã đọc hết toàn bộ dữ liệu
    tail_ptr = head_ptr;
}

void UART1_Clear_Rx_Buffer(void) {
    // Xóa bộ đệm trung gian và đồng bộ lại con trỏ Ring Buffer
    memset(app_rx_buffer, 0, RX_RING_SIZE);
    tail_ptr = head_ptr;
}

