#include "esp_config.h"
#include "apriltag.h"
#include "tag36h11.h"
#include "common/image_u8.h"
#include "common/zarray.h"
#include "stdint.h"
#include <math.h>
// MicroPython封装函数
static apriltag_detector_t *td = NULL;
static apriltag_family_t *tf = NULL;

static mp_obj_t apriltag_init(void) {
    if (td == NULL && tf == NULL) {
        tf = tag36h11_create();
        

        td = apriltag_detector_create();
        apriltag_detector_add_family(td, tf);
        td->quad_sigma = 0.0;
        td->quad_decimate = 4.0;
        td->refine_edges = 0;
        td->decode_sharpening = 0;
        td->nthreads = 1;
        td->debug = 0;
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(apriltag_init_obj, apriltag_init);

static mp_obj_t apriltag_deinit(void) {
    if (td) {
        apriltag_detector_destroy(td);
        td = NULL;
    }
    if (tf) {
        tag36h11_destroy(tf);
        tf = NULL;
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(apriltag_deinit_obj, apriltag_deinit);

static mp_obj_t apriltag_detect(mp_obj_t img_buf_obj, mp_obj_t width_obj, mp_obj_t height_obj) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(img_buf_obj, &bufinfo, MP_BUFFER_READ);

    int width = mp_obj_get_int(width_obj);
    int height = mp_obj_get_int(height_obj);

    image_u8_t im = {
        .width = width,
        .height = height,
        .stride = width,
        .buf = (uint8_t*)bufinfo.buf
    };

    if (!td || !tf) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("apriltag not initialized"));
    }

    zarray_t *detections = apriltag_detector_detect(td, &im);
    mp_obj_list_t *result_list = mp_obj_new_list(0, NULL);
    if (zarray_size(detections) > 0) {
        for (int i = 0; i < zarray_size(detections); i++) {
            apriltag_detection_t *det;
            zarray_get(detections, i, &det);
            float w1 = sqrtf(powf(det->p[1][0] - det->p[0][0], 2) + powf(det->p[1][1] - det->p[0][1], 2));
            float w2 = sqrtf(powf(det->p[2][0] - det->p[3][0], 2) + powf(det->p[2][1] - det->p[3][1], 2));
            float tag_width = (w1 + w2) / 2.0f;

            // 计算高度（上下边）
            float h1 = sqrtf(powf(det->p[3][0] - det->p[0][0], 2) + powf(det->p[3][1] - det->p[0][1], 2));
            float h2 = sqrtf(powf(det->p[2][0] - det->p[1][0], 2) + powf(det->p[2][1] - det->p[1][1], 2));
            float tag_height = (h1 + h2) / 2.0f;
            mp_obj_t dict = mp_obj_new_dict(0);
            mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_id), mp_obj_new_int(det->id));
            mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_center_x), mp_obj_new_float(det->c[0]-160));
            mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_center_y), mp_obj_new_float(120 - det->c[1]));
            mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_width), mp_obj_new_float(tag_width));
            mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_height), mp_obj_new_float(tag_height));
        
            mp_obj_list_append(result_list, dict);
        }
    }
    apriltag_detections_destroy(detections);

    return result_list;
}
static MP_DEFINE_CONST_FUN_OBJ_3(apriltag_detect_obj, apriltag_detect);

//人脸识别
static mp_obj_t mp_py_face_recognition_init(void)
{
    return esp_face_init();
}
static MP_DEFINE_CONST_FUN_OBJ_0(ai_face_recognition_init, mp_py_face_recognition_init);

static mp_obj_t mp_py_face_recognition_deinit(void)
{
    return esp_face_deinit();
}
static MP_DEFINE_CONST_FUN_OBJ_0(ai_face_recognition_deinit, mp_py_face_recognition_deinit);

static mp_obj_t mp_py_face_recognition(void)
{
    return esp_face_discern();
}
static MP_DEFINE_CONST_FUN_OBJ_0(ai_face_recognition, mp_py_face_recognition);

static mp_obj_t mp_py_face_set(mp_obj_t param_obj)
{
    return esp_face_set_mode(param_obj);
}
static MP_DEFINE_CONST_FUN_OBJ_1(ai_face_set, mp_py_face_set);

//二维码
static mp_obj_t mp_py_qr_init(void)
{
    return esp_qr_detection_ai_start();
}
static MP_DEFINE_CONST_FUN_OBJ_0(ai_qr_init, mp_py_qr_init);

static mp_obj_t mp_py_qr_deinit(void)
{
    return esp_qr_detection_ai_deinit();
}
static MP_DEFINE_CONST_FUN_OBJ_0(ai_qr_deinit, mp_py_qr_deinit);

static mp_obj_t mp_py_qr(void)
{
    return esp_qr_discern();
}
static MP_DEFINE_CONST_FUN_OBJ_0(ai_qr, mp_py_qr);

//人脸检测
static mp_obj_t mp_py_face_detection_init(void)
{
    return esp_face_detection_init();
}
static MP_DEFINE_CONST_FUN_OBJ_0(ai_face_detection_init, mp_py_face_detection_init);

static mp_obj_t mp_py_face_detection_deinit(void)
{
    return esp_face_detection_deinit();
}
static MP_DEFINE_CONST_FUN_OBJ_0(ai_face_detection_deinit, mp_py_face_detection_deinit);

static mp_obj_t mp_py_face_detection(void)
{
    return esp_face_detection();
}
static MP_DEFINE_CONST_FUN_OBJ_0(ai_face_detection, mp_py_face_detection);

//猫脸检测
static mp_obj_t mp_py_cat_detection_init(void)
{
    return esp_cat_detection_init();
}
static MP_DEFINE_CONST_FUN_OBJ_0(ai_cat_detection_init, mp_py_cat_detection_init);

