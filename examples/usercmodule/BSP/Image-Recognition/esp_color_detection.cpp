extern "C"
{
#include "esp_config.h"
#include "esp_log.h"
#include "esp_code_scanner.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "py/mphal.h"
#include "esp_camera.h"
#include "fb_gfx.h"
#include "img_converters.h" 
#include "extmod/modmachine.h"
}
#include "dl_image.hpp"
#include "who_ai_utils.hpp"
#include "color_detector.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <list>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <vector>

using namespace std;
using namespace dl;

typedef struct
{
    int x;             // 识别到的颜色x轴
    int y;             // 识别到的颜色y轴
} color_result_t;

static TaskHandle_t color_task_handle = NULL;
static bool draw_box = true;
static char selected_color[10] = "";
static SemaphoreHandle_t color_mutex = NULL;
static volatile bool color_selected = false;
static volatile color_result_t color_result = {0, 0};

typedef struct {
    int color_thresh[6];
    int area_thresh;
    char name[10];
} color_set;

static const color_set yuzhi_color_info[10] = {
    {{156, 10, 70, 255, 90, 255}, 64, "red"},
    {{11, 22, 70, 255, 90, 255}, 64, "orange"},
    {{23, 33, 70, 255, 90, 255}, 64, "yellow"},
    {{34, 75, 70, 255, 90, 255}, 64, "green"},
    {{76, 96, 70, 255, 90, 255}, 64, "cyan"},
    {{97, 124, 70, 255, 90, 255}, 64, "blue"},
    {{125, 155, 70, 255, 90, 255}, 64, "purple"},
    {{0, 180, 0, 40, 220, 255}, 64, "white"},
    {{0, 180, 0, 50, 50, 219}, 64, "gray"},
    {{0, 180, 0, 255, 0, 45}, 64, "black"}
};

static void color_recognition_task(void *arg) {
    ColorDetector detector;
    detector.set_detection_shape({80, 80, 1});
    char current_color[10] = "";
    bool color_registered = false;

    while (1) {
        xSemaphoreTake(color_mutex, portMAX_DELAY);
        bool enable_detection = color_selected;
        xSemaphoreGive(color_mutex);

        if (!enable_detection) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        xSemaphoreTake(color_mutex, portMAX_DELAY);
        if (strcmp(current_color, selected_color) != 0) {
            detector.clear_color();
            for (int i = 0; i < 10; ++i) {
                if (strcmp(yuzhi_color_info[i].name, selected_color) == 0) {
                    std::vector<uint8_t> thresh(
                        yuzhi_color_info[i].color_thresh,
                        yuzhi_color_info[i].color_thresh + 6);
                    detector.register_color(thresh, 
                        yuzhi_color_info[i].area_thresh,
                        yuzhi_color_info[i].name);
                    strncpy(current_color, selected_color, 10);
                    color_registered = true;
                    break;
                }
            }
            if (!color_registered) {
                ESP_LOGE("COLOR", "Invalid color: %s", selected_color);
                color_selected = false;
            }
        }
        xSemaphoreGive(color_mutex);

        camera_fb_t *camera_frame = esp_camera_fb_get();
        if (camera_frame == NULL) {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        std::vector<std::vector<color_detect_result_t>> &results = detector.detect(
            (uint16_t *)camera_frame->buf, {(int)camera_frame->height, (int)camera_frame->width, 3});
        
        color_result.x = 0;
        color_result.y = 0;
        if (!results.empty()) {
            for (int i = 0; i < results.size(); ++i) {
                for (int j = 0; j < results[i].size(); ++j) {
                    int x = results[i][j].center[0];
                    int y = results[i][j].center[1];
                    color_result.x = x;
                    color_result.y = y;
                }
            }
        }
        esp_camera_fb_return(camera_frame);
        camera_frame = NULL;
    }
}

mp_obj_t esp_color_init(void) {
    if (color_mutex == NULL) {
        color_mutex = xSemaphoreCreateMutex();
    }
    if (color_task_handle == NULL) {
        xTaskCreate(color_recognition_task, "color_recognition_task", 6 * 1024, NULL, 2, &color_task_handle);
        xSemaphoreTake(color_mutex, portMAX_DELAY);
        color_selected = false;
        selected_color[0] = '\0';
        xSemaphoreGive(color_mutex);
    }
    return mp_const_none;
}

mp_obj_t esp_set_selected_color(mp_obj_t color_name) {
    const char *name = mp_obj_str_get_str(color_name);
    xSemaphoreTake(color_mutex, portMAX_DELAY);
    strlcpy(selected_color, name, 10);
    color_selected = (strlen(selected_color) > 0);
    xSemaphoreGive(color_mutex);
    return mp_const_none;
}

mp_obj_t esp_color_discern(void) {
    // 创建信息的Python对象
    mp_obj_t x = mp_obj_new_int(color_result.x);
    mp_obj_t y = mp_obj_new_int(color_result.y);

    // 返回颜色位置
    mp_obj_t elements[] = {x, y};
    return mp_obj_new_list(2, elements);}

mp_obj_t esp_color_deinit(void) {
    if (color_task_handle != NULL) {
        vTaskDelete(color_task_handle);
        color_task_handle = NULL;
    }
    if (color_mutex != NULL) {
        xSemaphoreTake(color_mutex, portMAX_DELAY);
        color_selected = false;
        selected_color[0] = '\0';
        xSemaphoreGive(color_mutex);
        vSemaphoreDelete(color_mutex);
        color_mutex = NULL;
    }
    return mp_const_none;
}
