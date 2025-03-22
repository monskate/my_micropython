#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_process_sdkconfig.h"
#include "esp_wn_iface.h"
#include "esp_wn_models.h"
#include "esp_afe_sr_iface.h"
#include "esp_afe_sr_models.h"
#include "esp_mn_iface.h"
#include "esp_mn_models.h"
#include "esp_board_init.h"
#include "model_path.h"
#include "modaudio.h"
#include "dl_lib_coefgetter_if.h"
#include "esp_nsn_models.h"

#include "esp_http_client.h"
#include "esp_event.h"

#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "py/mphal.h"
#include "py/stream.h"

int detect_flag = 0;
volatile int result = -1;
int tuixia = 0;

static volatile int task_flag = 0;

esp_afe_sr_iface_t *afe_handle = NULL;
esp_afe_sr_data_t *afe_data = NULL;

srmodel_list_t *models = NULL;


int ai_vol = 50;
bool is_recording = false;

// 喂入语音处理
void feed_Task(void *arg)
{
    esp_afe_sr_data_t *afe_data = arg;
    int audio_chunksize = afe_handle->get_feed_chunksize(afe_data);
    int nch = afe_handle->get_channel_num(afe_data);
    int feed_channel = esp_get_feed_channel();
    assert(nch <= feed_channel);
    int16_t *i2s_buff = malloc(audio_chunksize * sizeof(int16_t) * feed_channel);
    assert(i2s_buff);
    while (task_flag)
    {
        esp_get_feed_data(false, i2s_buff, audio_chunksize * sizeof(int16_t) * feed_channel);

        afe_handle->feed(afe_data, i2s_buff);
    }
    if (i2s_buff)
    {
        free(i2s_buff);
        i2s_buff = NULL;
    }
    vTaskDelete(NULL);
}

// 语音识别
void detect_Task(void *arg)
{
    esp_afe_sr_data_t *afe_data = arg;
    int afe_chunksize = afe_handle->get_fetch_chunksize(afe_data);
    char *mn_name = esp_srmodel_filter(models, ESP_MN_PREFIX, ESP_MN_CHINESE);
    printf("multinet:%s\n", mn_name);
    esp_mn_iface_t *multinet = esp_mn_handle_from_name(mn_name);
    model_iface_data_t *model_data = multinet->create(mn_name, 6000);
    esp_mn_commands_update_from_sdkconfig(multinet, model_data); // Add speech commands from sdkconfig
    int mu_chunksize = multinet->get_samp_chunksize(model_data);
    assert(mu_chunksize == afe_chunksize);

    multinet->print_active_speech_commands(model_data);
    printf("------------detect start------------\n");

    while (task_flag)
    {
        afe_fetch_result_t *res = afe_handle->fetch(afe_data);
        if (!res || res->ret_value == ESP_FAIL)
        {
            break;
        }

        if (res->wakeup_state == WAKENET_DETECTED)
        {
            multinet->clean(model_data); // clean all status of multinet
        }
        else if (res->wakeup_state == WAKENET_CHANNEL_VERIFIED)
        {
            detect_flag = 1;
            printf("AFE_FETCH_CHANNEL_VERIFIED, channel index: %d\n", res->trigger_channel_id);
        }

        if (detect_flag == 1)
        {
            esp_mn_state_t mn_state = multinet->detect(model_data, res->data);

            if (mn_state == ESP_MN_STATE_DETECTING)
            {
                continue;
            }

            if (mn_state == ESP_MN_STATE_DETECTED)
            {
                result = -1;
                esp_mn_results_t *mn_result = multinet->get_results(model_data);

                printf("TOP %d, command_id: %d, phrase_id: %d, string:%s prob: %f\n",
                       0, mn_result->command_id[0], mn_result->phrase_id[0], mn_result->string, mn_result->prob[0]);
                if (mn_result->command_id[0]==0)
                {
                    tuixia = 1;
                }
                else{
                    result = mn_result->command_id[0];
                }
                

                printf("\n-----------listening-----------\n");
            }

            if (tuixia == 1)
            {
                esp_mn_results_t *mn_result = multinet->get_results(model_data);
                afe_handle->enable_wakenet(afe_data);
                detect_flag = 0;
                tuixia = 0;
                printf("\n-----------awaits to be waken up-----------\n");
                continue;
            }
        }
    }
    if (model_data)
    {
        multinet->destroy(model_data);
        model_data = NULL;
    }
    vTaskDelete(NULL);
}

