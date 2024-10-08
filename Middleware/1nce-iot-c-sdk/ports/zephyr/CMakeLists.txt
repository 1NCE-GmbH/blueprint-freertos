#
# Copyright (c) 2022 1NCE
# 1NCE IoT C SDK (ZEPHYR OS Module)
#


set(NCE_SDK_ROOT ../..)

# set(NCE_SDK_DIR ${ZEPHYR_CURRENT_MODULE_DIR})

zephyr_library()
zephyr_include_directories(${NCE_SDK_ROOT}/source/include)
zephyr_include_directories(${NCE_SDK_ROOT}/source/interface)
zephyr_library_sources(
	${NCE_SDK_ROOT}/source/nce_iot_c_sdk.c
)

zephyr_compile_definitions_ifdef(CONFIG_NCE_DEVICE_AUTHENTICATOR NCE_DEVICE_AUTHENTICATOR)
zephyr_compile_definitions_ifdef(CONFIG_NCE_ENERGY_SAVER NCE_ENERGY_SAVER)
zephyr_compile_definitions(
    NCE_SDK_ATTEMPTS=${CONFIG_NCE_SDK_ATTEMPTS})
zephyr_compile_definitions(
    NCE_SDK_MAX_STRING_SIZE=${CONFIG_NCE_SDK_MAX_STRING_SIZE})


zephyr_library_sources_ifdef(CONFIG_NCE_SDK_UDP_INTERFACE udp_interface_zephyr.c)
zephyr_include_directories(include)
