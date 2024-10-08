/*
 *  udp_demo.h
 *
 *  Created on: Sep 6, 2024
 *  Authors: Mohammed Abdelmaksoud & Hatim Jamali
 *  1NCE GmbH
 */

#ifndef UDP_DEMO_H
#define UDP_DEMO_H

#ifdef __cplusplus
    extern "C" {
#endif

/**
 * @brief Demo function to initialize and manage UDP communication tasks.
 *
 * This function sets up a FreeRTOS queue and two tasks for UDP communication:
 * one for sending UDP messages (`SendUDPData`) and another for listening
 * and receiving UDP messages (`ListenUDPData`). The function creates the
 * necessary queue and tasks, handles error checks, and logs status updates.
 * Once tasks are created successfully, it enters an infinite loop to keep
 * the tasks running.
 */
void UdpDemo( void );


#ifdef __cplusplus
    }
#endif
#endif /* ifndef UDP_DEMO_H */