// 语音识别初始化
static mp_obj_t esp_audio_ai_start(void)
{
    models = esp_srmodel_init("model");

    afe_handle = (esp_afe_sr_iface_t *)&ESP_AFE_SR_HANDLE;
    afe_config_t afe_config = AFE_CONFIG_DEFAULT();

    afe_config.wakenet_model_name = esp_srmodel_filter(models, ESP_WN_PREFIX, NULL);
    afe_data = afe_handle->create_from_config(&afe_config);
    
    task_flag = 1;
    xTaskCreatePinnedToCore(&detect_Task, "detect", 8 * 1024, (void *)afe_data, 1, NULL, 1);
    xTaskCreatePinnedToCore(&feed_Task, "feed", 8 * 1024, (void *)afe_data, 1, NULL, 0);
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(audio_ai_start, esp_audio_ai_start);

// 删除语音识别任务
static mp_obj_t esp_audio_ai_deinit(void)
{
    task_flag = 0;
    afe_handle->destroy(afe_data);
    afe_data = NULL;
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(audio_ai_deinit, esp_audio_ai_deinit);

// 返回识别结果
static mp_obj_t esp_audio_ai_result(void)
{
    return mp_obj_new_int(result);
}
static MP_DEFINE_CONST_FUN_OBJ_0(audio_ai_result, esp_audio_ai_result);

// I2C初始化
static mp_obj_t esp_audio_ai_I2C_init(void)
{
    esp_audio_i2c_init();

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(I2C_init, esp_audio_ai_I2C_init);

// I2S初始化
static mp_obj_t esp_audio_ai_I2S_init(void)
{
    esp_audio_i2s_init();

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(audio_I2S_init, esp_audio_ai_I2S_init);

// ES8311初始化
static mp_obj_t esp_audio_ai_ES8311_init(void)
{
    esp_audio_es8311_init();

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(audio_es8311_init, esp_audio_ai_ES8311_init);

// ES7210初始化
static mp_obj_t esp_audio_ai_ES7210_init(void)
{
    esp_audio_es7210_init();

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(audio_es7210_init, esp_audio_ai_ES7210_init);

// ES8311反初始化
static mp_obj_t esp_audio_ai_ES8311_deinit(void)
{
    esp_audio_es8311_deinit();

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(audio_es8311_deinit, esp_audio_ai_ES8311_deinit);

// ES7210反初始化
static mp_obj_t esp_audio_ai_ES7210_deinit(void)
{
    esp_audio_es7210_deinit();

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(audio_es7210_deinit, esp_audio_ai_ES7210_deinit);

// 返回声音大小
int16_t audio_buffer[1024];
static mp_obj_t esp_audio_ai_vol(void)
{
    esp_get_feed_data(false, audio_buffer, 1024);
    double sum_squares = 0.0;
    for (int i = 0; i < 1024; i++)
    {
        sum_squares += audio_buffer[i] * audio_buffer[i];
    }

    double rms = sqrt(sum_squares / 1024);
    int volume_db = 20 * log10(rms);
    return mp_obj_new_int(volume_db);
}
static MP_DEFINE_CONST_FUN_OBJ_0(audio_ai_vol, esp_audio_ai_vol);
//设置播放声音大小
static mp_obj_t esp_music_vol_set(mp_obj_t arg_1_obj)
{
    int arg_1 = mp_obj_get_int(arg_1_obj);
    esp_audio_set_play_vol(arg_1);
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_1(audio_music_vol_set, esp_music_vol_set);

// 播放声音(非阻塞)
static mp_obj_t esp_music_play(mp_obj_t music_obj)
{
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(music_obj, &bufinfo, MP_BUFFER_READ);

    esp_audio_play((int16_t *)bufinfo.buf, bufinfo.len, portMAX_DELAY);

    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_1(audio_music_play, esp_music_play);

static const mp_rom_map_elem_t mp_module_audio_ai_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_esp_audio)},

    {MP_ROM_QSTR(MP_QSTR_I2C_init), MP_ROM_PTR(&I2C_init)},
    {MP_ROM_QSTR(MP_QSTR_audio_I2S_init), MP_ROM_PTR(&audio_I2S_init)},
    {MP_ROM_QSTR(MP_QSTR_audio_es8311_init), MP_ROM_PTR(&audio_es8311_init)},
    {MP_ROM_QSTR(MP_QSTR_audio_es7210_init), MP_ROM_PTR(&audio_es7210_init)},
    {MP_ROM_QSTR(MP_QSTR_audio_es8311_deinit), MP_ROM_PTR(&audio_es8311_deinit)},
    {MP_ROM_QSTR(MP_QSTR_audio_es7210_deinit), MP_ROM_PTR(&audio_es7210_deinit)},

    {MP_ROM_QSTR(MP_QSTR_ai_audio_start), MP_ROM_PTR(&audio_ai_start)},
    {MP_ROM_QSTR(MP_QSTR_ai_audio_deinit), MP_ROM_PTR(&audio_ai_deinit)},
    {MP_ROM_QSTR(MP_QSTR_ai_audio_result), MP_ROM_PTR(&audio_ai_result)},
    {MP_ROM_QSTR(MP_QSTR_ai_audio_vol), MP_ROM_PTR(&audio_ai_vol)},

    {MP_ROM_QSTR(MP_QSTR_music_play), MP_ROM_PTR(&audio_music_play)},
    {MP_ROM_QSTR(MP_QSTR_music_vol_set), MP_ROM_PTR(&audio_music_vol_set)},
};
static MP_DEFINE_CONST_DICT(mp_module_audio_ai_globals, mp_module_audio_ai_globals_table);

const mp_obj_module_t mp_module_audio_ai = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&mp_module_audio_ai_globals,
};

MP_REGISTER_MODULE(MP_QSTR_esp_audio, mp_module_audio_ai);