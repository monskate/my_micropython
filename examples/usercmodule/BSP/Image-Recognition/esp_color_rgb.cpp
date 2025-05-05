extern "C"
{
#include "esp_config.h"
#include "esp_log.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "py/mphal.h"
#include "esp_camera.h"
#include "extmod/modmachine.h"
}

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <list>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <vector>


typedef struct {
    int r;
    int g;
    int b;
} rgb_result_t;

// 识别区域结构体
typedef struct {
    int x;
    int y;
    int w;
    int h;
} region_t;

static TaskHandle_t color_task_handle = NULL;
static SemaphoreHandle_t color_mutex = NULL;

static rgb_result_t color_result = {0, 0, 0};
static region_t detect_region = {0, 0, 40, 40};

static void color_recognition_task(void *arg) {
    while (1) {
        camera_fb_t *camera_frame = esp_camera_fb_get();
        if (camera_frame == NULL) {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        int img_width = camera_frame->width;
        int img_height = camera_frame->height;

        region_t region;
        xSemaphoreTake(color_mutex, portMAX_DELAY);
        region = detect_region;
        xSemaphoreGive(color_mutex);

        int start_x = region.x;
        int start_y = region.y;
        int region_w = region.w;
        int region_h = region.h;
        // 限制区域合法
        if (start_x < 0) start_x = 0;
        if (start_y < 0) start_y = 0;
        if (start_x + region_w > img_width) region_w = img_width - start_x;
        if (start_y + region_h > img_height) region_h = img_height - start_y;

        uint16_t *pixels = (uint16_t *)camera_frame->buf;

        uint8_t max_r = 0, max_g = 0, max_b = 0;

        for (int y = start_y; y < start_y + region_h; ++y) {
            for (int x = start_x; x < start_x + region_w; ++x) {
                int index = y * img_width + x;
                uint16_t pixel = pixels[index];

                uint8_t r = ((pixel >> 11) & 0x1F) << 3;
                uint8_t g = ((pixel >> 5) & 0x3F) << 2;
                uint8_t b = (pixel & 0x1F) << 3;

                if (r > max_r) max_r = r;
                if (g > max_g) max_g = g;
                if (b > max_b) max_b = b;
            }
        }

        rgb_result_t temp_result = {max_r, max_g, max_b};

        xSemaphoreTake(color_mutex, portMAX_DELAY);
        color_result = temp_result;
        xSemaphoreGive(color_mutex);

        esp_camera_fb_return(camera_frame);
        vTaskDelay(pdMS_TO_TICKS(100));
            
    }
}

mp_obj_t esp_color_rgb_init(void) {
    if (color_mutex == NULL) {
        color_mutex = xSemaphoreCreateMutex();
    }
    if (color_task_handle == NULL) {
        xTaskCreate(color_recognition_task, "color_recognition_task", 6 * 1024, NULL, 2, &color_task_handle);
    }
    return mp_const_none;
}

mp_obj_t esp_color_rgb(void) {
    mp_obj_t r, g, b;
    xSemaphoreTake(color_mutex, portMAX_DELAY);
    r = mp_obj_new_int(color_result.r);
    g = mp_obj_new_int(color_result.g);
    b = mp_obj_new_int(color_result.b);
    xSemaphoreGive(color_mutex);

    mp_obj_t rgb_list[] = {r, g, b};
    return mp_obj_new_list(3, rgb_list);
}

// 设置区域接口：传入 x, y, w, h
mp_obj_t esp_color_set_region(mp_obj_t x_obj, mp_obj_t y_obj, mp_obj_t w_obj, mp_obj_t h_obj) {
    int x = mp_obj_get_int(x_obj);
    int y = mp_obj_get_int(y_obj);
    int w = mp_obj_get_int(w_obj);
    int h = mp_obj_get_int(h_obj);

    xSemaphoreTake(color_mutex, portMAX_DELAY);
    detect_region.x = x;
    detect_region.y = y;
    detect_region.w = w;
    detect_region.h = h;
    xSemaphoreGive(color_mutex);

    return mp_const_none;
}

mp_obj_t esp_color_rgb_deinit(void) {
    if (color_task_handle != NULL) {
        vTaskDelete(color_task_handle);
        color_task_handle = NULL;
    }
    if (color_mutex != NULL) {
        vSemaphoreDelete(color_mutex);
        color_mutex = NULL;
    }
    return mp_const_none;
}
