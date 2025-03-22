# Create an INTERFACE library for our C module.
add_library(usermod_esp32audio INTERFACE)

# Add our source files to the lib
target_sources(usermod_esp32audio INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/modaudio.c
)

# Add the current directory as an include directory.
target_include_directories(usermod_esp32audio INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    ${IDF_PATH}/components/json/cJSON
    ${IDF_PATH}/components/hardware_driver/include
    ${IDF_PATH}/components/hardware_driver/boards/include
    ${IDF_PATH}/components/perf_tester
    ${IDF_PATH}/components/player
    ${IDF_PATH}/components/player/esp_tts_wav
    ${IDF_PATH}/components/sr_ringbuf
    ${IDF_PATH}/components/esp-sr/include/esp32s3
    ${IDF_PATH}/components/esp-sr/src/include
    ${IDF_PATH}/components/esp_codec_dev/include
    ${IDF_PATH}/components/esp_codec_dev/device/es8311
    ${IDF_PATH}/components/esp_codec_dev/device/es7210
    ${IDF_PATH}/components/esp_http_client/include
)

target_compile_definitions(usermod_esp32audio INTERFACE)

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_esp32audio)