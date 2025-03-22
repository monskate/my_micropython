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
#include "human_face_detect_msr01.hpp"
#include "human_face_detect_mnp01.hpp"

// 定义队列的数据结构
static struct {
    int num;
    int x1;
    int y1;
    int x2;
    int y2;
} face_result = {0, 0, 0, 0, 0};

static TaskHandle_t face_task_handle = NULL;
static SemaphoreHandle_t face_mutex = NULL;

// 人脸识别任务
void face_detection_task(void *pvParameters) {
    HumanFaceDetectMSR01 detector(0.3F, 0.3F, 10, 0.3F);
    HumanFaceDetectMNP01 detector2(0.4F, 0.3F, 10);
    while (true) {
        camera_fb_t *frame = esp_camera_fb_get();
        printf("idf1\n");
        if (frame != NULL) {
            std::list<dl::detect::result_t> &detect_candidates = detector.infer((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3});
            std::list<dl::detect::result_t> &detect_results = detector2.infer((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3}, detect_candidates);
            printf("idf2\n");
            // xSemaphoreTake(face_mutex, portMAX_DELAY);
            face_result.num = detect_results.size();
            if (!detect_results.empty()) {
                auto prediction = detect_results.begin();
                face_result.x1 = prediction->box[0];
                face_result.x2 = prediction->box[1];
                face_result.y1 = prediction->box[2];
                face_result.y2 = prediction->box[3];
            }
            // xSemaphoreGive(face_mutex);
            
            esp_camera_fb_return(frame);
        }
        vTaskDelay(pdMS_TO_TICKS(50)); // 降低CPU占用
    }
}

// 初始化函数
mp_obj_t esp_face_detection_init(void) {
    // if (face_mutex == NULL) {
    //     face_mutex = xSemaphoreCreateMutex();
    // }
    if (face_task_handle == NULL) {
        xTaskCreate(face_detection_task, "face_detection_task", 10 * 1024, NULL, 10, &face_task_handle);
        printf("任务创建\n");
    }
    return mp_const_none;
}

// 反初始化函数
mp_obj_t esp_face_detection_deinit(void) {
    if (face_task_handle != NULL) {
        vTaskDelete(face_task_handle);
        face_task_handle = NULL;
    }
    // if (face_mutex != NULL) {
    //     vSemaphoreDelete(face_mutex);
    //     face_mutex = NULL;
    // }
    return mp_const_none;
}

// Python接口：获取最新的人脸检测结果
mp_obj_t esp_face_detection(void) {
    // xSemaphoreTake(face_mutex, portMAX_DELAY);
    mp_obj_t elements[] = {
        mp_obj_new_int(face_result.num),
        mp_obj_new_int(face_result.x1),
        mp_obj_new_int(face_result.x2),
        mp_obj_new_int(face_result.y1),
        mp_obj_new_int(face_result.y2)
    };
    printf("jieguo\n");
    // xSemaphoreGive(face_mutex);
    return mp_obj_new_list(5, elements);
}