#include "models.h"


static mp_obj_t mp_py_classification_init(mp_obj_t models_obj)
{
    return classification_init(models_obj);
}
static MP_DEFINE_CONST_FUN_OBJ_1(class_init, mp_py_classification_init);

static mp_obj_t mp_py_classification(mp_obj_t param_obj)
{
    return classification(param_obj);
}
static MP_DEFINE_CONST_FUN_OBJ_1(class, mp_py_classification);

static mp_obj_t mp_py_classification_dinit(void)
{
    return classification_dinit();
}
static MP_DEFINE_CONST_FUN_OBJ_0(class_dinit, mp_py_classification_dinit);

static const mp_rom_map_elem_t mp_module_ai_models_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_esp_models) },
    { MP_ROM_QSTR(MP_QSTR_ai_class_init), MP_ROM_PTR(&class_init) },
	{ MP_ROM_QSTR(MP_QSTR_ai_class), MP_ROM_PTR(&class) },
	{ MP_ROM_QSTR(MP_QSTR_ai_class_dinit), MP_ROM_PTR(&class_dinit) },

};
static MP_DEFINE_CONST_DICT(mp_module_ai_models_globals, mp_module_ai_models_globals_table);


const mp_obj_module_t mp_module_ai_models = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_ai_models_globals,
};

MP_REGISTER_MODULE(MP_QSTR_esp_models, mp_module_ai_models);


