

// ================= TIMER INITIALIZATION FUNCTIONS =================
#include <Arduino.h> // Đảm bảo macro cli(), sei() hoạt động đúng

// Timer0: dùng cho truyền thông định kỳ (RS485)
void Timer0() {
  cli(); // Disable interrupts
  TCCR0A = 0;
  TCCR0B = 0;
  TCNT0 = 0;
  TIMSK0 = 0;

  // TCCR0B |= (1 << WGM02) | (1 << CS01 ) | (1 << CS00);   // PSC 16
  TCCR0B |= (1 << WGM02) | (1 << CS01); // PSC 8
  TCCR0A |= (1 << WGM01) | (1 << WGM00) | (1 << COM0A1);
  OCR0A = 200;
  TIMSK0 |= (1 << OCIE0A);
  sei(); // Enable interrupts
}

// Timer1: dùng cho điều khiển động cơ (PWM, capture encoder)
void Timer1() {
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TIMSK1 = 0;
  OCR1A = 0;
  OCR1B = 0;
  TCNT1 = 0; // Reset counter
  ICR1 = 799; // Set TOP for 20kHz
  // Mode 14: Fast PWM with ICR1 as TOP, non-inverting output
  TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
  // Prescaler = 1, enable Input Capture (for encoder if needed)
  TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << ICES1) | (1 << CS10);
  TIMSK1 = (1 << ICIE1) | (1 << TOIE1);
  sei();
}

// Timer3: dùng cho điều khiển servo và chu kỳ điều khiển 50Hz
void Timer3() {
  cli();
  TCCR3A = 0;
  TCCR3B = 0;
  TIMSK3 = 0;
  TCCR3B |= (1 << WGM33) | (1 << WGM32) | (1 << CS31);
  TCCR3A |= (1 << WGM31) | (1 << COM3A1) | (1 << COM3B1);
  ICR3 = 40000;
  OCR3A = 0;
  OCR3B = 0;
  TIMSK3 = (1 << TOIE3);
  sei();
}
