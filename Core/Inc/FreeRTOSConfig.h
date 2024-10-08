/*
 *  FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
 *  All rights reserved
 *
 *  VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.
 *
 *  This file is part of the FreeRTOS distribution.
 *
 *  FreeRTOS is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License (version 2) as published by the
 *  Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.
 *
 ***************************************************************************
 *  >>!   NOTE: The modification to the GPL is included to allow you to     !<<
 *  >>!   distribute a combined work that includes FreeRTOS without being   !<<
 *  >>!   obliged to provide the source code for proprietary components     !<<
 *  >>!   outside of the FreeRTOS kernel.                                   !<<
 ***************************************************************************
 *
 *  FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  Full license text is available on the following
 *  link: http://www.freertos.org/a00114.html
 *
 ***************************************************************************
 *                                                                       *
 *    FreeRTOS provides completely free yet professionally developed,    *
 *    robust, strictly quality controlled, supported, and cross          *
 *    platform software that is more than just the market leader, it     *
 *    is the industry's de facto standard.                               *
 *                                                                       *
 *    Help yourself get started quickly while simultaneously helping     *
 *    to support the FreeRTOS project by purchasing a FreeRTOS           *
 *    tutorial book, reference manual, or both:                          *
 *    http://www.FreeRTOS.org/Documentation                              *
 *                                                                       *
 ***************************************************************************
 *
 *  http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
 * the FAQ page "My application does not run, what could be wrong?".  Have you
 * defined configASSERT()?
 *
 * http://www.FreeRTOS.org/support - In return for receiving this top quality
 * embedded software for free we request you assist our global community by
 * participating in the support forum.
 *
 * http://www.FreeRTOS.org/training - Investing in training allows your team to
 * be as productive as possible as early as possible.  Now you can receive
 * FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
 * Ltd, and the world's leading authority on the world's leading RTOS.
 *
 *  http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
 *  including FreeRTOS+Trace - an indispensable productivity tool, a DOS
 *  compatible FAT file system, and our tiny thread aware UDP/IP stack.
 *
 *  http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
 *  Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.
 *
 *  http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
 *  Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
 *  licenses offer ticketed support, indemnification and commercial middleware.
 *
 *  http://www.SafeRTOS.com - High Integrity Systems also provide a safety
 *  engineered and independently SIL3 certified version for use in safety and
 *  mission critical applications that require provable dependability.
 *
 *  1 tab == 4 spaces!
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
* Application specific definitions.
*
* These definitions should be adjusted for your particular hardware and
* application requirements.
*
* THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
* FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
*
* See http://www.freertos.org/a00110.html.
*----------------------------------------------------------*/

/* USER CODE BEGIN Includes */
/* Section where include file can be added */
/* USER CODE END Includes */
/* Ensure stdint is only used by the compiler, and not the assembler. */
#if defined( __ICCARM__ ) || defined( __CC_ARM ) || defined( __GNUC__ )
    #include <stdint.h>
    extern uint32_t SystemCoreClock;
#endif /* (__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__) */

#define configSUPPORT_STATIC_ALLOCATION              1


#define configUSE_PREEMPTION                         1
#define configUSE_IDLE_HOOK                          1
#define configUSE_TICK_HOOK                          0
#define configUSE_TICKLESS_IDLE                      0
#define configUSE_DAEMON_TASK_STARTUP_HOOK           1
#define configCPU_CLOCK_HZ                           ( SystemCoreClock )
#define configTICK_RATE_HZ                           ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES                         ( 7 )
#define configMINIMAL_STACK_SIZE                     ( ( uint16_t ) 256 )
#define configTOTAL_HEAP_SIZE                        ( ( size_t ) ( 32 * 1024 ) )
#define configMAX_TASK_NAME_LEN                      ( 16 )
#define configUSE_TRACE_FACILITY                     1
#define configUSE_16_BIT_TICKS                       0
#define configIDLE_SHOULD_YIELD                      1
#define configUSE_MUTEXES                            1
#define configQUEUE_REGISTRY_SIZE                    8
#define configCHECK_FOR_STACK_OVERFLOW               2
#define configUSE_RECURSIVE_MUTEXES                  1
#define configUSE_MALLOC_FAILED_HOOK                 1
#define configUSE_APPLICATION_TASK_TAG               1
#define configUSE_COUNTING_SEMAPHORES                1
#define configGENERATE_RUN_TIME_STATS                0
#define configOVERRIDE_DEFAULT_TICK_CONFIGURATION    1
#define configRECORD_STACK_HIGH_ADDRESS              1
#define configUSE_POSIX_ERRNO                        1
#define configSUPPORT_DYNAMIC_ALLOCATION             1



