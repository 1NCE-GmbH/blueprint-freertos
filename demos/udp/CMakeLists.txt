# C SDK UDP demo
afr_demo_module(udp)

afr_set_demo_metadata(ID "UDP_DEMO")
afr_set_demo_metadata(DESCRIPTION "An example that demonstrates UDP")
afr_set_demo_metadata(DISPLAY_NAME "UDP Hello World")

afr_module_sources(
    ${AFR_CURRENT_MODULE}
    INTERFACE
        "${CMAKE_CURRENT_LIST_DIR}/iot_demo_udp.c"
)
afr_module_dependencies(
    ${AFR_CURRENT_MODULE}
    INTERFACE
        AFR::udp
)



