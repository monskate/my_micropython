#ifndef MICROPY_INCLUDED_ESP32_MODCAMERA_H
#define MICROPY_INCLUDED_ESP32_MODCAMERA_H

#define TAG "camera"


//
#define CAM_PIN_PWDN                -1
#define CAM_PIN_RESET               -1
#define CAM_PIN_XCLK                10
#define CAM_PIN_SIOD                15
#define CAM_PIN_SIOC                16

#define CAM_PIN_D7                  9
#define CAM_PIN_D6                  11
#define CAM_PIN_D5                  12
#define CAM_PIN_D4                  14
#define CAM_PIN_D3                  47
#define CAM_PIN_D2                  45
#define CAM_PIN_D1                  48
#define CAM_PIN_D0                  21
#define CAM_PIN_VSYNC               3
#define CAM_PIN_HREF                46
#define CAM_PIN_PCLK                13


//

#define XCLK_FREQ_10MHz    10000000
#define XCLK_FREQ_20MHz    20000000
#define XCLK_FREQ_24MHz    24000000

//White Balance
#define WB_NONE     0
#define WB_SUNNY    1
#define WB_CLOUDY   2
#define WB_OFFICE   3
#define WB_HOME     4

//Special Effect  
#define EFFECT_NONE    0
#define EFFECT_NEG     1
#define EFFECT_BW      2
#define EFFECT_RED     3
#define EFFECT_GREEN   4
#define EFFECT_BLUE    5
#define EFFECT_RETRO   6

#define MODULE_CAMERA_ENABLED   1

#endif
