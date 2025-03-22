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
#include "face_recognition_tool.hpp"
#include "face_recognition_112_v1_s8.hpp"

// 定义队列的数据结构
typedef struct
{
    int id;            // 识别到的人脸ID
} face_result_t;

static QueueHandle_t id_queue = NULL;
static TaskHandle_t face_task_handle = NULL;

static int recognition_mode = 0;

static face_info_t recognize_result;

// 设置模式
mp_obj_t esp_face_set_mode(mp_obj_t mode_obj)
{
    recognition_mode = mp_obj_get_int(mode_obj);
    return mp_const_none;
}

// 人脸识别任务
void face_recognition_task(void *pvParameters)
{
    HumanFaceDetectMSR01 detector(0.3F, 0.3F, 10, 0.3F);
    HumanFaceDetectMNP01 detector2(0.4F, 0.3F, 10);

    FaceRecognition112V1S8 *recognizer = new FaceRecognition112V1S8();

    recognizer->set_partition(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "fr");
    recognizer->set_ids_from_flash();

    int param = (int)(uintptr_t)pvParameters;

    while (true)
    {
        camera_fb_t *frame = esp_camera_fb_get();
        face_result_t result;
        result.id = -1;
        if (frame != NULL)
        {
            std::list<dl::detect::result_t> &detect_candidates = detector.infer((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3});
            std::list<dl::detect::result_t> &detect_results = detector2.infer((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3}, detect_candidates);

            if (detect_results.size() == 1)
            {

                if (recognition_mode == 0) // 识别模式
                {
                    recognize_result = recognizer->recognize((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3}, detect_results.front().keypoint);
                    result.id = recognize_result.id;
                    printf("Recognized ID: %d\n", result.id);
                }
                else if (recognition_mode == 1) // 注册模式
                {
                    recognizer->enroll_id((uint16_t *)frame->buf, {(int)frame->height, (int)frame->width, 3}, detect_results.front().keypoint, "", true);
                    result.id = recognizer->get_enrolled_ids().back().id;
                    printf("Enrolled new ID: %d\n", result.id);
                    recognition_mode = 0;
                }
            }
            // 将识别到的ID和图像数据发送到队列中
            xQueueSend(id_queue, &result, portMAX_DELAY);
            esp_camera_fb_return(frame);
            frame = NULL;
        }
    }
}

// 初始化函数
mp_obj_t esp_face_init(void)
{
    if (id_queue == NULL)
    {
        id_queue = xQueueCreate(5, sizeof(face_result_t)); // 队列容量为10
    }

    if (face_task_handle == NULL)
    {
        xTaskCreate(face_recognition_task, "face_recognition_task", 8 * 1024, NULL, 5, &face_task_handle);
    }

    return mp_const_none;
}

mp_obj_t esp_face_deinit(void)
{
    // 检查队列是否已创建
    if (id_queue != NULL)
    {
        vQueueDelete(id_queue);  // 删除队列
        id_queue = NULL;  // 清空队列指针
    }

    // 检查任务句柄是否有效
    if (face_task_handle != NULL)
    {
        vTaskDelete(face_task_handle);  // 删除任务
        face_task_handle = NULL;  // 清空任务句柄
    }

    return mp_const_none;
}

// Python接口：从队列中获取ID和图像数据
mp_obj_t esp_face_discern(void)
{
    face_result_t result;
    xQueueReceive(id_queue, &result, portMAX_DELAY);

    // 创建人脸ID的Python对象
    mp_obj_t id = mp_obj_new_int(result.id);

    return id;
}
