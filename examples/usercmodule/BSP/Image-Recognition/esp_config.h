#ifndef __ESP_CONFIG_H
#define __ESP_CONFIG_H

#include "py/runtime.h"
#include "esp_camera.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"


/*****************************颜色识别*****************************/
#define RGB565_LCD_RED      	0x00F8
#define RGB565_LCD_ORANGE   	0x20FD
#define RGB565_LCD_YELLOW   	0xE0FF
#define RGB565_LCD_GREEN    	0xE007
#define RGB565_LCD_CYAN     	0xFF07
#define RGB565_LCD_BLUE     	0x1F00
#define RGB565_LCD_PURPLE   	0x1EA1
#define RGB565_LCD_WHITE    	0xFFFF
#define RGB565_LCD_GRAY     	0x1084
#define RGB565_LCD_BLACK    	0x0000

#define RGB565_MASK_RED         0xF800
#define RGB565_MASK_GREEN       0x07E0
#define RGB565_MASK_BLUE        0x001F




extern mp_obj_t esp_face_discern(void);
extern mp_obj_t esp_face_init(void);
extern mp_obj_t esp_face_deinit(void);
extern mp_obj_t esp_face_set_mode(mp_obj_t mode_obj);

extern mp_obj_t esp_qr_detection_ai_start(void);
extern mp_obj_t esp_qr_discern(void);
extern mp_obj_t esp_qr_detection_ai_deinit(void);

extern mp_obj_t esp_color_init(void);
extern mp_obj_t esp_color_discern(void);
extern mp_obj_t esp_color_deinit(void);
extern mp_obj_t esp_set_selected_color(mp_obj_t color_name);

extern mp_obj_t esp_face_detection_init(void);
extern mp_obj_t esp_face_detection_deinit(void);
extern mp_obj_t esp_face_detection(void);

#endif
