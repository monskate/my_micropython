extern "C"
{
#include "esp_config.h"
#include "esp_code_scanner.h"
#include "esp_log.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "py/mphal.h"
#include "esp_camera.h"
#include "fb_gfx.h"
#include "img_converters.h" // 包含图像转换的库
#include "extmod/modmachine.h"
}
#include "dl_image.hpp"
#include "who_ai_utils.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <list>
#include <stdlib.h>
#include <string.h>


static TaskHandle_t qr_task_handle = NULL;
static char *last_qr_result = NULL; 

void qr_recognition_task(void *arg)
{
    while (true)
    {
        /* 获取摄像头图像 */
        camera_fb_t *camera_frame = esp_camera_fb_get();
        if (camera_frame != NULL)
        {
            /* 二维码识别 */
            esp_image_scanner_t *esp_scn = esp_code_scanner_create();
            esp_code_scanner_config_t config = {ESP_CODE_SCANNER_MODE_FAST, ESP_CODE_SCANNER_IMAGE_RGB565, camera_frame->width, camera_frame->height};
            esp_code_scanner_set_config(esp_scn, config);
            int decoded_num = esp_code_scanner_scan_image(esp_scn, camera_frame->buf);

            char *new_qr_result = NULL;
            if (decoded_num)
            {
                esp_code_scanner_symbol_t result = esp_code_scanner_result(esp_scn);
                new_qr_result = strdup(result.data); 
            }

            esp_code_scanner_destroy(esp_scn);
            
            // 更新全局变量
            if ((last_qr_result == NULL && new_qr_result != NULL) ||
                (last_qr_result != NULL && new_qr_result == NULL) ||
                (last_qr_result != NULL && new_qr_result != NULL && strcmp(last_qr_result, new_qr_result) != 0))
            {
                if (last_qr_result != NULL)
                {
                    free(last_qr_result);
                }
                last_qr_result = new_qr_result;
            }
            else
            {
                if (new_qr_result != NULL)
                {
                    free(new_qr_result);
                }
            }
            
            esp_camera_fb_return(camera_frame);
        }
    }
}

mp_obj_t esp_qr_detection_ai_start(void)
{
    if (qr_task_handle == NULL)
    {
        xTaskCreate(qr_recognition_task, "qr_recognition_task", 4 * 1024, NULL, 5, &qr_task_handle);
    }
    return mp_const_none;
}

mp_obj_t esp_qr_discern(void)
{
    if (last_qr_result != NULL)
    {
        return mp_obj_new_str(last_qr_result, strlen(last_qr_result));
    }
    return mp_const_none;
}

mp_obj_t esp_qr_detection_ai_deinit(void)
{
    if (qr_task_handle != NULL)
    {
        vTaskDelete(qr_task_handle);
        qr_task_handle = NULL;
    }

    if (last_qr_result != NULL)
    {
        free(last_qr_result);
        last_qr_result = NULL;
    }

    return mp_const_none;
}
