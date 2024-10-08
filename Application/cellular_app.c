/*
 * cellular_app.c
 *
 *  Created on: Sep 6, 2024
 *  Authors: Mohammed Abdelmaksoud & Hatim Jamali
 *  1NCE GmbH
 */

#include "cellular_app.h"
#include "nce_demo_config.h"

/**
 * @brief Runs the appropriate demo task based on the configuration flags.
 *
 * This function runs one of the following demos depending on which configuration flag is enabled:
 *
 * - If CONFIG_UDP_DEMO_ENABLED is defined, the UdpDemo() function is executed.
 * - If CONFIG_COAP_DEMO_ENABLED is defined, the CoAPDemo() function is executed.
 * - If CONFIG_LwM2M_DEMO_ENABLED is defined, the LwM2MDemo() function is executed.
 *
 * The task selection is controlled by preprocessor flags, ensuring only one demo is run depending on the build configuration in nce_demo_config.h.
 */
void RunDemoTask( void )
{
    #ifdef CONFIG_UDP_DEMO_ENABLED
        UdpDemo();
    #endif /* CONFIG_UDP_DEMO_ENABLED */

    #ifdef CONFIG_COAP_DEMO_ENABLED
        CoAPDemo();
    #endif /* CONFIG_COAP_DEMO_ENABLED */

    #ifdef CONFIG_LwM2M_DEMO_ENABLED
        LwM2MDemo();
    #endif /* CONFIG_LwM2M_DEMO_ENABLED */
}
