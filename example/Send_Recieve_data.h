
// ================= HEADER & CONFIG =================
#include <Arduino.h> // Đảm bảo các kiểu dữ liệu Arduino

// ================= SLAVE ID & FRAME CONFIG =================
uint16_t slave_IP = 0;
byte Frame_header = 36;
byte Modular_ID = slave_IP / 16;
byte Sending_order = ((Modular_ID == 2) ? 11 : (Modular_ID == 3) ? 12 : (Modular_ID == 4) ? 13 : (Modular_ID == 5) ? 14 : (Modular_ID == 6) ? 15 : 0);
byte Frame_footer = 35;
byte Receive_check = 37;

// ================= COMMUNICATION STATE =================
int counter1 = 0;
int counter2 = 0;
int send_status = 3;
int buffer_RX_index = 0;
int Header_index;
int Footer_index;
uint8_t Frame_position = 0;

// ================= CONTROL DATA FOR WHEEL MODULES =================
float FL_Angle = 0;
float FL_Speed = 0;
float FR_Angle = 0;
float FR_Speed = 0;
float RL_Angle = 0;
float RL_Speed = 0;
float RR_Angle = 0;
float RR_Speed = 0;

// ================= FLAGS =================
bool counter1_flag = 0;
bool counter2_flag = 0;
bool done_copy_flag = 0;
bool can_copy_flag = 0;
bool header_flag = 0;
bool footer_flag = 0;
bool send_flag = 0;
bool send_prepare_flag = 0;
bool recieve_flag = 0;
bool FL_flag = 0;
bool FR_flag = 0;
bool RL_flag = 0;
bool RR_flag = 0;

// ================= SERIAL BUFFERS =================
uint16_t Serial_available = 0;
byte buffer_read[24];
byte buffer_receive[1];
byte buffer_RX[80];

// ================= DATA STRUCTS =================
struct Data_send {
  byte Header[4] = {Frame_header, 40, Sending_order, 0};
  int16_t confirm_angle;
  int16_t confirm_speed;
  int16_t measure_speed;
  int16_t angle_PWM_signal;
  int16_t speed_PWM_signal;
  int16_t reserved_1 = 0;
  int16_t reserved_2 = 0;
  int16_t reserved_3 = 0;
  byte Footer[4] = {Receive_check, Sending_order, Modular_ID, Frame_footer};
} send_data;

struct Data_recieved {
  byte Header[4] = {0, 0, 0, 0};
  int16_t FL_Angle = 0;
  int16_t FL_Speed = 0;
  int16_t FR_Angle = 0;
  int16_t FR_Speed = 0;
  int16_t RL_Angle = 0;
  int16_t RL_Speed = 0;
  int16_t RR_Angle = 0;
  int16_t RR_Speed = 0;
  byte Footer[4] = {0, 0, 0, 0};
} recieve_data;

// ================= UTILITY FUNCTIONS =================
// Copy a valid frame from RX buffer to read buffer
void copy_array(int Header_index, int Footer_index) {
  int bufer_read_index = 0;
  for (int i = Header_index; i <= Footer_index; i++) {
    buffer_read[bufer_read_index] = buffer_RX[i];
    bufer_read_index++;
  }
}

// State machine for sending/receiving RS485 data
void Sending_data() {
  switch (send_status) {
    case 0: // Send
      if (counter1-- == 0) {
        //digitalWrite(4, HIGH);
        Serial1.write((byte*)&send_data, 24);
        send_status = 1;
      }
      break;
    case 1: // Check empty buffer write
      if (Serial1.availableForWrite() == 63) {
        send_status = 2;
        counter2 = 0;
      }
      break;
    case 2: // Waiting
      if (counter2-- == 0) {
        digitalWrite(4, LOW);
        send_status = 3;
      }
      break;
    case 3: // Receive
      Serial_available = Serial1.available();
      if (Serial_available > 23) {
        Serial1.readBytes(buffer_RX, Serial_available);
        send_status = 4;
      }
      break;
    case 4: // Check frame
      for (int i = 0; i < (Serial_available - 22); i++) {
        if (buffer_RX[i] == 36) {
          Header_index = i;
          break;
        }
      }
      if (buffer_RX[Header_index + 23] == 35) {
        copy_array(Header_index, Header_index + 23);
        if (buffer_read[22] == 1) {
          memcpy(&recieve_data, buffer_read, 24);
          switch (Modular_ID) {
            case 2: FL_flag = 1; break;
            case 3: FR_flag = 1; break;
            case 4: RL_flag = 1; break;
            case 5: RR_flag = 1; break;
          }
        }
        if (buffer_read[2] == (Sending_order - 1)) {
          send_prepare_flag = 1;
          send_status = 5;
        } else {
          send_status = 3;
        }
      } else {
        send_status = 3;
      }
      break;
    case 5: // Refresh data
      if (send_flag == 1) {
        send_flag = 0;
        send_status = 0;
        counter1 = 0;
      }
      break;
  }
}
