#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_random.h"
#include "driver/uart.h"

static const char *TAG = "anim";

void app_main(void)
{
    const char *bars[] = {
        "▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"
    };

    int level[16];
    uint8_t ch;

    // 初始化 UART0（確保有安裝 driver 才能用 uart_read_bytes 非阻塞讀取）
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB
    };
    uart_param_config(UART_NUM_0, &uart_config);
    // 不改變預設 TX/RX 腳位（通常 USB-to-UART 已接好），所以使用 UART_PIN_NO_CHANGE
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    // 安裝 driver，rx buffer 設為 2048 bytes，tx buffer 設為 0（不需要）
    const int rx_buffer_size = 2048;
    uart_driver_install(UART_NUM_0, rx_buffer_size, 0, 0, NULL, 0);

    // 提示使用者
    printf("Starting animation... Press [ESC] key to stop.\n");
    vTaskDelay(pdMS_TO_TICKS(500));

    // 清空 UART 緩衝區，確保不會讀到開機時的殘留雜訊
    uart_flush(UART_NUM_0);

    while (1) {
        // 非阻塞檢查是否按下 ESC 鍵
        int len = uart_read_bytes(UART_NUM_0, &ch, 1, 0); // timeout = 0 => non-blocking
        if (len > 0) {
            if (ch == 27) { // ESC
                printf("\n\033[2K\rESC pressed. Animation stopped.\n");
                break;
            }
        }

        // 產生隨機高度
        for (int i = 0; i < 16; i++) {
            level[i] = esp_random() % 8;
        }

        // 顯示
        printf("\033[2K\r");
        for (int i = 0; i < 16; i++) {
            printf("%s ", bars[level[i]]);
        }
        fflush(stdout);

        vTaskDelay(pdMS_TO_TICKS(120));
    }

    // 卸載 UART driver（可選）
    uart_driver_delete(UART_NUM_0);

    printf("Program finished.\n");
}
