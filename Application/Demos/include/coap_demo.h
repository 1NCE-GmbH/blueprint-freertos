/*
 *  coap_demo.h
 *
 *  Created on: Sep 6, 2024
 *  Authors: Mohammed Abdelmaksoud & Hatim Jamali
 *  1NCE GmbH
 */

#ifndef COAP_DEMO_H
#define COAP_DEMO_H

#ifdef __cplusplus
    extern "C" {
#endif

/**
 * @brief Demo function to initialize and manage CoAP communication tasks.
 *
 * This function sets up a FreeRTOS queue and two tasks for CoAP communication:
 * one for sending CoAP messages (`SendCoAPData`) and another for listening
 * and receiving CoAP messages (`ListenCoAPData`). The function creates the
 * necessary queue and tasks, handles error checks, and logs status updates.
 * Once tasks are created successfully, it enters an infinite loop to keep
 * the tasks running.
 */
void CoAPDemo( void );


#ifdef __cplusplus
    }
#endif
#endif /* ifndef COAP_DEMO_H */
