#include "button.h"
#include "esp_timer.h"
#include <string.h>

typedef struct
{
    int64_t last_time;  // thời điểm thay đổi trạng thái cuối
    int64_t hold_start; // thời điểm bắt đầu giữ
    uint8_t last_state; // trạng thái trước đó
    uint8_t is_held;    // đang giữ hay không
} ButtonState;

static ButtonState btn_ok = {0, 0, 1, 0};
static ButtonState btn_up = {0, 0, 1, 0};
static ButtonState btn_down = {0, 0, 1, 0};
static ButtonState btn_swap = {0, 0, 1, 0};

// -------------------- INIT GPIO --------------------
void Button_Init(void)
{
    gpio_config_t io = {
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};

    gpio_num_t pins[] = {
        BUTTON_OK,
        BUTTON_UP,
        BUTTON_DOWN,
        BUTTON_SWAP_MODE};

    for (int i = 0; i < BUTTON_NUM; i++)
    {
        io.pin_bit_mask = 1ULL << pins[i];
        gpio_config(&io);
    }
}

// -------------------- BUTTON PROCESS --------------------
gpio_num_t Button_Pressing(void)
{
    struct
    {
        gpio_num_t pin;
        ButtonState *state;
    } buttons[] = {
        {BUTTON_OK, &btn_ok},
        {BUTTON_UP, &btn_up},
        {BUTTON_DOWN, &btn_down},
        {BUTTON_SWAP_MODE, &btn_swap},
    };

    int64_t now = esp_timer_get_time() / 1000;

    for (int i = 0; i < BUTTON_NUM; i++)
    {
        uint8_t state = gpio_get_level(buttons[i].pin);
        ButtonState *btn = buttons[i].state;

        // ---------------- Thay đổi trạng thái ----------------
        if (state != btn->last_state)
        {
            if (now - btn->last_time > DEBOUNCE_TIME)
            {

                btn->last_time = now;
                btn->last_state = state;

                if (state == 0)
                { // nhấn (LOW)
                    btn->hold_start = now;
                    btn->is_held = 0;
                    return buttons[i].pin;
                }
                else
                {
                    btn->is_held = 0;
                }
            }
        }
        // ---------------- Giữ nút ----------------
        else if (state == 0)
        {
            // Nhấn giữ lần đầu
            if (!btn->is_held && (now - btn->hold_start > HOLD_TIME))
            {
                btn->is_held = 1;
                btn->last_time = now;
                return buttons[i].pin;
            }

            // Tự lặp khi giữ
            if (btn->is_held && (now - btn->last_time > REPEAT_INTERVAL))
            {
                btn->last_time = now;
                return buttons[i].pin;
            }
        }
    }

    return GPIO_NUM_NC;
}
