#ifndef __MODELS_H
#define __MODELS_H

#include "py/runtime.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"




// extern mp_obj_t class_camera_init(void);
extern mp_obj_t classification_init(mp_obj_t models_obj);
extern mp_obj_t classification(mp_obj_t param_obj);
extern mp_obj_t classification_dinit(void);

#endif
