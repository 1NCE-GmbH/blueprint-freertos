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

#ifndef __AWS_CRYPTO__H__
#define __AWS_CRYPTO__H__

#include "FreeRTOS.h"

/**
 * @brief Commonly used buffer sizes for storing cryptographic hash computation
 * results.
 */
#define cryptoSHA1_DIGEST_BYTES      20
#define cryptoSHA256_DIGEST_BYTES    32

/**
 * @brief Initializes the heap and threading functions for cryptography libraries.
 */
void CRYPTO_Init( void );

/**
 * @brief Initializes the mbedTLS mutex functions.
 *
 * Provides mbedTLS access to mutex create, destroy, take and free.
 *
 * @see MBEDTLS_THREADING_ALT
 */
void CRYPTO_ConfigureThreading( void );

/**
 * @brief Library-independent cryptographic algorithm identifiers.
 */
#define cryptoHASH_ALGORITHM_SHA1           1
#define cryptoHASH_ALGORITHM_SHA256         2
#define cryptoASYMMETRIC_ALGORITHM_RSA      1
#define cryptoASYMMETRIC_ALGORITHM_ECDSA    2


#endif /* ifndef __AWS_CRYPTO__H__ */