static mp_obj_t mp_py_cat_detection_deinit(void)
{
    return esp_cat_detection_deinit();
}
static MP_DEFINE_CONST_FUN_OBJ_0(ai_cat_detection_deinit, mp_py_cat_detection_deinit);

static mp_obj_t mp_py_cat_detection(void)
{
    return esp_cat_detection();
}
static MP_DEFINE_CONST_FUN_OBJ_0(ai_cat_detection, mp_py_cat_detection);

//颜色追踪
static mp_obj_t mp_py_color_init(void)
{
    return esp_color_init();
}
static MP_DEFINE_CONST_FUN_OBJ_0(ai_color_init, mp_py_color_init);

static mp_obj_t mp_py_color_deinit(void)
{
    return esp_color_deinit();
}
static MP_DEFINE_CONST_FUN_OBJ_0(ai_color_deinit, mp_py_color_deinit);

static mp_obj_t mp_py_color(void)
{
    return esp_color_discern();
}
static MP_DEFINE_CONST_FUN_OBJ_0(ai_color, mp_py_color);

static mp_obj_t mp_py_color_set(mp_obj_t param_obj)
{
    return esp_set_selected_color(param_obj);
}
static MP_DEFINE_CONST_FUN_OBJ_1(ai_color_set, mp_py_color_set);

//颜色识别
static mp_obj_t mp_py_color_rgb_init(void)
{
    return esp_color_rgb_init();
}
static MP_DEFINE_CONST_FUN_OBJ_0(ai_color_rgb_init, mp_py_color_rgb_init);

static mp_obj_t mp_py_color_rgb_deinit(void)
{
    return esp_color_rgb_deinit();
}
static MP_DEFINE_CONST_FUN_OBJ_0(ai_color_rgb_deinit, mp_py_color_rgb_deinit);

static mp_obj_t mp_py_color_rgb(void)
{
    return esp_color_rgb();
}
static MP_DEFINE_CONST_FUN_OBJ_0(ai_color_rgb, mp_py_color_rgb);

static mp_obj_t mp_py_color_region(size_t n_args, const mp_obj_t *args)
{
    return esp_color_set_region(args[0], args[1], args[2], args[3]);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(ai_color_region, 4, 4, mp_py_color_region);

static const mp_rom_map_elem_t mp_module_ai_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_esp_who) },

    { MP_ROM_QSTR(MP_QSTR_ai_face_recognition_init), MP_ROM_PTR(&ai_face_recognition_init) },
    { MP_ROM_QSTR(MP_QSTR_ai_face_recognition_deinit), MP_ROM_PTR(&ai_face_recognition_deinit) },
    { MP_ROM_QSTR(MP_QSTR_ai_face_set), MP_ROM_PTR(&ai_face_set) },
	{ MP_ROM_QSTR(MP_QSTR_ai_face_recognition), MP_ROM_PTR(&ai_face_recognition) },

    { MP_ROM_QSTR(MP_QSTR_ai_face_detection_init), MP_ROM_PTR(&ai_face_detection_init) },
    { MP_ROM_QSTR(MP_QSTR_ai_face_detection_deinit), MP_ROM_PTR(&ai_face_detection_deinit) },
	{ MP_ROM_QSTR(MP_QSTR_ai_face_detection), MP_ROM_PTR(&ai_face_detection) },

    { MP_ROM_QSTR(MP_QSTR_ai_cat_detection_init), MP_ROM_PTR(&ai_cat_detection_init) },
    { MP_ROM_QSTR(MP_QSTR_ai_cat_detection_deinit), MP_ROM_PTR(&ai_cat_detection_deinit) },
	{ MP_ROM_QSTR(MP_QSTR_ai_cat_detection), MP_ROM_PTR(&ai_cat_detection) },

    { MP_ROM_QSTR(MP_QSTR_ai_qr_init), MP_ROM_PTR(&ai_qr_init) },
    { MP_ROM_QSTR(MP_QSTR_ai_qr_deinit), MP_ROM_PTR(&ai_qr_deinit) },
	{ MP_ROM_QSTR(MP_QSTR_ai_qr), MP_ROM_PTR(&ai_qr) },

    { MP_ROM_QSTR(MP_QSTR_ai_color_init), MP_ROM_PTR(&ai_color_init) },
    { MP_ROM_QSTR(MP_QSTR_ai_color_deinit), MP_ROM_PTR(&ai_color_deinit) },
	{ MP_ROM_QSTR(MP_QSTR_ai_color), MP_ROM_PTR(&ai_color) },
    { MP_ROM_QSTR(MP_QSTR_ai_color_set), MP_ROM_PTR(&ai_color_set) },

    { MP_ROM_QSTR(MP_QSTR_ai_color_rgb_init), MP_ROM_PTR(&ai_color_rgb_init) },
    { MP_ROM_QSTR(MP_QSTR_ai_color_rgb_deinit), MP_ROM_PTR(&ai_color_rgb_deinit) },
	{ MP_ROM_QSTR(MP_QSTR_ai_color_rgb), MP_ROM_PTR(&ai_color_rgb) },
    { MP_ROM_QSTR(MP_QSTR_ai_color_region), MP_ROM_PTR(&ai_color_region) },

    { MP_ROM_QSTR(MP_QSTR_apriltag_init), MP_ROM_PTR(&apriltag_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_apriltag_deinit), MP_ROM_PTR(&apriltag_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_apriltag_detect), MP_ROM_PTR(&apriltag_detect_obj) },
    
};
static MP_DEFINE_CONST_DICT(mp_module_ai_globals, mp_module_ai_globals_table);


const mp_obj_module_t mp_module_ai = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_ai_globals,
};

MP_REGISTER_MODULE(MP_QSTR_esp_who, mp_module_ai);