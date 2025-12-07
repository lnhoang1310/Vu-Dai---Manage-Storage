#include "LCD.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"

static TickType_t ms(uint32_t milliseconds) {
    return milliseconds / portTICK_PERIOD_MS;
}

static void send_command(lcd_typedef *lcd, uint8_t cmd){
    uint8_t data[4];
    data[0] = (cmd & 0xF0) | 0x0C;
    data[1] = (cmd & 0xF0) | 0x08;
    data[2] = ((cmd << 4) & 0xF0) | 0x0C;
    data[3] = ((cmd << 4) & 0xF0) | 0x08;
    i2c_master_write_to_device(lcd->i2c_port, lcd->i2c_addr, data, 4, ms(1000));
}

static void send_data(lcd_typedef* lcd, uint8_t data){
    uint8_t packet[4];
    packet[0] = (data & 0xF0) | 0x0D;
    packet[1] = (data & 0xF0) | 0x09;
    packet[2] = ((data << 4) & 0xF0) | 0x0D;
    packet[3] = ((data << 4) & 0xF0) | 0x09;
    i2c_master_write_to_device(lcd->i2c_port, lcd->i2c_addr, packet, 4, 1000 / portTICK_PERIOD_MS);
}

void lcd_init(lcd_typedef* lcd, i2c_port_t i2c_port, uint8_t i2c_addr, uint8_t scl_pin, uint8_t sda_pin){
    lcd->i2c_port = i2c_port;
    lcd->i2c_addr = i2c_addr;
    lcd->scl_pin = scl_pin;
    lcd->sda_pin = sda_pin;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = lcd->sda_pin,
        .scl_io_num = lcd->scl_pin,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000
    };
    i2c_param_config(lcd->i2c_port, &conf);
    i2c_driver_install(lcd->i2c_port, conf.mode, 0, 0, 0);

    vTaskDelay(ms(50));
    send_command(lcd, 0x30);
    vTaskDelay(ms(5));
    send_command(lcd, 0x30);
    vTaskDelay(ms(1));
    send_command(lcd, 0x30);
    vTaskDelay(ms(10));
    send_command(lcd, 0x20);
    vTaskDelay(ms(10));
    send_command(lcd, 0x28);
    vTaskDelay(ms(1));
    send_command(lcd, 0x08);
    vTaskDelay(ms(1));
    send_command(lcd, 0x01);
    vTaskDelay(ms(2));
    send_command(lcd, 0x06);
    vTaskDelay(ms(1));
    send_command(lcd, 0x0C);
    vTaskDelay(ms(10));
}

void lcd_sendchar(lcd_typedef* lcd, char data){
    send_data(lcd, (uint8_t)data);
}

void lcd_sendstring(lcd_typedef* lcd, const char* str){
    while (*str){
        lcd_sendchar(lcd, *str++);
    }
}

void lcd_sendnumber(lcd_typedef* lcd, float number){
    char buffer[16];
    sprintf(buffer, "%.1f", number);
    lcd_sendstring(lcd, buffer);
}

void lcd_clear(lcd_typedef* lcd){
    send_command(lcd, 0x01);
    vTaskDelay(ms(2));
}

void lcd_setcursor(lcd_typedef* lcd, uint8_t row, uint8_t col){
    uint8_t address_offset[] = {0x00, 0x40, 0x14, 0x54};
    if (row >= 4) row = 3;
    send_command(lcd, 0x80 | (col + address_offset[row]));
}

void lcd_createchar(lcd_typedef* lcd, uint8_t location, uint8_t charmap[]){
    location &= 0x7;
    send_command(lcd, 0x40 | (location << 3));
    for (int i = 0; i < 8; i++){
        send_data(lcd, charmap[i]);
    }
}