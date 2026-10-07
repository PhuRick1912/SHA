#include "fsm.h"
#include "timer.h"
#include "sensor.h"
#include "relay.h"
// #include "relay.h" // Chứa hàm Turn_On_Devices(), Turn_Off_Devices()

#define SEQUENCE_TIMEOUT_MS  10000
#define COUNTDOWN_5MIN_MS    300000

typedef enum {
    STATE_IDLE,
    STATE_DOOR_OPENED_IN,
    STATE_WAIT_IR_IN,
    STATE_IR_DETECTED_OUT,
    STATE_DOOR_OPENED_OUT,
    STATE_COUNTDOWN_5MIN
} SystemState_t;

static SystemState_t current_state;
static uint32_t state_timer;

void Home_Automation_FSM_Init(void) {
    current_state = STATE_IDLE;
    state_timer = 0;
}

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
                        // Ngoại lệ 2: Cửa không đóng lại -> FSM sẽ kẹt ở đây mãi mãi (Đúng yêu cầu)
                        if (!door_open) { // Cửa đóng
                            current_state = STATE_WAIT_IR_IN;
                            state_timer = current_tick;
                        }
                        break;

                    case STATE_WAIT_IR_IN:
                        if (ir_triggered) {
                        	relay_set_state(RELAY_ON); // XÁC NHẬN VÀO NHÀ
                            current_state = STATE_IDLE;
                        }
                        else if ((current_tick - state_timer) > SEQUENCE_TIMEOUT_MS) {
                            current_state = STATE_IDLE; // Reset nếu quá 10s không thấy ai
                        }
                        break;

                    // ---------------- KỊCH BẢN RA KHỎI NHÀ ---------------- //
                    case STATE_IR_DETECTED_OUT:
                        if (door_open) {
                            current_state = STATE_DOOR_OPENED_OUT;
                        }
                        else if ((current_tick - state_timer) > SEQUENCE_TIMEOUT_MS) {
                            current_state = STATE_IDLE; // Người đi ngang IR nhưng không mở cửa -> Reset
                        }
                        break;

                    case STATE_DOOR_OPENED_OUT:
                        // Ngoại lệ 2: Cửa không đóng lại -> FSM treo tại đây
                        if (!door_open) { // Cửa đóng
                            current_state = STATE_COUNTDOWN_5MIN;
                            state_timer = current_tick; // Bắt đầu đếm 5 phút
                        }
                        break;

                    case STATE_COUNTDOWN_5MIN:
                        // Ngoại lệ 1: Chủ nhà quay lại (IR có tín hiệu)
                        if (ir_triggered) {
                            current_state = STATE_IDLE; // Lập tức hủy lệnh tắt
                        }
                        // Hết 5 phút
                        else if ((current_tick - state_timer) > COUNTDOWN_5MIN_MS) {
                        	relay_set_state(RELAY_OFF); // XÁC NHẬN RỜI ĐI
                            current_state = STATE_IDLE;
                        }
                        break;
                }

    }

