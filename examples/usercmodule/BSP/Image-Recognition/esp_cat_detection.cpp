#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <list>

extern "C"
{
#include "esp_config.h"
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

#include "who_ai_utils.hpp"
#include "dl_image.hpp"
#include "cat_face_detect_mn03.hpp"

// 定义队列的数据结构
static struct {
    int num;
    int x;
    int y;
} cat_result = {0, 0, 0};

static TaskHandle_t cat_task_handle = NULL;
static SemaphoreHandle_t cat_mutex = NULL;

// 猫脸识别任务
void cat_detection_task(void *pvParameters) {
    CatFaceDetectMN03 detector(0.4F, 0.3F, 10, 0.3F);
    while (true) {
        camera_fb_t *frame = esp_camera_fb_get();
        if (frame != NULL) {
            std::list<dl::detect::result_t> &detect_results = detector.infer((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3});
            cat_result.x = 0;
            cat_result.y = 0;
            cat_result.num = detect_results.size();
            if (!detect_results.empty()) {
                auto prediction = detect_results.begin();
                cat_result.x = ((prediction->box[0] + prediction->box[2]) / 2) - 160;
                cat_result.y = 120 - ((prediction->box[1] + prediction->box[3]) / 2);
            }
            
            esp_camera_fb_return(frame);
        }
        vTaskDelay(pdMS_TO_TICKS(50)); // 降低CPU占用
    }
}

// 初始化函数
mp_obj_t esp_cat_detection_init(void) {
    if (cat_task_handle == NULL) {
        xTaskCreate(cat_detection_task, "cat_detection_task", 10 * 1024, NULL, 10, &cat_task_handle);
    }
    return mp_const_none;
}

// 反初始化函数
mp_obj_t esp_cat_detection_deinit(void) {
    if (cat_task_handle != NULL) {
        vTaskDelete(cat_task_handle);
        cat_task_handle = NULL;
    }
    return mp_const_none;
}

// Python接口：获取最新的人脸检测结果
mp_obj_t esp_cat_detection(void) {
    // xSemaphoreTake(cat_mutex, portMAX_DELAY);
    mp_obj_t elements[] = {
        mp_obj_new_int(cat_result.num),
        mp_obj_new_int(cat_result.x),
        mp_obj_new_int(cat_result.y),
    };
    return mp_obj_new_list(3, elements);
}


