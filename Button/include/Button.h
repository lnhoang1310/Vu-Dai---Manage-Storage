#ifndef BUTTON_H
#define BUTTON_H

#include "driver/gpio.h"
#include <stdint.h>

// Thời gian tính theo ms
#define DEBOUNCE_TIME     20
#define HOLD_TIME         1000
#define REPEAT_INTERVAL   50
#define BUTTON_NUM    4

// Khai báo GPIO nút bấm
#define BUTTON_OK     GPIO_NUM_0
#define BUTTON_UP     GPIO_NUM_2
#define BUTTON_DOWN  GPIO_NUM_4
#define BUTTON_SWAP_MODE   GPIO_NUM_5


void Button_Init(void);
gpio_num_t Button_Pressing(void);

#endif
