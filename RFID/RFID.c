#include "RFID.h"
#include "RFID_Commands.h"

#define UART_BUF_SIZE 128
static uart_port_t rfid_uart_num;
char uart_buffer[UART_BUF_SIZE];
static uint8_t buffer_index = 0;
static bool flag_receive_complete = false;
static RFID_Mode current_mode = NO_READ;

void uart_receive_task(void)
{
    while (1)
    {
        int len = uart_read_bytes(rfid_uart_num, uart_buffer, sizeof(uart_buffer), 100);
        if (len > 0)
        {
            if (uart_buffer[len - 1] == '\n')
            {
                flag_receive_complete = true;
                uart_buffer[len] = '\0';
            }
        }
    }
}

static RFID_Status send_command(const char *command)
{
    int len = uart_write_bytes(rfid_uart_num, command, strlen(command));
    int len = uart_write_bytes(rfid_uart_num, LMRF3060_END_COMMAND, strlen(LMRF3060_END_COMMAND));
    if (len < 0)
    {
        return RFID_ERROR;
    }
    return RFID_OK;
}

static RFID_Status read_command(const char *expected_response)
{
    while (!flag_receive_complete)
    {
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    flag_receive_complete = false;
    if (strstr((char *)uart_buffer, expected_response) != NULL)
    {
        return RFID_OK;
    }
    return RFID_ERROR;
}

void rfid_init(uart_port_t uart_num, uint16_t tx_pin, uint16_t rx_pin)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_param_config(uart_num, &uart_config);
    uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(uart_num, 1024, 0, 0, NULL, 0);
    rfid_uart_num = uart_num;
}

RFID_Status rfid_read_card_auto(char *card_id, uint8_t *id_len)
{
}
