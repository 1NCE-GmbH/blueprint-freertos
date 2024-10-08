# CMakeLists.txt
#
# Created on: Sep 6, 2024
# Authors: Mohammed Abdelmaksoud & Hatim Jamali
# 1NCE GmbH


cmake_minimum_required(VERSION 3.22)
project(stm32cubemx)

# Define the stm32cubemx interface library
add_library(stm32cubemx INTERFACE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Compile definitions for the stm32cubemx interface library
target_compile_definitions(stm32cubemx INTERFACE 
    USE_HAL_DRIVER 
    MBEDTLS_CONFIG_FILE="mbedtls_dtls_config.h"
    STM32L496xx
    $<$<CONFIG:Debug>:DEBUG>
)

# Include directories for the stm32cubemx interface library
target_include_directories(stm32cubemx INTERFACE
    # Core and Driver Includes
    ../../Core/Inc
    ../../Drivers/STM32L4xx_HAL_Driver/Inc
    ../../Drivers/CMSIS/Device/ST/STM32L4xx/Include
    ../../Drivers/CMSIS/Include

    # FreeRTOS Includes
    ../../FreeRTOS/Source/include
    ../../FreeRTOS/CMSIS/RTOS2/Include
    ../../FreeRTOS/CMSIS/RTOS2/FreeRTOS/Include
    ../../FreeRTOS/Source/portable/GCC/ARM_CM4F

    # Middleware Includes
    ../../Middleware/freertos/abstractions/platform/include
    ../../Middleware/freertos/abstractions/platform/freertos/include
    ../../Middleware/freertos/abstractions/common_io/include
    ../../Middleware/freertos/c_sdk/standard/common/include/
    ../../Middleware/freertos/c_sdk/standard/common/include/private
    ../../Middleware/freertos/c_sdk/standard/common/include/types
    ../../Middleware/freertos/abstractions/secure_sockets/include
    ../../Middleware/freertos/freertos-plus/standard/utils/include
    ../../Cellular/CellularBG96/source
    ../../Cellular/CellularDemo/source/cellular
    ../../Cellular/CellularLibrary/source/include
    ../../Cellular/CellularLibrary/source/include/common
    ../../Cellular/CellularLibrary/source/include/private
    ../../Cellular/CellularLibrary/source/interface
    ../../Middleware/freertos/freertos-plus/standard/tls/include
    ../../Middleware/freertos/freertos-plus/standard/crypto/include
    ../../Middleware/wakaama/coap/er-coap-13
    ../../Middleware/wakaama/include
    ../../Middleware/mbedtls/include
    ../../Middleware/mbedtls/include/mbedtls
    ../../Middleware/mbedtls_utils
    ../../Middleware/mbedtls_config
    ../../Middleware/1nce_impl
    ../../Middleware/1nce-iot-c-sdk/source/interface
    ../../Middleware/1nce-iot-c-sdk/source/include
    ../../Middleware/wakaama/examples/shared
    ../../Middleware/wakaama/examples/client
    ../../Middleware/wakaama/core

    # Application Includes
    ../../Application/Config
    ../../Application/Demos/include
)

# Source files for the stm32cubemx interface library
target_sources(stm32cubemx INTERFACE
    # Core Sources
    ../../Core/Src/main.c
    ../../Core/Src/gpio.c
    ../../Core/Src/freertos.c
    ../../Core/Src/rng.c
    ../../Core/Src/rtc.c
    ../../Core/Src/usart.c
    ../../Core/Src/stm32l4xx_it.c
    ../../Core/Src/stm32l4xx_hal_msp.c
    ../../Core/Src/stm32l4xx_hal_timebase_tim.c
    ../../Core/Src/system_stm32l4xx.c
    ../../Core/Src/sysmem.c
    ../../Core/Src/syscalls.c
    ../../Core/Src/time.c

    # Driver Sources
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ramfunc.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gpio.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_exti.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rng.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim_ex.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart.c
    ../../Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart_ex.c

    # FreeRTOS Sources
    ../../FreeRTOS/Source/croutine.c
    ../../FreeRTOS/Source/event_groups.c
    ../../FreeRTOS/Source/list.c
    ../../FreeRTOS/Source/queue.c
    ../../FreeRTOS/Source/stream_buffer.c
    ../../FreeRTOS/Source/tasks.c
    ../../FreeRTOS/Source/timers.c
    ../../FreeRTOS/CMSIS/RTOS2/FreeRTOS/Source/cmsis_os2.c
    ../../FreeRTOS/Source/portable/MemMang/heap_5.c
    ../../FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c

    # Middleware Sources - FreeRTOS
    ../../Middleware/freertos/abstractions/platform/freertos/iot_clock_freertos.c
    ../../Middleware/freertos/abstractions/platform/freertos/iot_metrics.c
    ../../Middleware/freertos/abstractions/platform/freertos/iot_network_freertos.c
    ../../Middleware/freertos/abstractions/platform/freertos/iot_threads_freertos.c
    ../../Middleware/freertos/c_sdk/standard/common/logging/iot_logging.c
    ../../Middleware/freertos/c_sdk/standard/common/logging/iot_logging_task_dynamic_buffers.c
    ../../Middleware/freertos/freertos-plus/standard/utils/src/iot_system_init.c

    # Middleware Sources - Cellular
    ../../Cellular/iot_secure_sockets.c
    ../../Cellular/CellularBG96/source/cellular_bg96_api.c
    ../../Cellular/CellularBG96/source/cellular_bg96_urc_handler.c
    ../../Cellular/CellularBG96/source/cellular_bg96_wrapper.c
    ../../Cellular/CellularBG96/source/cellular_bg96.c
    ../../Cellular/CellularDemo/source/cellular_setup.c
    ../../Cellular/CellularDemo/source/cellular/cellular_platform.c
    ../../Cellular/CellularDemo/source/cellular/comm_if_st.c
    ../../Cellular/CellularDemo/source/cellular/device_control.c
    ../../Cellular/CellularDemo/source/cellular/iot_fifo.c
    ../../Cellular/CellularLibrary/source/cellular_3gpp_api.c
    ../../Cellular/CellularLibrary/source/cellular_3gpp_urc_handler.c
    ../../Cellular/CellularLibrary/source/cellular_at_core.c
    ../../Cellular/CellularLibrary/source/cellular_common_api.c
    ../../Cellular/CellularLibrary/source/cellular_common.c
    ../../Cellular/CellularLibrary/source/cellular_pkthandler.c
    ../../Cellular/CellularLibrary/source/cellular_pktio.c

    # Middleware Sources - Wakaama
    ../../Middleware/wakaama/coap/er-coap-13/er-coap-13.c
    ../../Middleware/wakaama/examples/shared/platform.c
    ../../Middleware/wakaama/examples/shared/connection.c
    ../../Middleware/wakaama/examples/shared/dtlsconnection.c
    ../../Middleware/wakaama/examples/shared/memtrace.c
    ../../Middleware/wakaama/examples/client/lwm2mclient.c
    ../../Middleware/wakaama/examples/client/object_server.c
    ../../Middleware/wakaama/examples/client/object_firmware.c
    ../../Middleware/wakaama/examples/client/object_location.c
    ../../Middleware/wakaama/examples/client/object_device.c
    ../../Middleware/wakaama/examples/client/object_connectivity_moni.c
    ../../Middleware/wakaama/examples/client/object_connectivity_stat.c
    ../../Middleware/wakaama/examples/client/object_access_control.c
    ../../Middleware/wakaama/examples/client/object_test.c
    ../../Middleware/wakaama/examples/client/system_api.c
    ../../Middleware/wakaama/examples/client/object_security.c
    ../../Middleware/wakaama/core/objects.c
    ../../Middleware/wakaama/core/liblwm2m.c
    ../../Middleware/wakaama/core/utils.c
    ../../Middleware/wakaama/core/uri.c
    ../../Middleware/wakaama/core/packet.c
    ../../Middleware/wakaama/core/list.c
    ../../Middleware/wakaama/core/observe.c
    ../../Middleware/wakaama/core/bootstrap.c
    ../../Middleware/wakaama/core/registration.c
    ../../Middleware/wakaama/coap/block.c
    ../../Middleware/wakaama/coap/transaction.c
    ../../Middleware/wakaama/data/data.c
    ../../Middleware/wakaama/data/tlv.c
    ../../Middleware/wakaama/data/senml_json.c
    ../../Middleware/wakaama/data/json.c
    ../../Middleware/wakaama/data/json_common.c
    ../../Middleware/wakaama/core/discover.c
    ../../Middleware/wakaama/core/management.c


    # Middleware Sources - mbedTLS
    ../../Middleware/mbedtls/library/aes.c
    ../../Middleware/mbedtls/library/aesni.c
    ../../Middleware/mbedtls/library/ctr_drbg.c
    ../../Middleware/mbedtls/library/cipher.c
    ../../Middleware/mbedtls/library/ssl_tls.c
    ../../Middleware/mbedtls/library/ssl_cli.c
    ../../Middleware/mbedtls/library/ssl_srv.c
    ../../Middleware/mbedtls/library/x509_crt.c
    ../../Middleware/mbedtls/library/x509.c
    ../../Middleware/mbedtls/library/pkcs5.c
    ../../Middleware/mbedtls/library/pkcs12.c
    ../../Middleware/mbedtls/library/x509_crl.c
    ../../Middleware/mbedtls/library/ecp.c
    ../../Middleware/mbedtls/library/ecdsa.c
    ../../Middleware/mbedtls/library/ecjpake.c
    ../../Middleware/mbedtls/library/ecdh.c
    ../../Middleware/mbedtls/library/pk.c
    ../../Middleware/mbedtls/library/pkparse.c
    ../../Middleware/mbedtls/library/rsa.c
    ../../Middleware/mbedtls/library/rsa_internal.c
    ../../Middleware/mbedtls/library/pk_wrap.c
    ../../Middleware/mbedtls/library/pkcs11.c
    ../../Middleware/mbedtls/library/platform_util.c
    ../../Middleware/mbedtls_utils/mbedtls_error.c
    ../../Middleware/mbedtls/library/md.c
    ../../Middleware/mbedtls/library/md_wrap.c
    ../../Middleware/mbedtls/library/md2.c
    ../../Middleware/mbedtls/library/md4.c
    ../../Middleware/mbedtls/library/md5.c
    ../../Middleware/mbedtls/library/sha1.c
    ../../Middleware/mbedtls/library/sha256.c
    ../../Middleware/mbedtls/library/sha512.c
    ../../Middleware/mbedtls/library/base64.c
    ../../Middleware/mbedtls/library/pem.c
    ../../Middleware/mbedtls/library/des.c
    ../../Middleware/mbedtls/library/bignum.c
    ../../Middleware/mbedtls/library/havege.c
    ../../Middleware/mbedtls/library/timing.c
    ../../Middleware/mbedtls/library/timing_alt.c
    ../../Middleware/mbedtls/library/version.c
    ../../Middleware/mbedtls/library/threading.c
    ../../Middleware/mbedtls/library/oid.c
    ../../Middleware/mbedtls/library/gcm.c
    ../../Middleware/mbedtls/library/ssl_ciphersuites.c
    ../../Middleware/mbedtls/library/cipher_wrap.c
    ../../Middleware/mbedtls/library/threading.c

    # Middleware Sources - FreeRTOS TLS
    ../../Middleware/freertos/freertos-plus/standard/tls/src/iot_tls.c

    # Middleware Sources - FreeRTOS Crypto
    ../../Middleware/freertos/freertos-plus/standard/crypto/src/iot_crypto.c

    # Middleware Sources - 1NCE IoT SDK
    ../../Middleware/1nce_impl/udp_impl.c
    ../../Middleware/1nce-iot-c-sdk/source/nce_iot_c_sdk.c

    # Application Sources
    ../../Application/Demos/source/udp_demo.c
    ../../Application/cellular_app.c
    ../../Application/Demos/source/coap_demo.c
    ../../Application/Demos/source/lwm2m_demo.c

    # Startup Sources
    ../../startup_stm32l496xx.s

    # Commented sources for potential future use
    # ../../Middleware/Ipc/Src/ipc_common.c
    # ../../Middleware/Ipc/Src/ipc_uart.c
    # ../../Cellular_demo/source/cellular/comm_if_st.c
    # ../../Cellular_demo/source/cellular/iot_fifo.c
    # ../../Cellular/source/cellular_3gpp_api.c
    # ../../Cellular/source/cellular_3gpp_urc_handler.c
    # ../../Cellular/source/cellular_at_core.c
    # ../../Cellular/source/cellular_common_api.c
    # ../../Cellular/source/cellular_common.c
    # ../../Cellular/source/cellular_pkthandler.c
    # ../../Cellular/source/cellular_pktio.c
    # ../../Cellular_bg96/source/cellular_bg96_api.c
    # ../../Cellular_bg96/source/cellular_bg96_urc_handler.c
    # ../../Cellular_bg96/source/cellular_bg96_wrapper.c
    # ../../Cellular_bg96/source/cellular_bg96.c
)

# Link directories (none specified)
target_link_directories(stm32cubemx INTERFACE)

# Link libraries (none specified)
target_link_libraries(stm32cubemx INTERFACE)

# Validate that STM32CubeMX code is compatible with C standard
if(CMAKE_C_STANDARD LESS 11)
    message(ERROR "Generated code requires C11 or higher")
endif()
