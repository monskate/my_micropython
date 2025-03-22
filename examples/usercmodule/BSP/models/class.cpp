
extern "C"
{
#include "models.h"
#include "esp_log.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "py/mphal.h"
#include "extmod/modmachine.h"
#include "img_converters.h"
#include <stdio.h>
#include <string.h>
#include "esp_camera.h"
}
#include "core/algorithm/el_algorithm_yolo.h"
#include "core/algorithm/el_algorithm_imcls.h"
#include "core/engine/el_engine_tflite.h"
#include "core/utils/el_cv.h"
#include "wu_20_model_data.h"
#include "lubiao_model_data.h"
#include "face_model_data.h"
#include "porting/el_device.h"

#define kTensorArenaSize (1024 * 1024)

using namespace edgelab;
EngineTFLite *engine = nullptr;      // 引擎指针
void *tensor_arena = nullptr;        // 张量区域指针
AlgorithmIMCLS *algorithm = nullptr; // 分类算法指针
AlgorithmYOLO *algorithm1 = nullptr;
uint8_t models_flag = 0;

uint16_t color[] = {
    0x0000,
    0x03E0,
    0x001F,
    0x7FE0,
    0xFFFF,
};

mp_obj_t classification_init(mp_obj_t models_obj)
{
    models_flag = mp_obj_get_int(models_obj);
    engine = new EngineTFLite();
    tensor_arena = heap_caps_malloc(kTensorArenaSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    engine->init(tensor_arena, kTensorArenaSize);
    if (models_flag == 0)
    {
        engine->load_model(WU_20_model_data, WU_20_model_data_len);
        algorithm = new AlgorithmIMCLS(engine, 20);
    }
    if (models_flag == 1)
    {
        engine->load_model(g_lubiao_model_data, g_lubiao_model_data_len);
        algorithm = new AlgorithmIMCLS(engine, 20);
    }
    if (models_flag == 2)
    {
        engine->load_model(g_face_model_data, g_face_model_data_len);
        algorithm1 = new AlgorithmYOLO(engine);
    }
    return mp_const_none;
}
size_t jpeg_len = 0;
uint8_t *jpeg = NULL;

mp_obj_t classification(mp_obj_t param_obj)
{
    mp_obj_t ai_image = mp_const_none;
    mp_obj_t result = mp_obj_new_str("", 0); // 初始化状态为 0
    mp_obj_t x = mp_obj_new_int(0);
    mp_obj_t y = mp_obj_new_int(0);
    mp_obj_t w = mp_obj_new_int(0);
    mp_obj_t h = mp_obj_new_int(0);
    uint8_t param = mp_obj_get_int(param_obj);
    el_img_t img;
    camera_fb_t *fb = esp_camera_fb_get();
    if (fb != NULL)
    {
        img.width = fb->width;
        img.height = fb->height;
        img.data = fb->buf;
        img.size = fb->len;
        img.format = EL_PIXEL_FORMAT_RGB565;
        esp_camera_fb_return(fb);
    }
    uint8_t i = 0u;
    if (models_flag == 0)
    {
        algorithm->run(&img);
        for (const auto &cls : algorithm->get_results())
        {
            if (cls.score > param)
            {
                result = mp_obj_new_str(WU_20_model_classes[cls.target], strlen(WU_20_model_classes[cls.target]));
            }
        }
    }
    if (models_flag == 1)
    {
        algorithm->run(&img);
        for (const auto &cls : algorithm->get_results())
        {
            if (cls.score > param)
            {
                result = mp_obj_new_str(g_lubiao_model_classes[cls.target], strlen(g_lubiao_model_classes[cls.target]));
            }
        }
    }
    if (models_flag == 2)
    {
        algorithm1->run(&img);
        for (const auto &box : algorithm1->get_results())
        {
            x = mp_obj_new_int(box.x);
            y = mp_obj_new_int(box.y);
            w = mp_obj_new_int(box.w);
            h = mp_obj_new_int(box.h);
            int16_t y = box.y - box.h / 2;
            int16_t x = box.x - box.w / 2;
            el_draw_rect(&img, x, y, box.w, box.h, color[++i % 5], 4);
        }
    }
    fmt2jpg(img.data, img.size, img.width, img.height, PIXFORMAT_RGB565, 80, &jpeg, &jpeg_len);
    ai_image = mp_obj_new_bytes(jpeg, jpeg_len);
    if (jpeg)
    {
        free(jpeg);
        jpeg = NULL;
    }
    esp_camera_fb_return(fb);
    mp_obj_t elements[] = {ai_image,result, x, y, w, h};

    return mp_obj_new_list(6, elements);
}

mp_obj_t classification_dinit(void)
{
    delete algorithm;
    delete algorithm1;
    delete engine;
    return mp_const_none;
}