/* Co-routine definitions. */
#define configUSE_CO_ROUTINES               0
#define configMAX_CO_ROUTINE_PRIORITIES     ( 2 )

/* Software timer definitions. */
#define configUSE_TIMERS                    1
#define configTIMER_TASK_PRIORITY           ( configMAX_PRIORITIES - 2 )
#define configTIMER_QUEUE_LENGTH            10
#define configTIMER_TASK_STACK_DEPTH        ( configMINIMAL_STACK_SIZE * 2 )

/* Set the following definitions to 1 to include the API function, or zero
 * to exclude the API function. */
#define INCLUDE_vTaskPrioritySet            1
#define INCLUDE_uxTaskPriorityGet           1
#define INCLUDE_vTaskDelete                 1
#define INCLUDE_vTaskCleanUpResources       0
#define INCLUDE_vTaskSuspend                1
#define INCLUDE_vTaskDelayUntil             1
#define INCLUDE_vTaskDelay                  1
#define INCLUDE_xTaskGetSchedulerState      1
#define INCLUDE_xSemaphoreGetMutexHolder    1
#define INCLUDE_xTimerPendFunctionCall      1

/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
/* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
    #define configPRIO_BITS    __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS    4
#endif /* __NVIC_PRIO_BITS */

/* The lowest interrupt priority that can be used in a call to a "set priority"
 * function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         0xf

/* The highest interrupt priority that can be used by any interrupt service
 * routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
 * INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
 * PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    1

/* Interrupt priorities used by the kernel port layer itself.  These are generic
* to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY \
        ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )

/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
 * See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
        ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )

/* Normal assert() semantics without relying on the provision of an assert.h
 * header file. */

/* The function that implements FreeRTOS printf style output, and the macro
 * that maps the configPRINTF() macros to that function. */
void vLoggingPrintf( char const * pcFormat,
                     ... );

/* Logging task definitions. */
extern void vMainUARTPrintString( char * pcString );
void vLoggingPrintf( const char * pcFormat,
                     ... );

/* Map the FreeRTOS printf() to the logging task printf. */
#define configPRINTF( X )    vLoggingPrintf X

/* Non-format version print. */
extern void vLoggingPrint( const char * pcMessage );
#define configPRINT( X )           vLoggingPrint( X )

/* Map the logging task's printf to the board specific output function. */
#define configPRINT_STRING( x )    vMainUARTPrintString( x );

/* Sets the length of the buffers into which logging messages are written - so
 * also defines the maximum length of each log message. */
#define configLOGGING_MAX_MESSAGE_LENGTH            128

/* Set to 1 to prepend each log message with a message number, the task name,
 * and a time stamp. */
#define configLOGGING_INCLUDE_TIME_AND_TASK_NAME    1

/* Pseudo random number generator, just used by demos so does not have to be
 * secure.  Do not use the standard C library rand() function as it can cause
 * unexpected behaviour, such as calls to malloc(). */
extern int iMainRand32( void );
#define configRAND32()    iMainRand32()

/* Demo specific macros that allow the application writer to insert code to be
 * executed immediately before the MCU's STOP low power mode is entered and exited
 * respectively.  These macros are in addition to the standard
 * configPRE_SLEEP_PROCESSING() and configPOST_SLEEP_PROCESSING() macros, which are
 * called pre and post the low power SLEEP mode being entered and exited.  These
 * macros can be used to turn turn off and on IO, clocks, the Flash etc. to obtain
 * the lowest power possible while the tick is off. */
#if defined( __ICCARM__ ) || defined( __CC_ARM ) || defined( __GNUC__ )
    void vMainPreStopProcessing( void );
    void vMainPostStopProcessing( void );
#endif /* defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__) */

#define configPRE_STOP_PROCESSING     vMainPreStopProcessing
#define configPOST_STOP_PROCESSING    vMainPostStopProcessing

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
 * standard names. */
#define vPortSVCHandler               SVC_Handler
#define xPortPendSVHandler            PendSV_Handler
#define vHardFault_Handler            HardFault_Handler

/* IMPORTANT: This define MUST be commented when used with STM32Cube firmware,
 *            to prevent overwriting SysTick_Handler defined within STM32Cube HAL */
/* #define xPortSysTickHandler SysTick_Handler */
/* The platform FreeRTOS is running on. */
#define configPLATFORM_NAME    "STM32L49"

#endif /* FREERTOS_CONFIG_H */
