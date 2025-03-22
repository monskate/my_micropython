# Create an INTERFACE library for our C module.
add_library(usermod_models INTERFACE)

# Add our source files to the lib
target_sources(usermod_models INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/wu_20_model_data.cpp
    ${CMAKE_CURRENT_LIST_DIR}/lubiao_model_data.cpp
    ${CMAKE_CURRENT_LIST_DIR}/face_model_data.cpp
    ${CMAKE_CURRENT_LIST_DIR}/models.c

)

# Add the current directory as an include directory.
target_include_directories(usermod_models INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    ${IDF_PATH}/components/bus
    ${IDF_PATH}/components/bus/include
    ${IDF_PATH}/components/esp32-camera
    ${IDF_PATH}/components/esp32-camera/driver/include
    ${IDF_PATH}/components/esp32-camera/driver/private_include
    ${IDF_PATH}/components/esp32-camera/conversions/include
    ${IDF_PATH}/components/esp32-camera/conversions/private_include
    ${IDF_PATH}/components/esp32-camera/sensors/private_include
    ${IDF_PATH}/components/esp-nn
    ${IDF_PATH}/components/esp-nn/include
    ${IDF_PATH}/components/esp-tflite-micro
    ${IDF_PATH}/components/esp-tflite-micro/signal/micro/kernels
    ${IDF_PATH}/components/esp-tflite-micro/signal/src 
    ${IDF_PATH}/components/esp-tflite-micro/signal/src/kiss_fft_wrappers
    ${IDF_PATH}/components/esp-tflite-micro/tensorflow/lite
    ${IDF_PATH}/components/esp-tflite-micro/tensorflow/lite/c 
    ${IDF_PATH}/components/esp-tflite-micro/tensorflow/lite/core 
    ${IDF_PATH}/components/esp-tflite-micro/tensorflow/lite/core/c
    ${IDF_PATH}/components/esp-tflite-micro/tensorflow/lite/core/api
    ${IDF_PATH}/components/esp-tflite-micro/tensorflow/lite/core
    ${IDF_PATH}/components/esp-tflite-micro/tensorflow/lite/experimental/microfrontend/lib
    ${IDF_PATH}/components/esp-tflite-micro/tensorflow/lite/kernels
    ${IDF_PATH}/components/esp-tflite-micro/tensorflow/lite/kernels/internal
    ${IDF_PATH}/components/esp-tflite-micro/tensorflow/lite/kernels/internal/optimized
    ${IDF_PATH}/components/esp-tflite-micro/tensorflow/lite/kernels/internal/reference
    ${IDF_PATH}/components/esp-tflite-micro/tensorflow/lite/kernels/internal/reference/integer_ops
    ${IDF_PATH}/components/esp-tflite-micro/tensorflow/lite/micro
    ${IDF_PATH}/components/esp-tflite-micro/tensorflow/lite/micro/arena_allocator
    ${IDF_PATH}/components/esp-tflite-micro/tensorflow/lite/micro/kernels
    ${IDF_PATH}/components/esp-tflite-micro/tensorflow/lite/micro/kernels/esp_nn
    ${IDF_PATH}/components/esp-tflite-micro/tensorflow/lite/micro/memory_planner
    ${IDF_PATH}/components/esp-tflite-micro/tensorflow/lite/micro/tflite_bridge
    ${IDF_PATH}/components/esp-tflite-micro/tensorflow/lite/schema
    ${IDF_PATH}/components/esp-tflite-micro/third_party/flatbuffers/include
    ${IDF_PATH}/components/esp-tflite-micro/third_party/gemmlowp/fixedpoint
    ${IDF_PATH}/components/esp-tflite-micro/third_party/gemmlowp/internal
    ${IDF_PATH}/components/esp-tflite-micro/third_party/kissfft
    ${IDF_PATH}/components/esp-tflite-micro/third_party/kissfft/tools
    ${IDF_PATH}/components/esp-tflite-micro/third_party/ruy/ruy/profiler
    ${IDF_PATH}/components/mdns/include
    ${IDF_PATH}/components/mdns/private_include
    ${IDF_PATH}/components/sscma-micro
    ${IDF_PATH}/components/sscma-micro/porting
    ${IDF_PATH}/components/sscma-micro/porting/espressif
    ${IDF_PATH}/components/sscma-micro/porting/espressif/boards/seeed_xiao_esp32s3
    ${IDF_PATH}/components/sscma-micro/sscma
    ${IDF_PATH}/components/sscma-micro/sscma/callback
    ${IDF_PATH}/components/sscma-micro/sscma/callback/extension
    ${IDF_PATH}/components/sscma-micro/sscma/interface
    ${IDF_PATH}/components/sscma-micro/sscma/interface/transport
    ${IDF_PATH}/components/sscma-micro/sscma/interpreter
    ${IDF_PATH}/components/sscma-micro/sscma/repl
    ${IDF_PATH}/components/sscma-micro/third_party/FlashDB
    ${IDF_PATH}/components/sscma-micro/third_party/JPEGENC
)



target_compile_definitions(usermod_models INTERFACE)

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_models)