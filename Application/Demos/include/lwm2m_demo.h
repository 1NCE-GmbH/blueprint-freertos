/*
 *  lwm2m_demo.h
 *
 *  Created on: Sep 6, 2024
 *  Authors: Mohammed Abdelmaksoud & Hatim Jamali
 *  1NCE GmbH
 */

#ifndef LWM2M_DEMO_H
#define LWM2M_DEMO_H

#ifdef __cplusplus
    extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/


/**
 * @brief LwM2MDemo function initializes and starts an LwM2M client.
 *
 * This function sets up an LwM2M client by initializing required security, server, device,
 * and firmware objects. It handles communications with an LwM2M server, including registering,
 * bootstrapping, and sending periodic updates.
 *
 * The client runs in a loop, processing incoming requests and managing its state.
 *
 * @note Depending on configuration, the client supports both DTLS-enabled and non-DTLS communications.
 */
void LwM2MDemo( void );


#ifdef __cplusplus
    }
#endif
#endif /* ifndef LWM2M_DEMO_H */
