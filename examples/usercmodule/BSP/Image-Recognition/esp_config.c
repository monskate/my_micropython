#include "esp_config.h"

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

//颜色
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

static const mp_rom_map_elem_t mp_module_ai_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_esp_who) },

    { MP_ROM_QSTR(MP_QSTR_ai_face_recognition_init), MP_ROM_PTR(&ai_face_recognition_init) },
    { MP_ROM_QSTR(MP_QSTR_ai_face_recognition_deinit), MP_ROM_PTR(&ai_face_recognition_deinit) },
    { MP_ROM_QSTR(MP_QSTR_ai_face_set), MP_ROM_PTR(&ai_face_set) },
	{ MP_ROM_QSTR(MP_QSTR_ai_face_recognition), MP_ROM_PTR(&ai_face_recognition) },

    { MP_ROM_QSTR(MP_QSTR_ai_face_detection_init), MP_ROM_PTR(&ai_face_detection_init) },
    { MP_ROM_QSTR(MP_QSTR_ai_face_detection_deinit), MP_ROM_PTR(&ai_face_detection_deinit) },
	{ MP_ROM_QSTR(MP_QSTR_ai_face_detection), MP_ROM_PTR(&ai_face_detection) },

    { MP_ROM_QSTR(MP_QSTR_ai_qr_init), MP_ROM_PTR(&ai_qr_init) },
    { MP_ROM_QSTR(MP_QSTR_ai_qr_deinit), MP_ROM_PTR(&ai_qr_deinit) },
	{ MP_ROM_QSTR(MP_QSTR_ai_qr), MP_ROM_PTR(&ai_qr) },

    { MP_ROM_QSTR(MP_QSTR_ai_color_init), MP_ROM_PTR(&ai_color_init) },
    { MP_ROM_QSTR(MP_QSTR_ai_color_deinit), MP_ROM_PTR(&ai_color_deinit) },
	{ MP_ROM_QSTR(MP_QSTR_ai_color), MP_ROM_PTR(&ai_color) },
    { MP_ROM_QSTR(MP_QSTR_ai_color_set), MP_ROM_PTR(&ai_color_set) },
    
};
static MP_DEFINE_CONST_DICT(mp_module_ai_globals, mp_module_ai_globals_table);


const mp_obj_module_t mp_module_ai = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_ai_globals,
};

MP_REGISTER_MODULE(MP_QSTR_esp_who, mp_module_ai);