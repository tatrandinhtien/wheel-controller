#include "main.h"
#include "system_stm32f1xx.h"
#include "Driver_RCC.h"

void delay(volatile uint32_t count) {
    while(count--) {
        __asm("nop"); // Lệnh rỗng (No Operation) để tránh GCC tối ưu hóa vòng lặp
    }
}
int main(void) {
    // 1. GỌI HÀM CỦA BẠN: Ép xung lên 72MHz
    RCC_SystemClock_72Mhz();
    SystemCoreClockUpdate();

    // 2. CODE DÃ CHIẾN: Cấu hình chân PC13 làm Output để nháy LED
    // Bật xung nhịp cho Port C (Bit 4 của thanh ghi APB2ENR)
    RCC->APB2ENR |= (1U << 4);
    
    // Cấu hình PC13 là Output Push-Pull, tốc độ 50MHz (Thanh ghi CRH, bit 20-23)
    GPIOC->CRH &= ~(0xFU << 20); // Xóa cấu hình cũ của chân 13
    GPIOC->CRH |=  (0x3U << 20); // Set mode 0b0011 (Output 50MHz)
    
    while(1) {
        GPIOC->ODR ^= (1U << 13);
        delay(500000);
    }
    
    return 0;
}