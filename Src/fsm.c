#include "fsm.h"
#include "timer.h"
#include "sensor.h"
#include "relay.h"
#include "esp8266_mqtt.h"

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


        case STATE_DOOR_OPENED_IN:
            if (!door_open) { // Cửa đóng
                current_state = STATE_WAIT_IR_IN;
                state_timer = current_tick;
            }
            break;

        case STATE_WAIT_IR_IN:
            if (ir_triggered) {
                relay_set_state(RELAY_ON);

                // Cập nhật số người vào nhà
                people_count++;
                flag_count_changed = true;

                current_state = STATE_IDLE;
            }
            else if ((current_tick - state_timer) > SEQUENCE_TIMEOUT_MS) {
                current_state = STATE_IDLE;
            }
            break;
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
                // Cập nhật số người ra khỏi nhà
                if (people_count > 0) {
                    people_count--;
                }
                flag_count_changed = true;

                //Quyết định có đếm lùi tắt điện không
                if (people_count == 0) {
                    current_state = STATE_COUNTDOWN_5MIN;
                    state_timer = current_tick;
                } else {

                    current_state = STATE_IDLE;
                }
            }
            break;

        case STATE_COUNTDOWN_5MIN:
            if (ir_triggered) { // Chủ nhà quay lại
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
