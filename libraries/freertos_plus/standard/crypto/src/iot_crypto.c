/*
 * FreeRTOS Crypto V1.1.1
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "iot_crypto.h"

/* mbedTLS includes. */

#if !defined( MBEDTLS_CONFIG_FILE )
    #include "mbedtls/config.h"
#else
    #include MBEDTLS_CONFIG_FILE
#endif

#include "mbedtls/platform.h"
#include "mbedtls/sha256.h"
#include "mbedtls/sha1.h"
//#include "mbedtls/pk.h"
//#include "mbedtls/x509_crt.h"
/* Threading mutex implementations for mbedTLS. */
#include "mbedtls/threading.h"
#include "threading_alt.h"

/* C runtime includes. */
#include <string.h>



#define CRYPTO_PRINT( X )    configPRINTF( X )

/**
 * @brief Internal signature verification context structure
 */
typedef struct SignatureVerificationState
{
    BaseType_t xAsymmetricAlgorithm;
    BaseType_t xHashAlgorithm;
    mbedtls_sha1_context xSHA1Context;
    mbedtls_sha256_context xSHA256Context;
} SignatureVerificationState_t, * SignatureVerificationStatePtr_t;

/*-----------------------------------------------------------*/
/*------ Helper functions for FreeRTOS heap management ------*/
/*-----------------------------------------------------------*/

/* If mbedTLS is using AFR managed memory, it needs access to an implementation of calloc. */
#ifdef CONFIG_MEDTLS_USE_AFR_MEMORY

/**
 * @brief Implements libc calloc semantics using the FreeRTOS heap
 */
    void * pvCalloc( size_t xNumElements,
                     size_t xSize )
    {
        void * pvNew = pvPortMalloc( xNumElements * xSize );

        if( NULL != pvNew )
        {
            memset( pvNew, 0, xNumElements * xSize );
        }

        return pvNew;
    }
#endif /* ifdef CONFIG_MEDTLS_USE_AFR_MEMORY */

/*-----------------------------------------------------------*/
/*--------- mbedTLS threading functions for FreeRTOS --------*/
/*--------------- See MBEDTLS_THREADING_ALT -----------------*/
/*-----------------------------------------------------------*/

/**
 * @brief Implementation of mbedtls_mutex_init for thread-safety.
 *
 */
void aws_mbedtls_mutex_init( mbedtls_threading_mutex_t * mutex )
{
    mutex->mutex = xSemaphoreCreateMutex();

    if( mutex->mutex != NULL )
    {
        mutex->is_valid = 1;
    }
    else
    {
        mutex->is_valid = 0;
        CRYPTO_PRINT( ( "Failed to initialize mbedTLS mutex.\r\n" ) );
    }
}

/**
 * @brief Implementation of mbedtls_mutex_free for thread-safety.
 *
 */
void aws_mbedtls_mutex_free( mbedtls_threading_mutex_t * mutex )
{
    if( mutex->is_valid == 1 )
    {
        vSemaphoreDelete( mutex->mutex );
        mutex->is_valid = 0;
    }
}

/**
 * @brief Implementation of mbedtls_mutex_lock for thread-safety.
 *
 * @return 0 if successful, MBEDTLS_ERR_THREADING_MUTEX_ERROR if timeout,
 * MBEDTLS_ERR_THREADING_BAD_INPUT_DATA if the mutex is not valid.
 */
int aws_mbedtls_mutex_lock( mbedtls_threading_mutex_t * mutex )
{
    int ret = MBEDTLS_ERR_THREADING_BAD_INPUT_DATA;

    if( mutex->is_valid == 1 )
    {
        if( xSemaphoreTake( mutex->mutex, portMAX_DELAY ) )
        {
            ret = 0;
        }
        else
        {
            ret = MBEDTLS_ERR_THREADING_MUTEX_ERROR;
            CRYPTO_PRINT( ( "Failed to obtain mbedTLS mutex.\r\n" ) );
        }
    }

    return ret;
}

/**
 * @brief Implementation of mbedtls_mutex_unlock for thread-safety.
 *
 * @return 0 if successful, MBEDTLS_ERR_THREADING_MUTEX_ERROR if timeout,
 * MBEDTLS_ERR_THREADING_BAD_INPUT_DATA if the mutex is not valid.
 */
int aws_mbedtls_mutex_unlock( mbedtls_threading_mutex_t * mutex )
{
    int ret = MBEDTLS_ERR_THREADING_BAD_INPUT_DATA;

    if( mutex->is_valid == 1 )
    {
        if( xSemaphoreGive( mutex->mutex ) )
        {
            ret = 0;
        }
        else
        {
            ret = MBEDTLS_ERR_THREADING_MUTEX_ERROR;
            CRYPTO_PRINT( ( "Failed to unlock mbedTLS mutex.\r\n" ) );
        }
    }

    return ret;
}


/*
 * Interface routines
 */

void CRYPTO_Init( void )
{
    CRYPTO_ConfigureThreading();
}

void CRYPTO_ConfigureThreading( void )
{
    /* Configure mbedtls to use FreeRTOS mutexes. */
    mbedtls_threading_set_alt( aws_mbedtls_mutex_init,
                               aws_mbedtls_mutex_free,
                               aws_mbedtls_mutex_lock,
                               aws_mbedtls_mutex_unlock );
}

