//#include "fsm.h"
//#include "timer.h"
//#include "sensor.h"
//#include "relay.h"
//// #include "relay.h" // Chứa hàm Turn_On_Devices(), Turn_Off_Devices()
//
//#define SEQUENCE_TIMEOUT_MS  10000
//#define COUNTDOWN_5MIN_MS    300000
//
//
//static SystemState_t current_state;
//static uint32_t state_timer;
//
//void Home_Automation_FSM_Init(void) {
//    current_state = STATE_IDLE;
//    state_timer = 0;
//}
//
//void Home_Automation_FSM_Update(void) {
//    bool door_open = Is_Door_Open();
//    bool ir_triggered = Is_IR_Triggered();
//    uint32_t current_tick = Get_System_Tick();
//
//    switch (current_state) {
//
//        case STATE_IDLE:
//            if (door_open) {
//                current_state = STATE_DOOR_OPENED_IN;
//            }
//            else if (ir_triggered) {
//
//                            current_state = STATE_IR_DETECTED_OUT;
//                            state_timer = current_tick;
//                        }
//            break;
//            // ---------------- KỊCH BẢN VÀO NHÀ ---------------- //
//                    case STATE_DOOR_OPENED_IN:
//                        // Ngoại lệ 2: Cửa không đóng lại -> FSM sẽ kẹt ở đây mãi mãi (Đúng yêu cầu)
//                        if (!door_open) { // Cửa đóng
//                            current_state = STATE_WAIT_IR_IN;
//                            state_timer = current_tick;
//                        }
//                        break;
//
//                    case STATE_WAIT_IR_IN:
//                        if (ir_triggered) {
//                        	relay_set_state(RELAY_ON); // XÁC NHẬN VÀO NHÀ
//                            current_state = STATE_IDLE;
//                        }
//                        else if ((current_tick - state_timer) > SEQUENCE_TIMEOUT_MS) {
//                            current_state = STATE_IDLE; // Reset nếu quá 10s không thấy ai
//                        }
//                        break;
//
//                    // ---------------- KỊCH BẢN RA KHỎI NHÀ ---------------- //
//                    case STATE_IR_DETECTED_OUT:
//                        if (door_open) {
//                            current_state = STATE_DOOR_OPENED_OUT;
//                        }
//                        else if ((current_tick - state_timer) > SEQUENCE_TIMEOUT_MS) {
//                            current_state = STATE_IDLE; // Người đi ngang IR nhưng không mở cửa -> Reset
//                        }
//                        break;
//
//                    case STATE_DOOR_OPENED_OUT:
//                        // Ngoại lệ 2: Cửa không đóng lại -> FSM treo tại đây
//                        if (!door_open) { // Cửa đóng
//                            current_state = STATE_COUNTDOWN_5MIN;
//                            state_timer = current_tick; // Bắt đầu đếm 5 phút
//                        }
//                        break;
//
//                    case STATE_COUNTDOWN_5MIN:
//                        // Ngoại lệ 1: Chủ nhà quay lại (IR có tín hiệu)
//                        if (ir_triggered) {
//                            current_state = STATE_IDLE; // Lập tức hủy lệnh tắt
//                        }
//                        // Hết 5 phút
//                        else if ((current_tick - state_timer) > COUNTDOWN_5MIN_MS) {
//                        	relay_set_state(RELAY_OFF); // XÁC NHẬN RỜI ĐI
//                            current_state = STATE_IDLE;
//                        }
//                        break;
//                }
//
//    }
//
#include "fsm.h"
#include "timer.h"
#include "sensor.h"
#include "relay.h"
#include "esp8266_mqtt.h" // Thêm thư viện này để truy xuất people_count

#define SEQUENCE_TIMEOUT_MS  10000
#define COUNTDOWN_5MIN_MS    300000
static SystemState_t current_state = STATE_IDLE;
static uint32_t state_timer = 0;

void Home_Automation_FSM_Init(void) {
    current_state = STATE_IDLE;
    state_timer = 0;
}
// ... (Khai báo biến state như cũ) ...

void Home_Automation_FSM_Update(void) {
    bool door_open = Is_Door_Open();
    bool ir_triggered = Is_IR_Triggered();
    uint32_t current_tick = Get_System_Tick();

    switch (current_state) {
        case STATE_IDLE:
            if (door_open) {
                current_state = STATE_DOOR_OPENED_IN;
            }
            else if (ir_triggered) {
                current_state = STATE_IR_DETECTED_OUT;
                state_timer = current_tick;
            }
            break;

        // ---------------- KỊCH BẢN VÀO NHÀ ---------------- //
        case STATE_DOOR_OPENED_IN:
            if (!door_open) { // Cửa đóng
                current_state = STATE_WAIT_IR_IN;
                state_timer = current_tick;
            }
            break;

        case STATE_WAIT_IR_IN:
            if (ir_triggered) {
                relay_set_state(RELAY_ON);

                // [NEW] Cập nhật số người vào nhà
                people_count++;
                flag_count_changed = true;

                current_state = STATE_IDLE;
            }
            else if ((current_tick - state_timer) > SEQUENCE_TIMEOUT_MS) {
                current_state = STATE_IDLE;
            }
            break;

        // ---------------- KỊCH BẢN RA KHỎI NHÀ ---------------- //
        case STATE_IR_DETECTED_OUT:
            if (door_open) {
                current_state = STATE_DOOR_OPENED_OUT;
            }
            else if ((current_tick - state_timer) > SEQUENCE_TIMEOUT_MS) {
                current_state = STATE_IDLE;
            }
            break;

        case STATE_DOOR_OPENED_OUT:
            if (!door_open) { // Cửa đóng -> Xác nhận có người vừa rời đi
                // [NEW] Cập nhật số người ra khỏi nhà
                if (people_count > 0) {
                    people_count--;
                }
                flag_count_changed = true;

                // [NEW] Quyết định có đếm lùi tắt điện không
                if (people_count == 0) {
                    current_state = STATE_COUNTDOWN_5MIN;
                    state_timer = current_tick;
                } else {
                    // Nếu vẫn còn người trong phòng, hủy tắt điện, về thẳng IDLE
                    current_state = STATE_IDLE;
                }
            }
            break;

        case STATE_COUNTDOWN_5MIN:
            if (ir_triggered) { // Chủ nhà quay lại (hoặc người khác di chuyển)
                // Phục hồi số lượng lên 1
                people_count = 1;
                flag_count_changed = true;
                current_state = STATE_IDLE;
            }
            else if ((current_tick - state_timer) > COUNTDOWN_5MIN_MS) {
                relay_set_state(RELAY_OFF);
                current_state = STATE_IDLE;
            }
            break;
    }
}
