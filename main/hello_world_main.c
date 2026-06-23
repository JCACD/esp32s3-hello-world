#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_vfs_dev.h" /* 必要：提供 USB Serial JTAG VFS 註冊 API */

static const char *TAG = "usb_anim";

void app_main(void)
{
    const char *bars[] = {
        "▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"
    };
    int level[16];
    uint8_t ch;

    /* 若在 menuconfig 已啟用 USB Serial JTAG，註冊 VFS 裝置
       注意：請在 sdkconfig 或 menuconfig 中啟用 CONFIG_USB_SERIAL_JTAG_ENABLED */
#ifdef CONFIG_USB_SERIAL_JTAG_ENABLED
    /* 這個呼叫在大多數 ESP-IDF 版本中存在；若你的 IDF 版本 API 名稱不同，
       可改用相對應的 API（例如 esp_vfs_usb_serial_jtag_use_driver） */
    esp_vfs_dev_usb_serial_jtag_register();
#else
    printf("Warning: CONFIG_USB_SERIAL_JTAG_ENABLED not set. Enable USB Serial JTAG in menuconfig.\n");
#endif

    /* 把 stdin 設為 non-blocking，使用 read() 非阻塞讀取 */
    int fd = fileno(stdin);
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        ESP_LOGW(TAG, "fcntl F_GETFL failed: %s", strerror(errno));
    } else {
        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            ESP_LOGW(TAG, "fcntl F_SETFL failed: %s", strerror(errno));
        }
    }

    printf("Starting animation over USB... Press [ESC] key to stop.\n");
    vTaskDelay(pdMS_TO_TICKS(500));

    while (1) {
        /* 非阻塞讀 stdin */
        ssize_t r = read(fd, &ch, 1);
        if (r > 0) {
            if (ch == 27) { /* ESC */
                printf("\n\033[2K\rESC pressed. Animation stopped.\n");
                break;
            }
        } else if (r < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            ESP_LOGW(TAG, "read error: %s", strerror(errno));
        }

        /* 產生隨機高度 */
        for (int i = 0; i < 16; i++) {
            level[i] = esp_random() % 8;
        }

        /* 顯示（使用 ANSI 控制碼覆蓋同一行） */
        printf("\033[2K\r");
        for (int i = 0; i < 16; i++) {
            printf("%s ", bars[level[i]]);
        }
        fflush(stdout);

        vTaskDelay(pdMS_TO_TICKS(120));
    }

    printf("Program finished.\n");
}
