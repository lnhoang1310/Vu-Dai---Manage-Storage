#ifndef RFID_H
#define RFID_H

#include <stdint.h>
#include <driver/uart.h>

typedef enum{
    RFID_OK,
    RFID_ERROR,
}RFID_Status;

typedef enum{
    AUTO_READ,
    SINGLE_READ,
    NO_READ
}RFID_Mode;

void rfid_init(uart_port_t uart_num, uint16_t tx_pin, uint16_t rx_pin);
RFID_Status rfid_read_card_auto(char *card_id, uint8_t* id_len);
RFID_Status rfid_read_card_single(char *card_id, uint8_t* id_len);
#endif
