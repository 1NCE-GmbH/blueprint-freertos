/**
 * \file compat-1.3.h
 *
 * \brief Compatibility definitions for using mbed TLS with client code written
 *  for the PolarSSL naming conventions.
 *
 * \deprecated Use the new names directly instead
 */
/*
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later
 *
 *  This file is provided under the Apache License 2.0, or the
 *  GNU General Public License v2.0 or later.
 *
 *  **********
 *  Apache License 2.0:
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  **********
 *
 *  **********
 *  GNU General Public License v2.0 or later:
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  **********
 */

#if !defined( MBEDTLS_CONFIG_FILE )
    #include "config.h"
#else
    #include MBEDTLS_CONFIG_FILE
#endif

#if !defined( MBEDTLS_DEPRECATED_REMOVED )

    #if defined( MBEDTLS_DEPRECATED_WARNING )
        #warning "Including compat-1.3.h is deprecated"
    #endif

    #ifndef MBEDTLS_COMPAT13_H
        #define MBEDTLS_COMPAT13_H

/*
 * config.h options
 */
        #if defined MBEDTLS_AESNI_C
            #define POLARSSL_AESNI_C                                      MBEDTLS_AESNI_C
        #endif
        #if defined MBEDTLS_AES_ALT
            #define POLARSSL_AES_ALT                                      MBEDTLS_AES_ALT
        #endif
        #if defined MBEDTLS_AES_C
            #define POLARSSL_AES_C                                        MBEDTLS_AES_C
        #endif
        #if defined MBEDTLS_AES_ROM_TABLES
            #define POLARSSL_AES_ROM_TABLES                               MBEDTLS_AES_ROM_TABLES
        #endif
        #if defined MBEDTLS_ARC4_ALT
            #define POLARSSL_ARC4_ALT                                     MBEDTLS_ARC4_ALT
        #endif
        #if defined MBEDTLS_ARC4_C
            #define POLARSSL_ARC4_C                                       MBEDTLS_ARC4_C
        #endif
        #if defined MBEDTLS_ASN1_PARSE_C
            #define POLARSSL_ASN1_PARSE_C                                 MBEDTLS_ASN1_PARSE_C
        #endif
        #if defined MBEDTLS_ASN1_WRITE_C
            #define POLARSSL_ASN1_WRITE_C                                 MBEDTLS_ASN1_WRITE_C
        #endif
        #if defined MBEDTLS_BASE64_C
            #define POLARSSL_BASE64_C                                     MBEDTLS_BASE64_C
        #endif
        #if defined MBEDTLS_BIGNUM_C
            #define POLARSSL_BIGNUM_C                                     MBEDTLS_BIGNUM_C
        #endif
        #if defined MBEDTLS_BLOWFISH_ALT
            #define POLARSSL_BLOWFISH_ALT                                 MBEDTLS_BLOWFISH_ALT
        #endif
        #if defined MBEDTLS_BLOWFISH_C
            #define POLARSSL_BLOWFISH_C                                   MBEDTLS_BLOWFISH_C
        #endif
        #if defined MBEDTLS_CAMELLIA_ALT
            #define POLARSSL_CAMELLIA_ALT                                 MBEDTLS_CAMELLIA_ALT
        #endif
        #if defined MBEDTLS_CAMELLIA_C
            #define POLARSSL_CAMELLIA_C                                   MBEDTLS_CAMELLIA_C
        #endif
        #if defined MBEDTLS_CAMELLIA_SMALL_MEMORY
            #define POLARSSL_CAMELLIA_SMALL_MEMORY                        MBEDTLS_CAMELLIA_SMALL_MEMORY
        #endif
        #if defined MBEDTLS_CCM_C
            #define POLARSSL_CCM_C                                        MBEDTLS_CCM_C
        #endif
        #if defined MBEDTLS_CERTS_C
            #define POLARSSL_CERTS_C                                      MBEDTLS_CERTS_C
        #endif
        #if defined MBEDTLS_CIPHER_C
            #define POLARSSL_CIPHER_C                                     MBEDTLS_CIPHER_C
        #endif
        #if defined MBEDTLS_CIPHER_MODE_CBC
            #define POLARSSL_CIPHER_MODE_CBC                              MBEDTLS_CIPHER_MODE_CBC
        #endif
        #if defined MBEDTLS_CIPHER_MODE_CFB
            #define POLARSSL_CIPHER_MODE_CFB                              MBEDTLS_CIPHER_MODE_CFB
        #endif
        #if defined MBEDTLS_CIPHER_MODE_CTR
            #define POLARSSL_CIPHER_MODE_CTR                              MBEDTLS_CIPHER_MODE_CTR
        #endif
        #if defined MBEDTLS_CIPHER_NULL_CIPHER
            #define POLARSSL_CIPHER_NULL_CIPHER                           MBEDTLS_CIPHER_NULL_CIPHER
        #endif
        #if defined MBEDTLS_CIPHER_PADDING_ONE_AND_ZEROS
            #define POLARSSL_CIPHER_PADDING_ONE_AND_ZEROS                 MBEDTLS_CIPHER_PADDING_ONE_AND_ZEROS
        #endif
        #if defined MBEDTLS_CIPHER_PADDING_PKCS7
            #define POLARSSL_CIPHER_PADDING_PKCS7                         MBEDTLS_CIPHER_PADDING_PKCS7
        #endif
        #if defined MBEDTLS_CIPHER_PADDING_ZEROS
            #define POLARSSL_CIPHER_PADDING_ZEROS                         MBEDTLS_CIPHER_PADDING_ZEROS
        #endif
        #if defined MBEDTLS_CIPHER_PADDING_ZEROS_AND_LEN
            #define POLARSSL_CIPHER_PADDING_ZEROS_AND_LEN                 MBEDTLS_CIPHER_PADDING_ZEROS_AND_LEN
        #endif
        #if defined MBEDTLS_CTR_DRBG_C
            #define POLARSSL_CTR_DRBG_C                                   MBEDTLS_CTR_DRBG_C
        #endif
        #if defined MBEDTLS_DEBUG_C
            #define POLARSSL_DEBUG_C                                      MBEDTLS_DEBUG_C
        #endif
        #if defined MBEDTLS_DEPRECATED_REMOVED
            #define POLARSSL_DEPRECATED_REMOVED                           MBEDTLS_DEPRECATED_REMOVED
        #endif
        #if defined MBEDTLS_DEPRECATED_WARNING
            #define POLARSSL_DEPRECATED_WARNING                           MBEDTLS_DEPRECATED_WARNING
        #endif
        #if defined MBEDTLS_DES_ALT
            #define POLARSSL_DES_ALT                                      MBEDTLS_DES_ALT
        #endif
        #if defined MBEDTLS_DES_C
            #define POLARSSL_DES_C                                        MBEDTLS_DES_C
        #endif
        #if defined MBEDTLS_DHM_C
            #define POLARSSL_DHM_C                                        MBEDTLS_DHM_C
        #endif
        #if defined MBEDTLS_ECDH_C
            #define POLARSSL_ECDH_C                                       MBEDTLS_ECDH_C
        #endif
        #if defined MBEDTLS_ECDSA_C
            #define POLARSSL_ECDSA_C                                      MBEDTLS_ECDSA_C
        #endif
        #if defined MBEDTLS_ECDSA_DETERMINISTIC
            #define POLARSSL_ECDSA_DETERMINISTIC                          MBEDTLS_ECDSA_DETERMINISTIC
        #endif
        #if defined MBEDTLS_ECP_C
            #define POLARSSL_ECP_C                                        MBEDTLS_ECP_C
        #endif
        #if defined MBEDTLS_ECP_DP_BP256R1_ENABLED
            #define POLARSSL_ECP_DP_BP256R1_ENABLED                       MBEDTLS_ECP_DP_BP256R1_ENABLED
        #endif
        #if defined MBEDTLS_ECP_DP_BP384R1_ENABLED
            #define POLARSSL_ECP_DP_BP384R1_ENABLED                       MBEDTLS_ECP_DP_BP384R1_ENABLED
        #endif
        #if defined MBEDTLS_ECP_DP_BP512R1_ENABLED
            #define POLARSSL_ECP_DP_BP512R1_ENABLED                       MBEDTLS_ECP_DP_BP512R1_ENABLED
        #endif
        #if defined MBEDTLS_ECP_DP_CURVE25519_ENABLED
            #define POLARSSL_ECP_DP_M255_ENABLED                          MBEDTLS_ECP_DP_CURVE25519_ENABLED
        #endif
        #if defined MBEDTLS_ECP_DP_SECP192K1_ENABLED
            #define POLARSSL_ECP_DP_SECP192K1_ENABLED                     MBEDTLS_ECP_DP_SECP192K1_ENABLED
        #endif
        #if defined MBEDTLS_ECP_DP_SECP192R1_ENABLED
            #define POLARSSL_ECP_DP_SECP192R1_ENABLED                     MBEDTLS_ECP_DP_SECP192R1_ENABLED
        #endif
        #if defined MBEDTLS_ECP_DP_SECP224K1_ENABLED
            #define POLARSSL_ECP_DP_SECP224K1_ENABLED                     MBEDTLS_ECP_DP_SECP224K1_ENABLED
        #endif
        #if defined MBEDTLS_ECP_DP_SECP224R1_ENABLED
            #define POLARSSL_ECP_DP_SECP224R1_ENABLED                     MBEDTLS_ECP_DP_SECP224R1_ENABLED
        #endif
        #if defined MBEDTLS_ECP_DP_SECP256K1_ENABLED
            #define POLARSSL_ECP_DP_SECP256K1_ENABLED                     MBEDTLS_ECP_DP_SECP256K1_ENABLED
        #endif
        #if defined MBEDTLS_ECP_DP_SECP256R1_ENABLED
            #define POLARSSL_ECP_DP_SECP256R1_ENABLED                     MBEDTLS_ECP_DP_SECP256R1_ENABLED
        #endif
        #if defined MBEDTLS_ECP_DP_SECP384R1_ENABLED
            #define POLARSSL_ECP_DP_SECP384R1_ENABLED                     MBEDTLS_ECP_DP_SECP384R1_ENABLED
        #endif
        #if defined MBEDTLS_ECP_DP_SECP521R1_ENABLED
            #define POLARSSL_ECP_DP_SECP521R1_ENABLED                     MBEDTLS_ECP_DP_SECP521R1_ENABLED
        #endif
        #if defined MBEDTLS_ECP_FIXED_POINT_OPTIM
            #define POLARSSL_ECP_FIXED_POINT_OPTIM                        MBEDTLS_ECP_FIXED_POINT_OPTIM
        #endif
        #if defined MBEDTLS_ECP_MAX_BITS
            #define POLARSSL_ECP_MAX_BITS                                 MBEDTLS_ECP_MAX_BITS
        #endif
        #if defined MBEDTLS_ECP_NIST_OPTIM
            #define POLARSSL_ECP_NIST_OPTIM                               MBEDTLS_ECP_NIST_OPTIM
        #endif
        #if defined MBEDTLS_ECP_WINDOW_SIZE
            #define POLARSSL_ECP_WINDOW_SIZE                              MBEDTLS_ECP_WINDOW_SIZE
        #endif
        #if defined MBEDTLS_ENABLE_WEAK_CIPHERSUITES
            #define POLARSSL_ENABLE_WEAK_CIPHERSUITES                     MBEDTLS_ENABLE_WEAK_CIPHERSUITES
        #endif
        #if defined MBEDTLS_ENTROPY_C
            #define POLARSSL_ENTROPY_C                                    MBEDTLS_ENTROPY_C
        #endif
        #if defined MBEDTLS_ENTROPY_FORCE_SHA256
            #define POLARSSL_ENTROPY_FORCE_SHA256                         MBEDTLS_ENTROPY_FORCE_SHA256
        #endif
        #if defined MBEDTLS_ERROR_C
            #define POLARSSL_ERROR_C                                      MBEDTLS_ERROR_C
        #endif
        #if defined MBEDTLS_ERROR_STRERROR_DUMMY
            #define POLARSSL_ERROR_STRERROR_DUMMY                         MBEDTLS_ERROR_STRERROR_DUMMY
        #endif
        #if defined MBEDTLS_FS_IO
            #define POLARSSL_FS_IO                                        MBEDTLS_FS_IO
        #endif
        #if defined MBEDTLS_GCM_C
            #define POLARSSL_GCM_C                                        MBEDTLS_GCM_C
        #endif
        #if defined MBEDTLS_GENPRIME
            #define POLARSSL_GENPRIME                                     MBEDTLS_GENPRIME
        #endif
        #if defined MBEDTLS_HAVEGE_C
            #define POLARSSL_HAVEGE_C                                     MBEDTLS_HAVEGE_C
        #endif
        #if defined MBEDTLS_HAVE_ASM
            #define POLARSSL_HAVE_ASM                                     MBEDTLS_HAVE_ASM
        #endif
        #if defined MBEDTLS_HAVE_SSE2
            #define POLARSSL_HAVE_SSE2                                    MBEDTLS_HAVE_SSE2
        #endif
        #if defined MBEDTLS_HAVE_TIME
            #define POLARSSL_HAVE_TIME                                    MBEDTLS_HAVE_TIME
        #endif
        #if defined MBEDTLS_HMAC_DRBG_C
            #define POLARSSL_HMAC_DRBG_C                                  MBEDTLS_HMAC_DRBG_C
        #endif
        #if defined MBEDTLS_HMAC_DRBG_MAX_INPUT
            #define POLARSSL_HMAC_DRBG_MAX_INPUT                          MBEDTLS_HMAC_DRBG_MAX_INPUT
        #endif
        #if defined MBEDTLS_HMAC_DRBG_MAX_REQUEST
            #define POLARSSL_HMAC_DRBG_MAX_REQUEST                        MBEDTLS_HMAC_DRBG_MAX_REQUEST
        #endif
        #if defined MBEDTLS_HMAC_DRBG_MAX_SEED_INPUT
            #define POLARSSL_HMAC_DRBG_MAX_SEED_INPUT                     MBEDTLS_HMAC_DRBG_MAX_SEED_INPUT
        #endif
        #if defined MBEDTLS_HMAC_DRBG_RESEED_INTERVAL
            #define POLARSSL_HMAC_DRBG_RESEED_INTERVAL                    MBEDTLS_HMAC_DRBG_RESEED_INTERVAL
        #endif
        #if defined MBEDTLS_KEY_EXCHANGE_DHE_PSK_ENABLED
            #define POLARSSL_KEY_EXCHANGE_DHE_PSK_ENABLED                 MBEDTLS_KEY_EXCHANGE_DHE_PSK_ENABLED
        #endif
        #if defined MBEDTLS_KEY_EXCHANGE_DHE_RSA_ENABLED
            #define POLARSSL_KEY_EXCHANGE_DHE_RSA_ENABLED                 MBEDTLS_KEY_EXCHANGE_DHE_RSA_ENABLED
        #endif
        #if defined MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
            #define POLARSSL_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED             MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
        #endif
        #if defined MBEDTLS_KEY_EXCHANGE_ECDHE_PSK_ENABLED
            #define POLARSSL_KEY_EXCHANGE_ECDHE_PSK_ENABLED               MBEDTLS_KEY_EXCHANGE_ECDHE_PSK_ENABLED
        #endif
        #if defined MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED
            #define POLARSSL_KEY_EXCHANGE_ECDHE_RSA_ENABLED               MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED
        #endif
        #if defined MBEDTLS_KEY_EXCHANGE_ECDH_ECDSA_ENABLED
            #define POLARSSL_KEY_EXCHANGE_ECDH_ECDSA_ENABLED              MBEDTLS_KEY_EXCHANGE_ECDH_ECDSA_ENABLED
        #endif
        #if defined MBEDTLS_KEY_EXCHANGE_ECDH_RSA_ENABLED
            #define POLARSSL_KEY_EXCHANGE_ECDH_RSA_ENABLED                MBEDTLS_KEY_EXCHANGE_ECDH_RSA_ENABLED
        #endif
        #if defined MBEDTLS_KEY_EXCHANGE_PSK_ENABLED
            #define POLARSSL_KEY_EXCHANGE_PSK_ENABLED                     MBEDTLS_KEY_EXCHANGE_PSK_ENABLED
        #endif
        #if defined MBEDTLS_KEY_EXCHANGE_RSA_ENABLED
            #define POLARSSL_KEY_EXCHANGE_RSA_ENABLED                     MBEDTLS_KEY_EXCHANGE_RSA_ENABLED
        #endif
        #if defined MBEDTLS_KEY_EXCHANGE_RSA_PSK_ENABLED
            #define POLARSSL_KEY_EXCHANGE_RSA_PSK_ENABLED                 MBEDTLS_KEY_EXCHANGE_RSA_PSK_ENABLED
        #endif
        #if defined MBEDTLS_MD2_ALT
            #define POLARSSL_MD2_ALT                                      MBEDTLS_MD2_ALT
        #endif
        #if defined MBEDTLS_MD2_C
            #define POLARSSL_MD2_C                                        MBEDTLS_MD2_C
        #endif
        #if defined MBEDTLS_MD2_PROCESS_ALT
            #define POLARSSL_MD2_PROCESS_ALT                              MBEDTLS_MD2_PROCESS_ALT
        #endif
        #if defined MBEDTLS_MD4_ALT
            #define POLARSSL_MD4_ALT                                      MBEDTLS_MD4_ALT
        #endif
        #if defined MBEDTLS_MD4_C
            #define POLARSSL_MD4_C                                        MBEDTLS_MD4_C
        #endif
        #if defined MBEDTLS_MD4_PROCESS_ALT
            #define POLARSSL_MD4_PROCESS_ALT                              MBEDTLS_MD4_PROCESS_ALT
        #endif
        #if defined MBEDTLS_MD5_ALT
            #define POLARSSL_MD5_ALT                                      MBEDTLS_MD5_ALT
        #endif
        #if defined MBEDTLS_MD5_C
            #define POLARSSL_MD5_C                                        MBEDTLS_MD5_C
        #endif
        #if defined MBEDTLS_MD5_PROCESS_ALT
            #define POLARSSL_MD5_PROCESS_ALT                              MBEDTLS_MD5_PROCESS_ALT
        #endif
        #if defined MBEDTLS_MD_C
            #define POLARSSL_MD_C                                         MBEDTLS_MD_C
        #endif
        #if defined MBEDTLS_MEMORY_ALIGN_MULTIPLE
            #define POLARSSL_MEMORY_ALIGN_MULTIPLE                        MBEDTLS_MEMORY_ALIGN_MULTIPLE
        #endif
        #if defined MBEDTLS_MEMORY_BACKTRACE
            #define POLARSSL_MEMORY_BACKTRACE                             MBEDTLS_MEMORY_BACKTRACE
        #endif
        #if defined MBEDTLS_MEMORY_BUFFER_ALLOC_C
            #define POLARSSL_MEMORY_BUFFER_ALLOC_C                        MBEDTLS_MEMORY_BUFFER_ALLOC_C
        #endif
        #if defined MBEDTLS_MEMORY_DEBUG
            #define POLARSSL_MEMORY_DEBUG                                 MBEDTLS_MEMORY_DEBUG
        #endif
        #if defined MBEDTLS_MPI_MAX_SIZE
            #define POLARSSL_MPI_MAX_SIZE                                 MBEDTLS_MPI_MAX_SIZE
        #endif
        #if defined MBEDTLS_MPI_WINDOW_SIZE
            #define POLARSSL_MPI_WINDOW_SIZE                              MBEDTLS_MPI_WINDOW_SIZE
        #endif
        #if defined MBEDTLS_NET_C
            #define POLARSSL_NET_C                                        MBEDTLS_NET_C
        #endif
        #if defined MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES
            #define POLARSSL_NO_DEFAULT_ENTROPY_SOURCES                   MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES
        #endif
        #if defined MBEDTLS_NO_PLATFORM_ENTROPY
            #define POLARSSL_NO_PLATFORM_ENTROPY                          MBEDTLS_NO_PLATFORM_ENTROPY
        #endif
        #if defined MBEDTLS_OID_C
            #define POLARSSL_OID_C                                        MBEDTLS_OID_C
        #endif
        #if defined MBEDTLS_PADLOCK_C
            #define POLARSSL_PADLOCK_C                                    MBEDTLS_PADLOCK_C
        #endif
        #if defined MBEDTLS_PEM_PARSE_C
            #define POLARSSL_PEM_PARSE_C                                  MBEDTLS_PEM_PARSE_C
        #endif
        #if defined MBEDTLS_PEM_WRITE_C
            #define POLARSSL_PEM_WRITE_C                                  MBEDTLS_PEM_WRITE_C
        #endif
        #if defined MBEDTLS_PKCS11_C
            #define POLARSSL_PKCS11_C                                     MBEDTLS_PKCS11_C
        #endif
        #if defined MBEDTLS_PKCS12_C
            #define POLARSSL_PKCS12_C                                     MBEDTLS_PKCS12_C
        #endif
        #if defined MBEDTLS_PKCS1_V15
            #define POLARSSL_PKCS1_V15                                    MBEDTLS_PKCS1_V15
        #endif
        #if defined MBEDTLS_PKCS1_V21
            #define POLARSSL_PKCS1_V21                                    MBEDTLS_PKCS1_V21
        #endif
        #if defined MBEDTLS_PKCS5_C
            #define POLARSSL_PKCS5_C                                      MBEDTLS_PKCS5_C
        #endif
        #if defined MBEDTLS_PK_C
            #define POLARSSL_PK_C                                         MBEDTLS_PK_C
        #endif
        #if defined MBEDTLS_PK_PARSE_C
            #define POLARSSL_PK_PARSE_C                                   MBEDTLS_PK_PARSE_C
        #endif
        #if defined MBEDTLS_PK_PARSE_EC_EXTENDED
            #define POLARSSL_PK_PARSE_EC_EXTENDED                         MBEDTLS_PK_PARSE_EC_EXTENDED
        #endif
        #if defined MBEDTLS_PK_RSA_ALT_SUPPORT
            #define POLARSSL_PK_RSA_ALT_SUPPORT                           MBEDTLS_PK_RSA_ALT_SUPPORT
        #endif
        #if defined MBEDTLS_PK_WRITE_C
            #define POLARSSL_PK_WRITE_C                                   MBEDTLS_PK_WRITE_C
        #endif
        #if defined MBEDTLS_PLATFORM_C
            #define POLARSSL_PLATFORM_C                                   MBEDTLS_PLATFORM_C
        #endif
        #if defined MBEDTLS_PLATFORM_EXIT_ALT
            #define POLARSSL_PLATFORM_EXIT_ALT                            MBEDTLS_PLATFORM_EXIT_ALT
        #endif
        #if defined MBEDTLS_PLATFORM_EXIT_MACRO
            #define POLARSSL_PLATFORM_EXIT_MACRO                          MBEDTLS_PLATFORM_EXIT_MACRO
        #endif
        #if defined MBEDTLS_PLATFORM_FPRINTF_ALT
            #define POLARSSL_PLATFORM_FPRINTF_ALT                         MBEDTLS_PLATFORM_FPRINTF_ALT
        #endif
        #if defined MBEDTLS_PLATFORM_FPRINTF_MACRO
            #define POLARSSL_PLATFORM_FPRINTF_MACRO                       MBEDTLS_PLATFORM_FPRINTF_MACRO
        #endif
        #if defined MBEDTLS_PLATFORM_FREE_MACRO
            #define POLARSSL_PLATFORM_FREE_MACRO                          MBEDTLS_PLATFORM_FREE_MACRO
        #endif
        #if defined MBEDTLS_PLATFORM_MEMORY
            #define POLARSSL_PLATFORM_MEMORY                              MBEDTLS_PLATFORM_MEMORY
        #endif
        #if defined MBEDTLS_PLATFORM_NO_STD_FUNCTIONS
            #define POLARSSL_PLATFORM_NO_STD_FUNCTIONS                    MBEDTLS_PLATFORM_NO_STD_FUNCTIONS
        #endif
        #if defined MBEDTLS_PLATFORM_PRINTF_ALT
            #define POLARSSL_PLATFORM_PRINTF_ALT                          MBEDTLS_PLATFORM_PRINTF_ALT
        #endif
        #if defined MBEDTLS_PLATFORM_PRINTF_MACRO
            #define POLARSSL_PLATFORM_PRINTF_MACRO                        MBEDTLS_PLATFORM_PRINTF_MACRO
        #endif
        #if defined MBEDTLS_PLATFORM_SNPRINTF_ALT
            #define POLARSSL_PLATFORM_SNPRINTF_ALT                        MBEDTLS_PLATFORM_SNPRINTF_ALT
        #endif
        #if defined MBEDTLS_PLATFORM_SNPRINTF_MACRO
            #define POLARSSL_PLATFORM_SNPRINTF_MACRO                      MBEDTLS_PLATFORM_SNPRINTF_MACRO
        #endif
        #if defined MBEDTLS_PLATFORM_STD_EXIT
            #define POLARSSL_PLATFORM_STD_EXIT                            MBEDTLS_PLATFORM_STD_EXIT
        #endif
        #if defined MBEDTLS_PLATFORM_STD_FPRINTF
            #define POLARSSL_PLATFORM_STD_FPRINTF                         MBEDTLS_PLATFORM_STD_FPRINTF
        #endif
        #if defined MBEDTLS_PLATFORM_STD_FREE
            #define POLARSSL_PLATFORM_STD_FREE                            MBEDTLS_PLATFORM_STD_FREE
        #endif
        #if defined MBEDTLS_PLATFORM_STD_MEM_HDR
            #define POLARSSL_PLATFORM_STD_MEM_HDR                         MBEDTLS_PLATFORM_STD_MEM_HDR
        #endif
        #if defined MBEDTLS_PLATFORM_STD_PRINTF
            #define POLARSSL_PLATFORM_STD_PRINTF                          MBEDTLS_PLATFORM_STD_PRINTF
        #endif
        #if defined MBEDTLS_PLATFORM_STD_SNPRINTF
            #define POLARSSL_PLATFORM_STD_SNPRINTF                        MBEDTLS_PLATFORM_STD_SNPRINTF
        #endif
        #if defined MBEDTLS_PSK_MAX_LEN
            #define POLARSSL_PSK_MAX_LEN                                  MBEDTLS_PSK_MAX_LEN
        #endif
        #if defined MBEDTLS_REMOVE_ARC4_CIPHERSUITES
            #define POLARSSL_REMOVE_ARC4_CIPHERSUITES                     MBEDTLS_REMOVE_ARC4_CIPHERSUITES
        #endif
        #if defined MBEDTLS_RIPEMD160_ALT
            #define POLARSSL_RIPEMD160_ALT                                MBEDTLS_RIPEMD160_ALT
        #endif
        #if defined MBEDTLS_RIPEMD160_C
            #define POLARSSL_RIPEMD160_C                                  MBEDTLS_RIPEMD160_C
        #endif
        #if defined MBEDTLS_RIPEMD160_PROCESS_ALT
            #define POLARSSL_RIPEMD160_PROCESS_ALT                        MBEDTLS_RIPEMD160_PROCESS_ALT
        #endif
        #if defined MBEDTLS_RSA_C
            #define POLARSSL_RSA_C                                        MBEDTLS_RSA_C
        #endif
        #if defined MBEDTLS_RSA_NO_CRT
            #define POLARSSL_RSA_NO_CRT                                   MBEDTLS_RSA_NO_CRT
        #endif
        #if defined MBEDTLS_SELF_TEST
            #define POLARSSL_SELF_TEST                                    MBEDTLS_SELF_TEST
        #endif
        #if defined MBEDTLS_SHA1_ALT
            #define POLARSSL_SHA1_ALT                                     MBEDTLS_SHA1_ALT
        #endif
        #if defined MBEDTLS_SHA1_C
            #define POLARSSL_SHA1_C                                       MBEDTLS_SHA1_C
        #endif
        #if defined MBEDTLS_SHA1_PROCESS_ALT
            #define POLARSSL_SHA1_PROCESS_ALT                             MBEDTLS_SHA1_PROCESS_ALT
        #endif
        #if defined MBEDTLS_SHA256_ALT
            #define POLARSSL_SHA256_ALT                                   MBEDTLS_SHA256_ALT
        #endif
        #if defined MBEDTLS_SHA256_C
            #define POLARSSL_SHA256_C                                     MBEDTLS_SHA256_C
        #endif
        #if defined MBEDTLS_SHA256_PROCESS_ALT
            #define POLARSSL_SHA256_PROCESS_ALT                           MBEDTLS_SHA256_PROCESS_ALT
        #endif
        #if defined MBEDTLS_SHA512_ALT
            #define POLARSSL_SHA512_ALT                                   MBEDTLS_SHA512_ALT
        #endif
        #if defined MBEDTLS_SHA512_C
            #define POLARSSL_SHA512_C                                     MBEDTLS_SHA512_C
        #endif
        #if defined MBEDTLS_SHA512_PROCESS_ALT
            #define POLARSSL_SHA512_PROCESS_ALT                           MBEDTLS_SHA512_PROCESS_ALT
        #endif
        #if defined MBEDTLS_SSL_ALL_ALERT_MESSAGES
            #define POLARSSL_SSL_ALL_ALERT_MESSAGES                       MBEDTLS_SSL_ALL_ALERT_MESSAGES
        #endif
        #if defined MBEDTLS_SSL_ALPN
            #define POLARSSL_SSL_ALPN                                     MBEDTLS_SSL_ALPN
        #endif
        #if defined MBEDTLS_SSL_CACHE_C
            #define POLARSSL_SSL_CACHE_C                                  MBEDTLS_SSL_CACHE_C
        #endif
        #if defined MBEDTLS_SSL_CBC_RECORD_SPLITTING
            #define POLARSSL_SSL_CBC_RECORD_SPLITTING                     MBEDTLS_SSL_CBC_RECORD_SPLITTING
        #endif
        #if defined MBEDTLS_SSL_CLI_C
            #define POLARSSL_SSL_CLI_C                                    MBEDTLS_SSL_CLI_C
        #endif
        #if defined MBEDTLS_SSL_COOKIE_C
            #define POLARSSL_SSL_COOKIE_C                                 MBEDTLS_SSL_COOKIE_C
        #endif
        #if defined MBEDTLS_SSL_COOKIE_TIMEOUT
            #define POLARSSL_SSL_COOKIE_TIMEOUT                           MBEDTLS_SSL_COOKIE_TIMEOUT
        #endif
        #if defined MBEDTLS_SSL_DEBUG_ALL
            #define POLARSSL_SSL_DEBUG_ALL                                MBEDTLS_SSL_DEBUG_ALL
        #endif
        #if defined MBEDTLS_SSL_DTLS_ANTI_REPLAY
            #define POLARSSL_SSL_DTLS_ANTI_REPLAY                         MBEDTLS_SSL_DTLS_ANTI_REPLAY
        #endif
        #if defined MBEDTLS_SSL_DTLS_BADMAC_LIMIT
            #define POLARSSL_SSL_DTLS_BADMAC_LIMIT                        MBEDTLS_SSL_DTLS_BADMAC_LIMIT
        #endif
        #if defined MBEDTLS_SSL_DTLS_HELLO_VERIFY
            #define POLARSSL_SSL_DTLS_HELLO_VERIFY                        MBEDTLS_SSL_DTLS_HELLO_VERIFY
        #endif
        #if defined MBEDTLS_SSL_ENCRYPT_THEN_MAC
            #define POLARSSL_SSL_ENCRYPT_THEN_MAC                         MBEDTLS_SSL_ENCRYPT_THEN_MAC
        #endif
        #if defined MBEDTLS_SSL_EXTENDED_MASTER_SECRET
            #define POLARSSL_SSL_EXTENDED_MASTER_SECRET                   MBEDTLS_SSL_EXTENDED_MASTER_SECRET
        #endif
        #if defined MBEDTLS_SSL_FALLBACK_SCSV
            #define POLARSSL_SSL_FALLBACK_SCSV                            MBEDTLS_SSL_FALLBACK_SCSV
        #endif
        #if defined MBEDTLS_SSL_HW_RECORD_ACCEL
            #define POLARSSL_SSL_HW_RECORD_ACCEL                          MBEDTLS_SSL_HW_RECORD_ACCEL
        #endif
        #if defined MBEDTLS_SSL_MAX_FRAGMENT_LENGTH
            #define POLARSSL_SSL_MAX_FRAGMENT_LENGTH                      MBEDTLS_SSL_MAX_FRAGMENT_LENGTH
        #endif
        #if defined MBEDTLS_SSL_PROTO_DTLS
            #define POLARSSL_SSL_PROTO_DTLS                               MBEDTLS_SSL_PROTO_DTLS
        #endif
        #if defined MBEDTLS_SSL_PROTO_SSL3
            #define POLARSSL_SSL_PROTO_SSL3                               MBEDTLS_SSL_PROTO_SSL3
        #endif
        #if defined MBEDTLS_SSL_PROTO_TLS1
            #define POLARSSL_SSL_PROTO_TLS1                               MBEDTLS_SSL_PROTO_TLS1
        #endif
        #if defined MBEDTLS_SSL_PROTO_TLS1_1
            #define POLARSSL_SSL_PROTO_TLS1_1                             MBEDTLS_SSL_PROTO_TLS1_1
        #endif
        #if defined MBEDTLS_SSL_PROTO_TLS1_2
            #define POLARSSL_SSL_PROTO_TLS1_2                             MBEDTLS_SSL_PROTO_TLS1_2
        #endif
        #if defined MBEDTLS_SSL_RENEGOTIATION
            #define POLARSSL_SSL_RENEGOTIATION                            MBEDTLS_SSL_RENEGOTIATION
        #endif
        #if defined MBEDTLS_SSL_SERVER_NAME_INDICATION
            #define POLARSSL_SSL_SERVER_NAME_INDICATION                   MBEDTLS_SSL_SERVER_NAME_INDICATION
        #endif
        #if defined MBEDTLS_SSL_SESSION_TICKETS
            #define POLARSSL_SSL_SESSION_TICKETS                          MBEDTLS_SSL_SESSION_TICKETS
        #endif
        #if defined MBEDTLS_SSL_SRV_C
            #define POLARSSL_SSL_SRV_C                                    MBEDTLS_SSL_SRV_C
        #endif
        #if defined MBEDTLS_SSL_SRV_RESPECT_CLIENT_PREFERENCE
            #define POLARSSL_SSL_SRV_RESPECT_CLIENT_PREFERENCE            MBEDTLS_SSL_SRV_RESPECT_CLIENT_PREFERENCE
        #endif
        #if defined MBEDTLS_SSL_SRV_SUPPORT_SSLV2_CLIENT_HELLO
            #define POLARSSL_SSL_SRV_SUPPORT_SSLV2_CLIENT_HELLO           MBEDTLS_SSL_SRV_SUPPORT_SSLV2_CLIENT_HELLO
        #endif
        #if defined MBEDTLS_SSL_TLS_C
            #define POLARSSL_SSL_TLS_C                                    MBEDTLS_SSL_TLS_C
        #endif
        #if defined MBEDTLS_SSL_TRUNCATED_HMAC
            #define POLARSSL_SSL_TRUNCATED_HMAC                           MBEDTLS_SSL_TRUNCATED_HMAC
        #endif
        #if defined MBEDTLS_THREADING_ALT
            #define POLARSSL_THREADING_ALT                                MBEDTLS_THREADING_ALT
        #endif
        #if defined MBEDTLS_THREADING_C
            #define POLARSSL_THREADING_C                                  MBEDTLS_THREADING_C
        #endif
        #if defined MBEDTLS_THREADING_PTHREAD
            #define POLARSSL_THREADING_PTHREAD                            MBEDTLS_THREADING_PTHREAD
        #endif
        #if defined MBEDTLS_TIMING_ALT
            #define POLARSSL_TIMING_ALT                                   MBEDTLS_TIMING_ALT
        #endif
        #if defined MBEDTLS_TIMING_C
            #define POLARSSL_TIMING_C                                     MBEDTLS_TIMING_C
        #endif
        #if defined MBEDTLS_VERSION_C
            #define POLARSSL_VERSION_C                                    MBEDTLS_VERSION_C
        #endif
        #if defined MBEDTLS_VERSION_FEATURES
            #define POLARSSL_VERSION_FEATURES                             MBEDTLS_VERSION_FEATURES
        #endif
        #if defined MBEDTLS_X509_ALLOW_EXTENSIONS_NON_V3
            #define POLARSSL_X509_ALLOW_EXTENSIONS_NON_V3                 MBEDTLS_X509_ALLOW_EXTENSIONS_NON_V3
        #endif
        #if defined MBEDTLS_X509_ALLOW_UNSUPPORTED_CRITICAL_EXTENSION
            #define POLARSSL_X509_ALLOW_UNSUPPORTED_CRITICAL_EXTENSION    MBEDTLS_X509_ALLOW_UNSUPPORTED_CRITICAL_EXTENSION
        #endif
        #if defined MBEDTLS_X509_CHECK_EXTENDED_KEY_USAGE
            #define POLARSSL_X509_CHECK_EXTENDED_KEY_USAGE                MBEDTLS_X509_CHECK_EXTENDED_KEY_USAGE
        #endif
        #if defined MBEDTLS_X509_CHECK_KEY_USAGE
            #define POLARSSL_X509_CHECK_KEY_USAGE                         MBEDTLS_X509_CHECK_KEY_USAGE
        #endif
        #if defined MBEDTLS_X509_CREATE_C
            #define POLARSSL_X509_CREATE_C                                MBEDTLS_X509_CREATE_C
        #endif
        #if defined MBEDTLS_X509_CRL_PARSE_C
            #define POLARSSL_X509_CRL_PARSE_C                             MBEDTLS_X509_CRL_PARSE_C
        #endif
        #if defined MBEDTLS_X509_CRT_PARSE_C
            #define POLARSSL_X509_CRT_PARSE_C                             MBEDTLS_X509_CRT_PARSE_C
        #endif
        #if defined MBEDTLS_X509_CRT_WRITE_C
            #define POLARSSL_X509_CRT_WRITE_C                             MBEDTLS_X509_CRT_WRITE_C
        #endif
        #if defined MBEDTLS_X509_CSR_PARSE_C
            #define POLARSSL_X509_CSR_PARSE_C                             MBEDTLS_X509_CSR_PARSE_C
        #endif
        #if defined MBEDTLS_X509_CSR_WRITE_C
            #define POLARSSL_X509_CSR_WRITE_C                             MBEDTLS_X509_CSR_WRITE_C
        #endif
        #if defined MBEDTLS_X509_MAX_INTERMEDIATE_CA
            #define POLARSSL_X509_MAX_INTERMEDIATE_CA                     MBEDTLS_X509_MAX_INTERMEDIATE_CA
        #endif
        #if defined MBEDTLS_X509_RSASSA_PSS_SUPPORT
            #define POLARSSL_X509_RSASSA_PSS_SUPPORT                      MBEDTLS_X509_RSASSA_PSS_SUPPORT
        #endif
        #if defined MBEDTLS_X509_USE_C
            #define POLARSSL_X509_USE_C                                   MBEDTLS_X509_USE_C
        #endif
        #if defined MBEDTLS_XTEA_ALT
            #define POLARSSL_XTEA_ALT                                     MBEDTLS_XTEA_ALT
        #endif
        #if defined MBEDTLS_XTEA_C
            #define POLARSSL_XTEA_C                                       MBEDTLS_XTEA_C
        #endif
        #if defined MBEDTLS_ZLIB_SUPPORT
            #define POLARSSL_ZLIB_SUPPORT                                 MBEDTLS_ZLIB_SUPPORT
        #endif

/*
 * Misc names (macros, types, functions, enum constants...)
 */
        #define AES_DECRYPT                                       MBEDTLS_AES_DECRYPT
        #define AES_ENCRYPT                                       MBEDTLS_AES_ENCRYPT
        #define ASN1_BIT_STRING                                   MBEDTLS_ASN1_BIT_STRING
        #define ASN1_BMP_STRING                                   MBEDTLS_ASN1_BMP_STRING
        #define ASN1_BOOLEAN                                      MBEDTLS_ASN1_BOOLEAN
        #define ASN1_CHK_ADD                                      MBEDTLS_ASN1_CHK_ADD
        #define ASN1_CONSTRUCTED                                  MBEDTLS_ASN1_CONSTRUCTED
        #define ASN1_CONTEXT_SPECIFIC                             MBEDTLS_ASN1_CONTEXT_SPECIFIC
        #define ASN1_GENERALIZED_TIME                             MBEDTLS_ASN1_GENERALIZED_TIME
        #define ASN1_IA5_STRING                                   MBEDTLS_ASN1_IA5_STRING
        #define ASN1_INTEGER                                      MBEDTLS_ASN1_INTEGER
        #define ASN1_NULL                                         MBEDTLS_ASN1_NULL
        #define ASN1_OCTET_STRING                                 MBEDTLS_ASN1_OCTET_STRING
        #define ASN1_OID                                          MBEDTLS_ASN1_OID
        #define ASN1_PRIMITIVE                                    MBEDTLS_ASN1_PRIMITIVE
        #define ASN1_PRINTABLE_STRING                             MBEDTLS_ASN1_PRINTABLE_STRING
        #define ASN1_SEQUENCE                                     MBEDTLS_ASN1_SEQUENCE
        #define ASN1_SET                                          MBEDTLS_ASN1_SET
        #define ASN1_T61_STRING                                   MBEDTLS_ASN1_T61_STRING
        #define ASN1_UNIVERSAL_STRING                             MBEDTLS_ASN1_UNIVERSAL_STRING
        #define ASN1_UTC_TIME                                     MBEDTLS_ASN1_UTC_TIME
        #define ASN1_UTF8_STRING                                  MBEDTLS_ASN1_UTF8_STRING
        #define BADCERT_CN_MISMATCH                               MBEDTLS_X509_BADCERT_CN_MISMATCH
        #define BADCERT_EXPIRED                                   MBEDTLS_X509_BADCERT_EXPIRED
        #define BADCERT_FUTURE                                    MBEDTLS_X509_BADCERT_FUTURE
        #define BADCERT_MISSING                                   MBEDTLS_X509_BADCERT_MISSING
        #define BADCERT_NOT_TRUSTED                               MBEDTLS_X509_BADCERT_NOT_TRUSTED
        #define BADCERT_OTHER                                     MBEDTLS_X509_BADCERT_OTHER
        #define BADCERT_REVOKED                                   MBEDTLS_X509_BADCERT_REVOKED
        #define BADCERT_SKIP_VERIFY                               MBEDTLS_X509_BADCERT_SKIP_VERIFY
        #define BADCRL_EXPIRED                                    MBEDTLS_X509_BADCRL_EXPIRED
        #define BADCRL_FUTURE                                     MBEDTLS_X509_BADCRL_FUTURE
        #define BADCRL_NOT_TRUSTED                                MBEDTLS_X509_BADCRL_NOT_TRUSTED
        #define BLOWFISH_BLOCKSIZE                                MBEDTLS_BLOWFISH_BLOCKSIZE
        #define BLOWFISH_DECRYPT                                  MBEDTLS_BLOWFISH_DECRYPT
        #define BLOWFISH_ENCRYPT                                  MBEDTLS_BLOWFISH_ENCRYPT
        #define BLOWFISH_MAX_KEY                                  MBEDTLS_BLOWFISH_MAX_KEY_BITS
        #define BLOWFISH_MIN_KEY                                  MBEDTLS_BLOWFISH_MIN_KEY_BITS
        #define BLOWFISH_ROUNDS                                   MBEDTLS_BLOWFISH_ROUNDS
        #define CAMELLIA_DECRYPT                                  MBEDTLS_CAMELLIA_DECRYPT
        #define CAMELLIA_ENCRYPT                                  MBEDTLS_CAMELLIA_ENCRYPT
        #define COLLECT_SIZE                                      MBEDTLS_HAVEGE_COLLECT_SIZE
        #define CTR_DRBG_BLOCKSIZE                                MBEDTLS_CTR_DRBG_BLOCKSIZE
        #define CTR_DRBG_ENTROPY_LEN                              MBEDTLS_CTR_DRBG_ENTROPY_LEN
        #define CTR_DRBG_KEYBITS                                  MBEDTLS_CTR_DRBG_KEYBITS
        #define CTR_DRBG_KEYSIZE                                  MBEDTLS_CTR_DRBG_KEYSIZE
        #define CTR_DRBG_MAX_INPUT                                MBEDTLS_CTR_DRBG_MAX_INPUT
        #define CTR_DRBG_MAX_REQUEST                              MBEDTLS_CTR_DRBG_MAX_REQUEST
        #define CTR_DRBG_MAX_SEED_INPUT                           MBEDTLS_CTR_DRBG_MAX_SEED_INPUT
        #define CTR_DRBG_PR_OFF                                   MBEDTLS_CTR_DRBG_PR_OFF
        #define CTR_DRBG_PR_ON                                    MBEDTLS_CTR_DRBG_PR_ON
        #define CTR_DRBG_RESEED_INTERVAL                          MBEDTLS_CTR_DRBG_RESEED_INTERVAL
        #define CTR_DRBG_SEEDLEN                                  MBEDTLS_CTR_DRBG_SEEDLEN
        #define DEPRECATED                                        MBEDTLS_DEPRECATED
        #define DES_DECRYPT                                       MBEDTLS_DES_DECRYPT
        #define DES_ENCRYPT                                       MBEDTLS_DES_ENCRYPT
        #define DES_KEY_SIZE                                      MBEDTLS_DES_KEY_SIZE
        #define ENTROPY_BLOCK_SIZE                                MBEDTLS_ENTROPY_BLOCK_SIZE
        #define ENTROPY_MAX_GATHER                                MBEDTLS_ENTROPY_MAX_GATHER
        #define ENTROPY_MAX_SEED_SIZE                             MBEDTLS_ENTROPY_MAX_SEED_SIZE
        #define ENTROPY_MAX_SOURCES                               MBEDTLS_ENTROPY_MAX_SOURCES
        #define ENTROPY_MIN_HARDCLOCK                             MBEDTLS_ENTROPY_MIN_HARDCLOCK
        #define ENTROPY_MIN_HAVEGE                                MBEDTLS_ENTROPY_MIN_HAVEGE
        #define ENTROPY_MIN_PLATFORM                              MBEDTLS_ENTROPY_MIN_PLATFORM
        #define ENTROPY_SOURCE_MANUAL                             MBEDTLS_ENTROPY_SOURCE_MANUAL
        #define EXT_AUTHORITY_KEY_IDENTIFIER                      MBEDTLS_X509_EXT_AUTHORITY_KEY_IDENTIFIER
        #define EXT_BASIC_CONSTRAINTS                             MBEDTLS_X509_EXT_BASIC_CONSTRAINTS
        #define EXT_CERTIFICATE_POLICIES                          MBEDTLS_X509_EXT_CERTIFICATE_POLICIES
        #define EXT_CRL_DISTRIBUTION_POINTS                       MBEDTLS_X509_EXT_CRL_DISTRIBUTION_POINTS
        #define EXT_EXTENDED_KEY_USAGE                            MBEDTLS_X509_EXT_EXTENDED_KEY_USAGE
        #define EXT_FRESHEST_CRL                                  MBEDTLS_X509_EXT_FRESHEST_CRL
        #define EXT_INIHIBIT_ANYPOLICY                            MBEDTLS_X509_EXT_INIHIBIT_ANYPOLICY
        #define EXT_ISSUER_ALT_NAME                               MBEDTLS_X509_EXT_ISSUER_ALT_NAME
        #define EXT_KEY_USAGE                                     MBEDTLS_X509_EXT_KEY_USAGE
        #define EXT_NAME_CONSTRAINTS                              MBEDTLS_X509_EXT_NAME_CONSTRAINTS
        #define EXT_NS_CERT_TYPE                                  MBEDTLS_X509_EXT_NS_CERT_TYPE
        #define EXT_POLICY_CONSTRAINTS                            MBEDTLS_X509_EXT_POLICY_CONSTRAINTS
        #define EXT_POLICY_MAPPINGS                               MBEDTLS_X509_EXT_POLICY_MAPPINGS
        #define EXT_SUBJECT_ALT_NAME                              MBEDTLS_X509_EXT_SUBJECT_ALT_NAME
        #define EXT_SUBJECT_DIRECTORY_ATTRS                       MBEDTLS_X509_EXT_SUBJECT_DIRECTORY_ATTRS
        #define EXT_SUBJECT_KEY_IDENTIFIER                        MBEDTLS_X509_EXT_SUBJECT_KEY_IDENTIFIER
        #define GCM_DECRYPT                                       MBEDTLS_GCM_DECRYPT
        #define GCM_ENCRYPT                                       MBEDTLS_GCM_ENCRYPT
        #define KU_CRL_SIGN                                       MBEDTLS_X509_KU_CRL_SIGN
        #define KU_DATA_ENCIPHERMENT                              MBEDTLS_X509_KU_DATA_ENCIPHERMENT
        #define KU_DIGITAL_SIGNATURE                              MBEDTLS_X509_KU_DIGITAL_SIGNATURE
        #define KU_KEY_AGREEMENT                                  MBEDTLS_X509_KU_KEY_AGREEMENT
        #define KU_KEY_CERT_SIGN                                  MBEDTLS_X509_KU_KEY_CERT_SIGN
        #define KU_KEY_ENCIPHERMENT                               MBEDTLS_X509_KU_KEY_ENCIPHERMENT
        #define KU_NON_REPUDIATION                                MBEDTLS_X509_KU_NON_REPUDIATION
        #define LN_2_DIV_LN_10_SCALE100                           MBEDTLS_LN_2_DIV_LN_10_SCALE100
        #define MEMORY_VERIFY_ALLOC                               MBEDTLS_MEMORY_VERIFY_ALLOC
        #define MEMORY_VERIFY_ALWAYS                              MBEDTLS_MEMORY_VERIFY_ALWAYS
        #define MEMORY_VERIFY_FREE                                MBEDTLS_MEMORY_VERIFY_FREE
        #define MEMORY_VERIFY_NONE                                MBEDTLS_MEMORY_VERIFY_NONE
        #define MPI_CHK                                           MBEDTLS_MPI_CHK
        #define NET_PROTO_TCP                                     MBEDTLS_NET_PROTO_TCP
        #define NET_PROTO_UDP                                     MBEDTLS_NET_PROTO_UDP
        #define NS_CERT_TYPE_EMAIL                                MBEDTLS_X509_NS_CERT_TYPE_EMAIL
        #define NS_CERT_TYPE_EMAIL_CA                             MBEDTLS_X509_NS_CERT_TYPE_EMAIL_CA
        #define NS_CERT_TYPE_OBJECT_SIGNING                       MBEDTLS_X509_NS_CERT_TYPE_OBJECT_SIGNING
        #define NS_CERT_TYPE_OBJECT_SIGNING_CA                    MBEDTLS_X509_NS_CERT_TYPE_OBJECT_SIGNING_CA
        #define NS_CERT_TYPE_RESERVED                             MBEDTLS_X509_NS_CERT_TYPE_RESERVED
        #define NS_CERT_TYPE_SSL_CA                               MBEDTLS_X509_NS_CERT_TYPE_SSL_CA
        #define NS_CERT_TYPE_SSL_CLIENT                           MBEDTLS_X509_NS_CERT_TYPE_SSL_CLIENT
        #define NS_CERT_TYPE_SSL_SERVER                           MBEDTLS_X509_NS_CERT_TYPE_SSL_SERVER
        #define OID_ANSI_X9_62                                    MBEDTLS_OID_ANSI_X9_62
        #define OID_ANSI_X9_62_FIELD_TYPE                         MBEDTLS_OID_ANSI_X9_62_FIELD_TYPE
        #define OID_ANSI_X9_62_PRIME_FIELD                        MBEDTLS_OID_ANSI_X9_62_PRIME_FIELD
        #define OID_ANSI_X9_62_SIG                                MBEDTLS_OID_ANSI_X9_62_SIG
        #define OID_ANSI_X9_62_SIG_SHA2                           MBEDTLS_OID_ANSI_X9_62_SIG_SHA2
        #define OID_ANY_EXTENDED_KEY_USAGE                        MBEDTLS_OID_ANY_EXTENDED_KEY_USAGE
        #define OID_AT                                            MBEDTLS_OID_AT
        #define OID_AT_CN                                         MBEDTLS_OID_AT_CN
        #define OID_AT_COUNTRY                                    MBEDTLS_OID_AT_COUNTRY
        #define OID_AT_DN_QUALIFIER                               MBEDTLS_OID_AT_DN_QUALIFIER
        #define OID_AT_GENERATION_QUALIFIER                       MBEDTLS_OID_AT_GENERATION_QUALIFIER
        #define OID_AT_GIVEN_NAME                                 MBEDTLS_OID_AT_GIVEN_NAME
        #define OID_AT_INITIALS                                   MBEDTLS_OID_AT_INITIALS
        #define OID_AT_LOCALITY                                   MBEDTLS_OID_AT_LOCALITY
        #define OID_AT_ORGANIZATION                               MBEDTLS_OID_AT_ORGANIZATION
        #define OID_AT_ORG_UNIT                                   MBEDTLS_OID_AT_ORG_UNIT
        #define OID_AT_POSTAL_ADDRESS                             MBEDTLS_OID_AT_POSTAL_ADDRESS
        #define OID_AT_POSTAL_CODE                                MBEDTLS_OID_AT_POSTAL_CODE
        #define OID_AT_PSEUDONYM                                  MBEDTLS_OID_AT_PSEUDONYM
        #define OID_AT_SERIAL_NUMBER                              MBEDTLS_OID_AT_SERIAL_NUMBER
        #define OID_AT_STATE                                      MBEDTLS_OID_AT_STATE
        #define OID_AT_SUR_NAME                                   MBEDTLS_OID_AT_SUR_NAME
        #define OID_AT_TITLE                                      MBEDTLS_OID_AT_TITLE
        #define OID_AT_UNIQUE_IDENTIFIER                          MBEDTLS_OID_AT_UNIQUE_IDENTIFIER
        #define OID_AUTHORITY_KEY_IDENTIFIER                      MBEDTLS_OID_AUTHORITY_KEY_IDENTIFIER
        #define OID_BASIC_CONSTRAINTS                             MBEDTLS_OID_BASIC_CONSTRAINTS
        #define OID_CERTICOM                                      MBEDTLS_OID_CERTICOM
        #define OID_CERTIFICATE_POLICIES                          MBEDTLS_OID_CERTIFICATE_POLICIES
        #define OID_CLIENT_AUTH                                   MBEDTLS_OID_CLIENT_AUTH
        #define OID_CMP                                           MBEDTLS_OID_CMP
        #define OID_CODE_SIGNING                                  MBEDTLS_OID_CODE_SIGNING
        #define OID_COUNTRY_US                                    MBEDTLS_OID_COUNTRY_US
        #define OID_CRL_DISTRIBUTION_POINTS                       MBEDTLS_OID_CRL_DISTRIBUTION_POINTS
        #define OID_CRL_NUMBER                                    MBEDTLS_OID_CRL_NUMBER
        #define OID_DES_CBC                                       MBEDTLS_OID_DES_CBC
        #define OID_DES_EDE3_CBC                                  MBEDTLS_OID_DES_EDE3_CBC
        #define OID_DIGEST_ALG_MD2                                MBEDTLS_OID_DIGEST_ALG_MD2
        #define OID_DIGEST_ALG_MD4                                MBEDTLS_OID_DIGEST_ALG_MD4
        #define OID_DIGEST_ALG_MD5                                MBEDTLS_OID_DIGEST_ALG_MD5
        #define OID_DIGEST_ALG_SHA1                               MBEDTLS_OID_DIGEST_ALG_SHA1
        #define OID_DIGEST_ALG_SHA224                             MBEDTLS_OID_DIGEST_ALG_SHA224
        #define OID_DIGEST_ALG_SHA256                             MBEDTLS_OID_DIGEST_ALG_SHA256
        #define OID_DIGEST_ALG_SHA384                             MBEDTLS_OID_DIGEST_ALG_SHA384
        #define OID_DIGEST_ALG_SHA512                             MBEDTLS_OID_DIGEST_ALG_SHA512
        #define OID_DOMAIN_COMPONENT                              MBEDTLS_OID_DOMAIN_COMPONENT
        #define OID_ECDSA_SHA1                                    MBEDTLS_OID_ECDSA_SHA1
        #define OID_ECDSA_SHA224                                  MBEDTLS_OID_ECDSA_SHA224
        #define OID_ECDSA_SHA256                                  MBEDTLS_OID_ECDSA_SHA256
        #define OID_ECDSA_SHA384                                  MBEDTLS_OID_ECDSA_SHA384
        #define OID_ECDSA_SHA512                                  MBEDTLS_OID_ECDSA_SHA512
        #define OID_EC_ALG_ECDH                                   MBEDTLS_OID_EC_ALG_ECDH
        #define OID_EC_ALG_UNRESTRICTED                           MBEDTLS_OID_EC_ALG_UNRESTRICTED
        #define OID_EC_BRAINPOOL_V1                               MBEDTLS_OID_EC_BRAINPOOL_V1
        #define OID_EC_GRP_BP256R1                                MBEDTLS_OID_EC_GRP_BP256R1
        #define OID_EC_GRP_BP384R1                                MBEDTLS_OID_EC_GRP_BP384R1
        #define OID_EC_GRP_BP512R1                                MBEDTLS_OID_EC_GRP_BP512R1
        #define OID_EC_GRP_SECP192K1                              MBEDTLS_OID_EC_GRP_SECP192K1
        #define OID_EC_GRP_SECP192R1                              MBEDTLS_OID_EC_GRP_SECP192R1
        #define OID_EC_GRP_SECP224K1                              MBEDTLS_OID_EC_GRP_SECP224K1
        #define OID_EC_GRP_SECP224R1                              MBEDTLS_OID_EC_GRP_SECP224R1
        #define OID_EC_GRP_SECP256K1                              MBEDTLS_OID_EC_GRP_SECP256K1
        #define OID_EC_GRP_SECP256R1                              MBEDTLS_OID_EC_GRP_SECP256R1
        #define OID_EC_GRP_SECP384R1                              MBEDTLS_OID_EC_GRP_SECP384R1
        #define OID_EC_GRP_SECP521R1                              MBEDTLS_OID_EC_GRP_SECP521R1
        #define OID_EMAIL_PROTECTION                              MBEDTLS_OID_EMAIL_PROTECTION
        #define OID_EXTENDED_KEY_USAGE                            MBEDTLS_OID_EXTENDED_KEY_USAGE
        #define OID_FRESHEST_CRL                                  MBEDTLS_OID_FRESHEST_CRL
        #define OID_GOV                                           MBEDTLS_OID_GOV
        #define OID_HMAC_SHA1                                     MBEDTLS_OID_HMAC_SHA1
        #define OID_ID_CE                                         MBEDTLS_OID_ID_CE
        #define OID_INIHIBIT_ANYPOLICY                            MBEDTLS_OID_INIHIBIT_ANYPOLICY
        #define OID_ISO_CCITT_DS                                  MBEDTLS_OID_ISO_CCITT_DS
        #define OID_ISO_IDENTIFIED_ORG                            MBEDTLS_OID_ISO_IDENTIFIED_ORG
        #define OID_ISO_ITU_COUNTRY                               MBEDTLS_OID_ISO_ITU_COUNTRY
        #define OID_ISO_ITU_US_ORG                                MBEDTLS_OID_ISO_ITU_US_ORG
        #define OID_ISO_MEMBER_BODIES                             MBEDTLS_OID_ISO_MEMBER_BODIES
        #define OID_ISSUER_ALT_NAME                               MBEDTLS_OID_ISSUER_ALT_NAME
        #define OID_KEY_USAGE                                     MBEDTLS_OID_KEY_USAGE
        #define OID_KP                                            MBEDTLS_OID_KP
        #define OID_MGF1                                          MBEDTLS_OID_MGF1
        #define OID_NAME_CONSTRAINTS                              MBEDTLS_OID_NAME_CONSTRAINTS
        #define OID_NETSCAPE                                      MBEDTLS_OID_NETSCAPE
        #define OID_NS_BASE_URL                                   MBEDTLS_OID_NS_BASE_URL
        #define OID_NS_CA_POLICY_URL                              MBEDTLS_OID_NS_CA_POLICY_URL
        #define OID_NS_CA_REVOCATION_URL                          MBEDTLS_OID_NS_CA_REVOCATION_URL
        #define OID_NS_CERT                                       MBEDTLS_OID_NS_CERT
        #define OID_NS_CERT_SEQUENCE                              MBEDTLS_OID_NS_CERT_SEQUENCE
        #define OID_NS_CERT_TYPE                                  MBEDTLS_OID_NS_CERT_TYPE
        #define OID_NS_COMMENT                                    MBEDTLS_OID_NS_COMMENT
        #define OID_NS_DATA_TYPE                                  MBEDTLS_OID_NS_DATA_TYPE
        #define OID_NS_RENEWAL_URL                                MBEDTLS_OID_NS_RENEWAL_URL
        #define OID_NS_REVOCATION_URL                             MBEDTLS_OID_NS_REVOCATION_URL
        #define OID_NS_SSL_SERVER_NAME                            MBEDTLS_OID_NS_SSL_SERVER_NAME
        #define OID_OCSP_SIGNING                                  MBEDTLS_OID_OCSP_SIGNING
        #define OID_OIW_SECSIG                                    MBEDTLS_OID_OIW_SECSIG
        #define OID_OIW_SECSIG_ALG                                MBEDTLS_OID_OIW_SECSIG_ALG
        #define OID_OIW_SECSIG_SHA1                               MBEDTLS_OID_OIW_SECSIG_SHA1
        #define OID_ORGANIZATION                                  MBEDTLS_OID_ORGANIZATION
        #define OID_ORG_ANSI_X9_62                                MBEDTLS_OID_ORG_ANSI_X9_62
        #define OID_ORG_CERTICOM                                  MBEDTLS_OID_ORG_CERTICOM
        #define OID_ORG_DOD                                       MBEDTLS_OID_ORG_DOD
        #define OID_ORG_GOV                                       MBEDTLS_OID_ORG_GOV
        #define OID_ORG_NETSCAPE                                  MBEDTLS_OID_ORG_NETSCAPE
        #define OID_ORG_OIW                                       MBEDTLS_OID_ORG_OIW
        #define OID_ORG_RSA_DATA_SECURITY                         MBEDTLS_OID_ORG_RSA_DATA_SECURITY
        #define OID_ORG_TELETRUST                                 MBEDTLS_OID_ORG_TELETRUST
        #define OID_PKCS                                          MBEDTLS_OID_PKCS
        #define OID_PKCS1                                         MBEDTLS_OID_PKCS1
        #define OID_PKCS12                                        MBEDTLS_OID_PKCS12
        #define OID_PKCS12_PBE                                    MBEDTLS_OID_PKCS12_PBE
        #define OID_PKCS12_PBE_SHA1_DES2_EDE_CBC                  MBEDTLS_OID_PKCS12_PBE_SHA1_DES2_EDE_CBC
        #define OID_PKCS12_PBE_SHA1_DES3_EDE_CBC                  MBEDTLS_OID_PKCS12_PBE_SHA1_DES3_EDE_CBC
        #define OID_PKCS12_PBE_SHA1_RC2_128_CBC                   MBEDTLS_OID_PKCS12_PBE_SHA1_RC2_128_CBC
        #define OID_PKCS12_PBE_SHA1_RC2_40_CBC                    MBEDTLS_OID_PKCS12_PBE_SHA1_RC2_40_CBC
        #define OID_PKCS12_PBE_SHA1_RC4_128                       MBEDTLS_OID_PKCS12_PBE_SHA1_RC4_128
        #define OID_PKCS12_PBE_SHA1_RC4_40                        MBEDTLS_OID_PKCS12_PBE_SHA1_RC4_40
        #define OID_PKCS1_MD2                                     MBEDTLS_OID_PKCS1_MD2
        #define OID_PKCS1_MD4                                     MBEDTLS_OID_PKCS1_MD4
        #define OID_PKCS1_MD5                                     MBEDTLS_OID_PKCS1_MD5
        #define OID_PKCS1_RSA                                     MBEDTLS_OID_PKCS1_RSA
        #define OID_PKCS1_SHA1                                    MBEDTLS_OID_PKCS1_SHA1
        #define OID_PKCS1_SHA224                                  MBEDTLS_OID_PKCS1_SHA224
        #define OID_PKCS1_SHA256                                  MBEDTLS_OID_PKCS1_SHA256
        #define OID_PKCS1_SHA384                                  MBEDTLS_OID_PKCS1_SHA384
        #define OID_PKCS1_SHA512                                  MBEDTLS_OID_PKCS1_SHA512
        #define OID_PKCS5                                         MBEDTLS_OID_PKCS5
        #define OID_PKCS5_PBES2                                   MBEDTLS_OID_PKCS5_PBES2
        #define OID_PKCS5_PBE_MD2_DES_CBC                         MBEDTLS_OID_PKCS5_PBE_MD2_DES_CBC
        #define OID_PKCS5_PBE_MD2_RC2_CBC                         MBEDTLS_OID_PKCS5_PBE_MD2_RC2_CBC
        #define OID_PKCS5_PBE_MD5_DES_CBC                         MBEDTLS_OID_PKCS5_PBE_MD5_DES_CBC
        #define OID_PKCS5_PBE_MD5_RC2_CBC                         MBEDTLS_OID_PKCS5_PBE_MD5_RC2_CBC
        #define OID_PKCS5_PBE_SHA1_DES_CBC                        MBEDTLS_OID_PKCS5_PBE_SHA1_DES_CBC
        #define OID_PKCS5_PBE_SHA1_RC2_CBC                        MBEDTLS_OID_PKCS5_PBE_SHA1_RC2_CBC
        #define OID_PKCS5_PBKDF2                                  MBEDTLS_OID_PKCS5_PBKDF2
        #define OID_PKCS5_PBMAC1                                  MBEDTLS_OID_PKCS5_PBMAC1
        #define OID_PKCS9                                         MBEDTLS_OID_PKCS9
        #define OID_PKCS9_CSR_EXT_REQ                             MBEDTLS_OID_PKCS9_CSR_EXT_REQ
        #define OID_PKCS9_EMAIL                                   MBEDTLS_OID_PKCS9_EMAIL
        #define OID_PKIX                                          MBEDTLS_OID_PKIX
        #define OID_POLICY_CONSTRAINTS                            MBEDTLS_OID_POLICY_CONSTRAINTS
        #define OID_POLICY_MAPPINGS                               MBEDTLS_OID_POLICY_MAPPINGS
        #define OID_PRIVATE_KEY_USAGE_PERIOD                      MBEDTLS_OID_PRIVATE_KEY_USAGE_PERIOD
        #define OID_RSASSA_PSS                                    MBEDTLS_OID_RSASSA_PSS
        #define OID_RSA_COMPANY                                   MBEDTLS_OID_RSA_COMPANY
        #define OID_RSA_SHA_OBS                                   MBEDTLS_OID_RSA_SHA_OBS
        #define OID_SERVER_AUTH                                   MBEDTLS_OID_SERVER_AUTH
        #define OID_SIZE                                          MBEDTLS_OID_SIZE
        #define OID_SUBJECT_ALT_NAME                              MBEDTLS_OID_SUBJECT_ALT_NAME
        #define OID_SUBJECT_DIRECTORY_ATTRS                       MBEDTLS_OID_SUBJECT_DIRECTORY_ATTRS
        #define OID_SUBJECT_KEY_IDENTIFIER                        MBEDTLS_OID_SUBJECT_KEY_IDENTIFIER
        #define OID_TELETRUST                                     MBEDTLS_OID_TELETRUST
        #define OID_TIME_STAMPING                                 MBEDTLS_OID_TIME_STAMPING
        #define PADLOCK_ACE                                       MBEDTLS_PADLOCK_ACE
        #define PADLOCK_ALIGN16                                   MBEDTLS_PADLOCK_ALIGN16
        #define PADLOCK_PHE                                       MBEDTLS_PADLOCK_PHE
        #define PADLOCK_PMM                                       MBEDTLS_PADLOCK_PMM
        #define PADLOCK_RNG                                       MBEDTLS_PADLOCK_RNG
        #define PKCS12_DERIVE_IV                                  MBEDTLS_PKCS12_DERIVE_IV
        #define PKCS12_DERIVE_KEY                                 MBEDTLS_PKCS12_DERIVE_KEY
        #define PKCS12_DERIVE_MAC_KEY                             MBEDTLS_PKCS12_DERIVE_MAC_KEY
        #define PKCS12_PBE_DECRYPT                                MBEDTLS_PKCS12_PBE_DECRYPT
        #define PKCS12_PBE_ENCRYPT                                MBEDTLS_PKCS12_PBE_ENCRYPT
        #define PKCS5_DECRYPT                                     MBEDTLS_PKCS5_DECRYPT
        #define PKCS5_ENCRYPT                                     MBEDTLS_PKCS5_ENCRYPT
        #define POLARSSL_AESNI_AES                                MBEDTLS_AESNI_AES
        #define POLARSSL_AESNI_CLMUL                              MBEDTLS_AESNI_CLMUL
        #define POLARSSL_AESNI_H                                  MBEDTLS_AESNI_H
        #define POLARSSL_AES_H                                    MBEDTLS_AES_H
        #define POLARSSL_ARC4_H                                   MBEDTLS_ARC4_H
        #define POLARSSL_ASN1_H                                   MBEDTLS_ASN1_H
        #define POLARSSL_ASN1_WRITE_H                             MBEDTLS_ASN1_WRITE_H
        #define POLARSSL_BASE64_H                                 MBEDTLS_BASE64_H
        #define POLARSSL_BIGNUM_H                                 MBEDTLS_BIGNUM_H
        #define POLARSSL_BLOWFISH_H                               MBEDTLS_BLOWFISH_H
        #define POLARSSL_BN_MUL_H                                 MBEDTLS_BN_MUL_H
        #define POLARSSL_CAMELLIA_H                               MBEDTLS_CAMELLIA_H
        #define POLARSSL_CCM_H                                    MBEDTLS_CCM_H
        #define POLARSSL_CERTS_H                                  MBEDTLS_CERTS_H
        #define POLARSSL_CHECK_CONFIG_H                           MBEDTLS_CHECK_CONFIG_H
        #define POLARSSL_CIPHERSUITE_NODTLS                       MBEDTLS_CIPHERSUITE_NODTLS
        #define POLARSSL_CIPHERSUITE_SHORT_TAG                    MBEDTLS_CIPHERSUITE_SHORT_TAG
        #define POLARSSL_CIPHERSUITE_WEAK                         MBEDTLS_CIPHERSUITE_WEAK
        #define POLARSSL_CIPHER_AES_128_CBC                       MBEDTLS_CIPHER_AES_128_CBC
        #define POLARSSL_CIPHER_AES_128_CCM                       MBEDTLS_CIPHER_AES_128_CCM
        #define POLARSSL_CIPHER_AES_128_CFB128                    MBEDTLS_CIPHER_AES_128_CFB128
        #define POLARSSL_CIPHER_AES_128_CTR                       MBEDTLS_CIPHER_AES_128_CTR
        #define POLARSSL_CIPHER_AES_128_ECB                       MBEDTLS_CIPHER_AES_128_ECB
        #define POLARSSL_CIPHER_AES_128_GCM                       MBEDTLS_CIPHER_AES_128_GCM
        #define POLARSSL_CIPHER_AES_192_CBC                       MBEDTLS_CIPHER_AES_192_CBC
        #define POLARSSL_CIPHER_AES_192_CCM                       MBEDTLS_CIPHER_AES_192_CCM
        #define POLARSSL_CIPHER_AES_192_CFB128                    MBEDTLS_CIPHER_AES_192_CFB128
        #define POLARSSL_CIPHER_AES_192_CTR                       MBEDTLS_CIPHER_AES_192_CTR
        #define POLARSSL_CIPHER_AES_192_ECB                       MBEDTLS_CIPHER_AES_192_ECB
        #define POLARSSL_CIPHER_AES_192_GCM                       MBEDTLS_CIPHER_AES_192_GCM
        #define POLARSSL_CIPHER_AES_256_CBC                       MBEDTLS_CIPHER_AES_256_CBC
        #define POLARSSL_CIPHER_AES_256_CCM                       MBEDTLS_CIPHER_AES_256_CCM
        #define POLARSSL_CIPHER_AES_256_CFB128                    MBEDTLS_CIPHER_AES_256_CFB128
        #define POLARSSL_CIPHER_AES_256_CTR                       MBEDTLS_CIPHER_AES_256_CTR
        #define POLARSSL_CIPHER_AES_256_ECB                       MBEDTLS_CIPHER_AES_256_ECB
        #define POLARSSL_CIPHER_AES_256_GCM                       MBEDTLS_CIPHER_AES_256_GCM
        #define POLARSSL_CIPHER_ARC4_128                          MBEDTLS_CIPHER_ARC4_128
        #define POLARSSL_CIPHER_BLOWFISH_CBC                      MBEDTLS_CIPHER_BLOWFISH_CBC
        #define POLARSSL_CIPHER_BLOWFISH_CFB64                    MBEDTLS_CIPHER_BLOWFISH_CFB64
        #define POLARSSL_CIPHER_BLOWFISH_CTR                      MBEDTLS_CIPHER_BLOWFISH_CTR
        #define POLARSSL_CIPHER_BLOWFISH_ECB                      MBEDTLS_CIPHER_BLOWFISH_ECB
        #define POLARSSL_CIPHER_CAMELLIA_128_CBC                  MBEDTLS_CIPHER_CAMELLIA_128_CBC
        #define POLARSSL_CIPHER_CAMELLIA_128_CCM                  MBEDTLS_CIPHER_CAMELLIA_128_CCM
        #define POLARSSL_CIPHER_CAMELLIA_128_CFB128               MBEDTLS_CIPHER_CAMELLIA_128_CFB128
        #define POLARSSL_CIPHER_CAMELLIA_128_CTR                  MBEDTLS_CIPHER_CAMELLIA_128_CTR
        #define POLARSSL_CIPHER_CAMELLIA_128_ECB                  MBEDTLS_CIPHER_CAMELLIA_128_ECB
        #define POLARSSL_CIPHER_CAMELLIA_128_GCM                  MBEDTLS_CIPHER_CAMELLIA_128_GCM
        #define POLARSSL_CIPHER_CAMELLIA_192_CBC                  MBEDTLS_CIPHER_CAMELLIA_192_CBC
        #define POLARSSL_CIPHER_CAMELLIA_192_CCM                  MBEDTLS_CIPHER_CAMELLIA_192_CCM
        #define POLARSSL_CIPHER_CAMELLIA_192_CFB128               MBEDTLS_CIPHER_CAMELLIA_192_CFB128
        #define POLARSSL_CIPHER_CAMELLIA_192_CTR                  MBEDTLS_CIPHER_CAMELLIA_192_CTR
        #define POLARSSL_CIPHER_CAMELLIA_192_ECB                  MBEDTLS_CIPHER_CAMELLIA_192_ECB
        #define POLARSSL_CIPHER_CAMELLIA_192_GCM                  MBEDTLS_CIPHER_CAMELLIA_192_GCM
        #define POLARSSL_CIPHER_CAMELLIA_256_CBC                  MBEDTLS_CIPHER_CAMELLIA_256_CBC
        #define POLARSSL_CIPHER_CAMELLIA_256_CCM                  MBEDTLS_CIPHER_CAMELLIA_256_CCM
        #define POLARSSL_CIPHER_CAMELLIA_256_CFB128               MBEDTLS_CIPHER_CAMELLIA_256_CFB128
        #define POLARSSL_CIPHER_CAMELLIA_256_CTR                  MBEDTLS_CIPHER_CAMELLIA_256_CTR
        #define POLARSSL_CIPHER_CAMELLIA_256_ECB                  MBEDTLS_CIPHER_CAMELLIA_256_ECB
        #define POLARSSL_CIPHER_CAMELLIA_256_GCM                  MBEDTLS_CIPHER_CAMELLIA_256_GCM
        #define POLARSSL_CIPHER_DES_CBC                           MBEDTLS_CIPHER_DES_CBC
        #define POLARSSL_CIPHER_DES_ECB                           MBEDTLS_CIPHER_DES_ECB
        #define POLARSSL_CIPHER_DES_EDE3_CBC                      MBEDTLS_CIPHER_DES_EDE3_CBC
        #define POLARSSL_CIPHER_DES_EDE3_ECB                      MBEDTLS_CIPHER_DES_EDE3_ECB
        #define POLARSSL_CIPHER_DES_EDE_CBC                       MBEDTLS_CIPHER_DES_EDE_CBC
        #define POLARSSL_CIPHER_DES_EDE_ECB                       MBEDTLS_CIPHER_DES_EDE_ECB
        #define POLARSSL_CIPHER_H                                 MBEDTLS_CIPHER_H
        #define POLARSSL_CIPHER_ID_3DES                           MBEDTLS_CIPHER_ID_3DES
        #define POLARSSL_CIPHER_ID_AES                            MBEDTLS_CIPHER_ID_AES
        #define POLARSSL_CIPHER_ID_ARC4                           MBEDTLS_CIPHER_ID_ARC4
        #define POLARSSL_CIPHER_ID_BLOWFISH                       MBEDTLS_CIPHER_ID_BLOWFISH
        #define POLARSSL_CIPHER_ID_CAMELLIA                       MBEDTLS_CIPHER_ID_CAMELLIA
        #define POLARSSL_CIPHER_ID_DES                            MBEDTLS_CIPHER_ID_DES
        #define POLARSSL_CIPHER_ID_NONE                           MBEDTLS_CIPHER_ID_NONE
        #define POLARSSL_CIPHER_ID_NULL                           MBEDTLS_CIPHER_ID_NULL
        #define POLARSSL_CIPHER_MODE_AEAD                         MBEDTLS_CIPHER_MODE_AEAD
        #define POLARSSL_CIPHER_MODE_STREAM                       MBEDTLS_CIPHER_MODE_STREAM
        #define POLARSSL_CIPHER_MODE_WITH_PADDING                 MBEDTLS_CIPHER_MODE_WITH_PADDING
        #define POLARSSL_CIPHER_NONE                              MBEDTLS_CIPHER_NONE
        #define POLARSSL_CIPHER_NULL                              MBEDTLS_CIPHER_NULL
        #define POLARSSL_CIPHER_VARIABLE_IV_LEN                   MBEDTLS_CIPHER_VARIABLE_IV_LEN
        #define POLARSSL_CIPHER_VARIABLE_KEY_LEN                  MBEDTLS_CIPHER_VARIABLE_KEY_LEN
        #define POLARSSL_CIPHER_WRAP_H                            MBEDTLS_CIPHER_WRAP_H
        #define POLARSSL_CONFIG_H                                 MBEDTLS_CONFIG_H
        #define POLARSSL_CTR_DRBG_H                               MBEDTLS_CTR_DRBG_H
        #define POLARSSL_DEBUG_H                                  MBEDTLS_DEBUG_H
        #define POLARSSL_DECRYPT                                  MBEDTLS_DECRYPT
        #define POLARSSL_DES_H                                    MBEDTLS_DES_H
        #define POLARSSL_DHM_H                                    MBEDTLS_DHM_H
        #define POLARSSL_DHM_RFC3526_MODP_2048_G                  MBEDTLS_DHM_RFC3526_MODP_2048_G
        #define POLARSSL_DHM_RFC3526_MODP_2048_P                  MBEDTLS_DHM_RFC3526_MODP_2048_P
        #define POLARSSL_DHM_RFC3526_MODP_3072_G                  MBEDTLS_DHM_RFC3526_MODP_3072_G
        #define POLARSSL_DHM_RFC3526_MODP_3072_P                  MBEDTLS_DHM_RFC3526_MODP_3072_P
        #define POLARSSL_DHM_RFC5114_MODP_2048_G                  MBEDTLS_DHM_RFC5114_MODP_2048_G
        #define POLARSSL_DHM_RFC5114_MODP_2048_P                  MBEDTLS_DHM_RFC5114_MODP_2048_P
        #define POLARSSL_ECDH_H                                   MBEDTLS_ECDH_H
        #define POLARSSL_ECDH_OURS                                MBEDTLS_ECDH_OURS
        #define POLARSSL_ECDH_THEIRS                              MBEDTLS_ECDH_THEIRS
        #define POLARSSL_ECDSA_H                                  MBEDTLS_ECDSA_H
        #define POLARSSL_ECP_DP_BP256R1                           MBEDTLS_ECP_DP_BP256R1
        #define POLARSSL_ECP_DP_BP384R1                           MBEDTLS_ECP_DP_BP384R1
        #define POLARSSL_ECP_DP_BP512R1                           MBEDTLS_ECP_DP_BP512R1
        #define POLARSSL_ECP_DP_M255                              MBEDTLS_ECP_DP_CURVE25519
        #define POLARSSL_ECP_DP_MAX                               MBEDTLS_ECP_DP_MAX
        #define POLARSSL_ECP_DP_NONE                              MBEDTLS_ECP_DP_NONE
        #define POLARSSL_ECP_DP_SECP192K1                         MBEDTLS_ECP_DP_SECP192K1
        #define POLARSSL_ECP_DP_SECP192R1                         MBEDTLS_ECP_DP_SECP192R1
        #define POLARSSL_ECP_DP_SECP224K1                         MBEDTLS_ECP_DP_SECP224K1
        #define POLARSSL_ECP_DP_SECP224R1                         MBEDTLS_ECP_DP_SECP224R1
        #define POLARSSL_ECP_DP_SECP256K1                         MBEDTLS_ECP_DP_SECP256K1
        #define POLARSSL_ECP_DP_SECP256R1                         MBEDTLS_ECP_DP_SECP256R1
        #define POLARSSL_ECP_DP_SECP384R1                         MBEDTLS_ECP_DP_SECP384R1
        #define POLARSSL_ECP_DP_SECP521R1                         MBEDTLS_ECP_DP_SECP521R1
        #define POLARSSL_ECP_H                                    MBEDTLS_ECP_H
        #define POLARSSL_ECP_MAX_BYTES                            MBEDTLS_ECP_MAX_BYTES
        #define POLARSSL_ECP_MAX_PT_LEN                           MBEDTLS_ECP_MAX_PT_LEN
        #define POLARSSL_ECP_PF_COMPRESSED                        MBEDTLS_ECP_PF_COMPRESSED
        #define POLARSSL_ECP_PF_UNCOMPRESSED                      MBEDTLS_ECP_PF_UNCOMPRESSED
        #define POLARSSL_ECP_TLS_NAMED_CURVE                      MBEDTLS_ECP_TLS_NAMED_CURVE
        #define POLARSSL_ENCRYPT                                  MBEDTLS_ENCRYPT
        #define POLARSSL_ENTROPY_H                                MBEDTLS_ENTROPY_H
        #define POLARSSL_ENTROPY_POLL_H                           MBEDTLS_ENTROPY_POLL_H
        #define POLARSSL_ENTROPY_SHA256_ACCUMULATOR               MBEDTLS_ENTROPY_SHA256_ACCUMULATOR
        #define POLARSSL_ENTROPY_SHA512_ACCUMULATOR               MBEDTLS_ENTROPY_SHA512_ACCUMULATOR
        #define POLARSSL_ERROR_H                                  MBEDTLS_ERROR_H
        #define POLARSSL_ERR_AES_INVALID_INPUT_LENGTH             MBEDTLS_ERR_AES_INVALID_INPUT_LENGTH
        #define POLARSSL_ERR_AES_INVALID_KEY_LENGTH               MBEDTLS_ERR_AES_INVALID_KEY_LENGTH
        #define POLARSSL_ERR_ASN1_BUF_TOO_SMALL                   MBEDTLS_ERR_ASN1_BUF_TOO_SMALL
        #define POLARSSL_ERR_ASN1_INVALID_DATA                    MBEDTLS_ERR_ASN1_INVALID_DATA
        #define POLARSSL_ERR_ASN1_INVALID_LENGTH                  MBEDTLS_ERR_ASN1_INVALID_LENGTH
        #define POLARSSL_ERR_ASN1_LENGTH_MISMATCH                 MBEDTLS_ERR_ASN1_LENGTH_MISMATCH
        #define POLARSSL_ERR_ASN1_MALLOC_FAILED                   MBEDTLS_ERR_ASN1_ALLOC_FAILED
        #define POLARSSL_ERR_ASN1_OUT_OF_DATA                     MBEDTLS_ERR_ASN1_OUT_OF_DATA
        #define POLARSSL_ERR_ASN1_UNEXPECTED_TAG                  MBEDTLS_ERR_ASN1_UNEXPECTED_TAG
        #define POLARSSL_ERR_BASE64_BUFFER_TOO_SMALL              MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL
        #define POLARSSL_ERR_BASE64_INVALID_CHARACTER             MBEDTLS_ERR_BASE64_INVALID_CHARACTER
        #define POLARSSL_ERR_BLOWFISH_INVALID_INPUT_LENGTH        MBEDTLS_ERR_BLOWFISH_INVALID_INPUT_LENGTH
        #define POLARSSL_ERR_BLOWFISH_INVALID_KEY_LENGTH          MBEDTLS_ERR_BLOWFISH_INVALID_KEY_LENGTH
        #define POLARSSL_ERR_CAMELLIA_INVALID_INPUT_LENGTH        MBEDTLS_ERR_CAMELLIA_INVALID_INPUT_LENGTH
        #define POLARSSL_ERR_CAMELLIA_INVALID_KEY_LENGTH          MBEDTLS_ERR_CAMELLIA_INVALID_KEY_LENGTH
        #define POLARSSL_ERR_CCM_AUTH_FAILED                      MBEDTLS_ERR_CCM_AUTH_FAILED
        #define POLARSSL_ERR_CCM_BAD_INPUT                        MBEDTLS_ERR_CCM_BAD_INPUT
        #define POLARSSL_ERR_CIPHER_ALLOC_FAILED                  MBEDTLS_ERR_CIPHER_ALLOC_FAILED
        #define POLARSSL_ERR_CIPHER_AUTH_FAILED                   MBEDTLS_ERR_CIPHER_AUTH_FAILED
        #define POLARSSL_ERR_CIPHER_BAD_INPUT_DATA                MBEDTLS_ERR_CIPHER_BAD_INPUT_DATA
        #define POLARSSL_ERR_CIPHER_FEATURE_UNAVAILABLE           MBEDTLS_ERR_CIPHER_FEATURE_UNAVAILABLE
        #define POLARSSL_ERR_CIPHER_FULL_BLOCK_EXPECTED           MBEDTLS_ERR_CIPHER_FULL_BLOCK_EXPECTED
        #define POLARSSL_ERR_CIPHER_INVALID_PADDING               MBEDTLS_ERR_CIPHER_INVALID_PADDING
        #define POLARSSL_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED       MBEDTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED
        #define POLARSSL_ERR_CTR_DRBG_FILE_IO_ERROR               MBEDTLS_ERR_CTR_DRBG_FILE_IO_ERROR
        #define POLARSSL_ERR_CTR_DRBG_INPUT_TOO_BIG               MBEDTLS_ERR_CTR_DRBG_INPUT_TOO_BIG
        #define POLARSSL_ERR_CTR_DRBG_REQUEST_TOO_BIG             MBEDTLS_ERR_CTR_DRBG_REQUEST_TOO_BIG
        #define POLARSSL_ERR_DES_INVALID_INPUT_LENGTH             MBEDTLS_ERR_DES_INVALID_INPUT_LENGTH
        #define POLARSSL_ERR_DHM_BAD_INPUT_DATA                   MBEDTLS_ERR_DHM_BAD_INPUT_DATA
        #define POLARSSL_ERR_DHM_CALC_SECRET_FAILED               MBEDTLS_ERR_DHM_CALC_SECRET_FAILED
        #define POLARSSL_ERR_DHM_FILE_IO_ERROR                    MBEDTLS_ERR_DHM_FILE_IO_ERROR
        #define POLARSSL_ERR_DHM_INVALID_FORMAT                   MBEDTLS_ERR_DHM_INVALID_FORMAT
        #define POLARSSL_ERR_DHM_MAKE_PARAMS_FAILED               MBEDTLS_ERR_DHM_MAKE_PARAMS_FAILED
        #define POLARSSL_ERR_DHM_MAKE_PUBLIC_FAILED               MBEDTLS_ERR_DHM_MAKE_PUBLIC_FAILED
        #define POLARSSL_ERR_DHM_MALLOC_FAILED                    MBEDTLS_ERR_DHM_ALLOC_FAILED
        #define POLARSSL_ERR_DHM_READ_PARAMS_FAILED               MBEDTLS_ERR_DHM_READ_PARAMS_FAILED
        #define POLARSSL_ERR_DHM_READ_PUBLIC_FAILED               MBEDTLS_ERR_DHM_READ_PUBLIC_FAILED
        #define POLARSSL_ERR_ECP_BAD_INPUT_DATA                   MBEDTLS_ERR_ECP_BAD_INPUT_DATA
        #define POLARSSL_ERR_ECP_BUFFER_TOO_SMALL                 MBEDTLS_ERR_ECP_BUFFER_TOO_SMALL
        #define POLARSSL_ERR_ECP_FEATURE_UNAVAILABLE              MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE
        #define POLARSSL_ERR_ECP_INVALID_KEY                      MBEDTLS_ERR_ECP_INVALID_KEY
        #define POLARSSL_ERR_ECP_MALLOC_FAILED                    MBEDTLS_ERR_ECP_ALLOC_FAILED
        #define POLARSSL_ERR_ECP_RANDOM_FAILED                    MBEDTLS_ERR_ECP_RANDOM_FAILED
        #define POLARSSL_ERR_ECP_SIG_LEN_MISMATCH                 MBEDTLS_ERR_ECP_SIG_LEN_MISMATCH
        #define POLARSSL_ERR_ECP_VERIFY_FAILED                    MBEDTLS_ERR_ECP_VERIFY_FAILED
        #define POLARSSL_ERR_ENTROPY_FILE_IO_ERROR                MBEDTLS_ERR_ENTROPY_FILE_IO_ERROR
        #define POLARSSL_ERR_ENTROPY_MAX_SOURCES                  MBEDTLS_ERR_ENTROPY_MAX_SOURCES
        #define POLARSSL_ERR_ENTROPY_NO_SOURCES_DEFINED           MBEDTLS_ERR_ENTROPY_NO_SOURCES_DEFINED
        #define POLARSSL_ERR_ENTROPY_SOURCE_FAILED                MBEDTLS_ERR_ENTROPY_SOURCE_FAILED
        #define POLARSSL_ERR_GCM_AUTH_FAILED                      MBEDTLS_ERR_GCM_AUTH_FAILED
        #define POLARSSL_ERR_GCM_BAD_INPUT                        MBEDTLS_ERR_GCM_BAD_INPUT
        #define POLARSSL_ERR_HMAC_DRBG_ENTROPY_SOURCE_FAILED      MBEDTLS_ERR_HMAC_DRBG_ENTROPY_SOURCE_FAILED
        #define POLARSSL_ERR_HMAC_DRBG_FILE_IO_ERROR              MBEDTLS_ERR_HMAC_DRBG_FILE_IO_ERROR
        #define POLARSSL_ERR_HMAC_DRBG_INPUT_TOO_BIG              MBEDTLS_ERR_HMAC_DRBG_INPUT_TOO_BIG
        #define POLARSSL_ERR_HMAC_DRBG_REQUEST_TOO_BIG            MBEDTLS_ERR_HMAC_DRBG_REQUEST_TOO_BIG
        #define POLARSSL_ERR_MD_ALLOC_FAILED                      MBEDTLS_ERR_MD_ALLOC_FAILED
        #define POLARSSL_ERR_MD_BAD_INPUT_DATA                    MBEDTLS_ERR_MD_BAD_INPUT_DATA
        #define POLARSSL_ERR_MD_FEATURE_UNAVAILABLE               MBEDTLS_ERR_MD_FEATURE_UNAVAILABLE
        #define POLARSSL_ERR_MD_FILE_IO_ERROR                     MBEDTLS_ERR_MD_FILE_IO_ERROR
        #define POLARSSL_ERR_MPI_BAD_INPUT_DATA                   MBEDTLS_ERR_MPI_BAD_INPUT_DATA
        #define POLARSSL_ERR_MPI_BUFFER_TOO_SMALL                 MBEDTLS_ERR_MPI_BUFFER_TOO_SMALL
        #define POLARSSL_ERR_MPI_DIVISION_BY_ZERO                 MBEDTLS_ERR_MPI_DIVISION_BY_ZERO
        #define POLARSSL_ERR_MPI_FILE_IO_ERROR                    MBEDTLS_ERR_MPI_FILE_IO_ERROR
        #define POLARSSL_ERR_MPI_INVALID_CHARACTER                MBEDTLS_ERR_MPI_INVALID_CHARACTER
        #define POLARSSL_ERR_MPI_MALLOC_FAILED                    MBEDTLS_ERR_MPI_ALLOC_FAILED
        #define POLARSSL_ERR_MPI_NEGATIVE_VALUE                   MBEDTLS_ERR_MPI_NEGATIVE_VALUE
        #define POLARSSL_ERR_MPI_NOT_ACCEPTABLE                   MBEDTLS_ERR_MPI_NOT_ACCEPTABLE
        #define POLARSSL_ERR_NET_ACCEPT_FAILED                    MBEDTLS_ERR_NET_ACCEPT_FAILED
        #define POLARSSL_ERR_NET_BIND_FAILED                      MBEDTLS_ERR_NET_BIND_FAILED
        #define POLARSSL_ERR_NET_CONNECT_FAILED                   MBEDTLS_ERR_NET_CONNECT_FAILED
        #define POLARSSL_ERR_NET_CONN_RESET                       MBEDTLS_ERR_NET_CONN_RESET
        #define POLARSSL_ERR_NET_LISTEN_FAILED                    MBEDTLS_ERR_NET_LISTEN_FAILED
        #define POLARSSL_ERR_NET_RECV_FAILED                      MBEDTLS_ERR_NET_RECV_FAILED
        #define POLARSSL_ERR_NET_SEND_FAILED                      MBEDTLS_ERR_NET_SEND_FAILED
        #define POLARSSL_ERR_NET_SOCKET_FAILED                    MBEDTLS_ERR_NET_SOCKET_FAILED
        #define POLARSSL_ERR_NET_TIMEOUT                          MBEDTLS_ERR_SSL_TIMEOUT
        #define POLARSSL_ERR_NET_UNKNOWN_HOST                     MBEDTLS_ERR_NET_UNKNOWN_HOST
        #define POLARSSL_ERR_NET_WANT_READ                        MBEDTLS_ERR_SSL_WANT_READ
        #define POLARSSL_ERR_NET_WANT_WRITE                       MBEDTLS_ERR_SSL_WANT_WRITE
        #define POLARSSL_ERR_OID_BUF_TOO_SMALL                    MBEDTLS_ERR_OID_BUF_TOO_SMALL
        #define POLARSSL_ERR_OID_NOT_FOUND                        MBEDTLS_ERR_OID_NOT_FOUND
        #define POLARSSL_ERR_PADLOCK_DATA_MISALIGNED              MBEDTLS_ERR_PADLOCK_DATA_MISALIGNED
        #define POLARSSL_ERR_PEM_BAD_INPUT_DATA                   MBEDTLS_ERR_PEM_BAD_INPUT_DATA
        #define POLARSSL_ERR_PEM_FEATURE_UNAVAILABLE              MBEDTLS_ERR_PEM_FEATURE_UNAVAILABLE
        #define POLARSSL_ERR_PEM_INVALID_DATA                     MBEDTLS_ERR_PEM_INVALID_DATA
        #define POLARSSL_ERR_PEM_INVALID_ENC_IV                   MBEDTLS_ERR_PEM_INVALID_ENC_IV
        #define POLARSSL_ERR_PEM_MALLOC_FAILED                    MBEDTLS_ERR_PEM_ALLOC_FAILED
        #define POLARSSL_ERR_PEM_NO_HEADER_FOOTER_PRESENT         MBEDTLS_ERR_PEM_NO_HEADER_FOOTER_PRESENT
        #define POLARSSL_ERR_PEM_PASSWORD_MISMATCH                MBEDTLS_ERR_PEM_PASSWORD_MISMATCH
        #define POLARSSL_ERR_PEM_PASSWORD_REQUIRED                MBEDTLS_ERR_PEM_PASSWORD_REQUIRED
        #define POLARSSL_ERR_PEM_UNKNOWN_ENC_ALG                  MBEDTLS_ERR_PEM_UNKNOWN_ENC_ALG
        #define POLARSSL_ERR_PKCS12_BAD_INPUT_DATA                MBEDTLS_ERR_PKCS12_BAD_INPUT_DATA
        #define POLARSSL_ERR_PKCS12_FEATURE_UNAVAILABLE           MBEDTLS_ERR_PKCS12_FEATURE_UNAVAILABLE
        #define POLARSSL_ERR_PKCS12_PASSWORD_MISMATCH             MBEDTLS_ERR_PKCS12_PASSWORD_MISMATCH
        #define POLARSSL_ERR_PKCS12_PBE_INVALID_FORMAT            MBEDTLS_ERR_PKCS12_PBE_INVALID_FORMAT
        #define POLARSSL_ERR_PKCS5_BAD_INPUT_DATA                 MBEDTLS_ERR_PKCS5_BAD_INPUT_DATA
        #define POLARSSL_ERR_PKCS5_FEATURE_UNAVAILABLE            MBEDTLS_ERR_PKCS5_FEATURE_UNAVAILABLE
        #define POLARSSL_ERR_PKCS5_INVALID_FORMAT                 MBEDTLS_ERR_PKCS5_INVALID_FORMAT
        #define POLARSSL_ERR_PKCS5_PASSWORD_MISMATCH              MBEDTLS_ERR_PKCS5_PASSWORD_MISMATCH
        #define POLARSSL_ERR_PK_BAD_INPUT_DATA                    MBEDTLS_ERR_PK_BAD_INPUT_DATA
        #define POLARSSL_ERR_PK_FEATURE_UNAVAILABLE               MBEDTLS_ERR_PK_FEATURE_UNAVAILABLE
        #define POLARSSL_ERR_PK_FILE_IO_ERROR                     MBEDTLS_ERR_PK_FILE_IO_ERROR
        #define POLARSSL_ERR_PK_INVALID_ALG                       MBEDTLS_ERR_PK_INVALID_ALG
        #define POLARSSL_ERR_PK_INVALID_PUBKEY                    MBEDTLS_ERR_PK_INVALID_PUBKEY
        #define POLARSSL_ERR_PK_KEY_INVALID_FORMAT                MBEDTLS_ERR_PK_KEY_INVALID_FORMAT
        #define POLARSSL_ERR_PK_KEY_INVALID_VERSION               MBEDTLS_ERR_PK_KEY_INVALID_VERSION
        #define POLARSSL_ERR_PK_MALLOC_FAILED                     MBEDTLS_ERR_PK_ALLOC_FAILED
        #define POLARSSL_ERR_PK_PASSWORD_MISMATCH                 MBEDTLS_ERR_PK_PASSWORD_MISMATCH
        #define POLARSSL_ERR_PK_PASSWORD_REQUIRED                 MBEDTLS_ERR_PK_PASSWORD_REQUIRED
        #define POLARSSL_ERR_PK_SIG_LEN_MISMATCH                  MBEDTLS_ERR_PK_SIG_LEN_MISMATCH
        #define POLARSSL_ERR_PK_TYPE_MISMATCH                     MBEDTLS_ERR_PK_TYPE_MISMATCH
        #define POLARSSL_ERR_PK_UNKNOWN_NAMED_CURVE               MBEDTLS_ERR_PK_UNKNOWN_NAMED_CURVE
        #define POLARSSL_ERR_PK_UNKNOWN_PK_ALG                    MBEDTLS_ERR_PK_UNKNOWN_PK_ALG
        #define POLARSSL_ERR_RSA_BAD_INPUT_DATA                   MBEDTLS_ERR_RSA_BAD_INPUT_DATA
        #define POLARSSL_ERR_RSA_INVALID_PADDING                  MBEDTLS_ERR_RSA_INVALID_PADDING
        #define POLARSSL_ERR_RSA_KEY_CHECK_FAILED                 MBEDTLS_ERR_RSA_KEY_CHECK_FAILED
        #define POLARSSL_ERR_RSA_KEY_GEN_FAILED                   MBEDTLS_ERR_RSA_KEY_GEN_FAILED
        #define POLARSSL_ERR_RSA_OUTPUT_TOO_LARGE                 MBEDTLS_ERR_RSA_OUTPUT_TOO_LARGE
        #define POLARSSL_ERR_RSA_PRIVATE_FAILED                   MBEDTLS_ERR_RSA_PRIVATE_FAILED
        #define POLARSSL_ERR_RSA_PUBLIC_FAILED                    MBEDTLS_ERR_RSA_PUBLIC_FAILED
        #define POLARSSL_ERR_RSA_RNG_FAILED                       MBEDTLS_ERR_RSA_RNG_FAILED
        #define POLARSSL_ERR_RSA_VERIFY_FAILED                    MBEDTLS_ERR_RSA_VERIFY_FAILED
        #define POLARSSL_ERR_SSL_BAD_HS_CERTIFICATE               MBEDTLS_ERR_SSL_BAD_HS_CERTIFICATE
        #define POLARSSL_ERR_SSL_BAD_HS_CERTIFICATE_REQUEST       MBEDTLS_ERR_SSL_BAD_HS_CERTIFICATE_REQUEST
        #define POLARSSL_ERR_SSL_BAD_HS_CERTIFICATE_VERIFY        MBEDTLS_ERR_SSL_BAD_HS_CERTIFICATE_VERIFY
        #define POLARSSL_ERR_SSL_BAD_HS_CHANGE_CIPHER_SPEC        MBEDTLS_ERR_SSL_BAD_HS_CHANGE_CIPHER_SPEC
        #define POLARSSL_ERR_SSL_BAD_HS_CLIENT_HELLO              MBEDTLS_ERR_SSL_BAD_HS_CLIENT_HELLO
        #define POLARSSL_ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE       MBEDTLS_ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE
        #define POLARSSL_ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE_CS    MBEDTLS_ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE_CS
        #define POLARSSL_ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE_RP    MBEDTLS_ERR_SSL_BAD_HS_CLIENT_KEY_EXCHANGE_RP
        #define POLARSSL_ERR_SSL_BAD_HS_FINISHED                  MBEDTLS_ERR_SSL_BAD_HS_FINISHED
        #define POLARSSL_ERR_SSL_BAD_HS_NEW_SESSION_TICKET        MBEDTLS_ERR_SSL_BAD_HS_NEW_SESSION_TICKET
        #define POLARSSL_ERR_SSL_BAD_HS_PROTOCOL_VERSION          MBEDTLS_ERR_SSL_BAD_HS_PROTOCOL_VERSION
        #define POLARSSL_ERR_SSL_BAD_HS_SERVER_HELLO              MBEDTLS_ERR_SSL_BAD_HS_SERVER_HELLO
        #define POLARSSL_ERR_SSL_BAD_HS_SERVER_HELLO_DONE         MBEDTLS_ERR_SSL_BAD_HS_SERVER_HELLO_DONE
        #define POLARSSL_ERR_SSL_BAD_HS_SERVER_KEY_EXCHANGE       MBEDTLS_ERR_SSL_BAD_HS_SERVER_KEY_EXCHANGE
        #define POLARSSL_ERR_SSL_BAD_INPUT_DATA                   MBEDTLS_ERR_SSL_BAD_INPUT_DATA
        #define POLARSSL_ERR_SSL_BUFFER_TOO_SMALL                 MBEDTLS_ERR_SSL_BUFFER_TOO_SMALL
        #define POLARSSL_ERR_SSL_CA_CHAIN_REQUIRED                MBEDTLS_ERR_SSL_CA_CHAIN_REQUIRED
        #define POLARSSL_ERR_SSL_CERTIFICATE_REQUIRED             MBEDTLS_ERR_SSL_CERTIFICATE_REQUIRED
        #define POLARSSL_ERR_SSL_CERTIFICATE_TOO_LARGE            MBEDTLS_ERR_SSL_CERTIFICATE_TOO_LARGE
        #define POLARSSL_ERR_SSL_COMPRESSION_FAILED               MBEDTLS_ERR_SSL_COMPRESSION_FAILED
        #define POLARSSL_ERR_SSL_CONN_EOF                         MBEDTLS_ERR_SSL_CONN_EOF
        #define POLARSSL_ERR_SSL_COUNTER_WRAPPING                 MBEDTLS_ERR_SSL_COUNTER_WRAPPING
        #define POLARSSL_ERR_SSL_FATAL_ALERT_MESSAGE              MBEDTLS_ERR_SSL_FATAL_ALERT_MESSAGE
        #define POLARSSL_ERR_SSL_FEATURE_UNAVAILABLE              MBEDTLS_ERR_SSL_FEATURE_UNAVAILABLE
        #define POLARSSL_ERR_SSL_HELLO_VERIFY_REQUIRED            MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED
        #define POLARSSL_ERR_SSL_HW_ACCEL_FAILED                  MBEDTLS_ERR_SSL_HW_ACCEL_FAILED
        #define POLARSSL_ERR_SSL_HW_ACCEL_FALLTHROUGH             MBEDTLS_ERR_SSL_HW_ACCEL_FALLTHROUGH
        #define POLARSSL_ERR_SSL_INTERNAL_ERROR                   MBEDTLS_ERR_SSL_INTERNAL_ERROR
        #define POLARSSL_ERR_SSL_INVALID_MAC                      MBEDTLS_ERR_SSL_INVALID_MAC
        #define POLARSSL_ERR_SSL_INVALID_RECORD                   MBEDTLS_ERR_SSL_INVALID_RECORD
        #define POLARSSL_ERR_SSL_MALLOC_FAILED                    MBEDTLS_ERR_SSL_ALLOC_FAILED
        #define POLARSSL_ERR_SSL_NO_CIPHER_CHOSEN                 MBEDTLS_ERR_SSL_NO_CIPHER_CHOSEN
        #define POLARSSL_ERR_SSL_NO_CLIENT_CERTIFICATE            MBEDTLS_ERR_SSL_NO_CLIENT_CERTIFICATE
        #define POLARSSL_ERR_SSL_NO_RNG                           MBEDTLS_ERR_SSL_NO_RNG
        #define POLARSSL_ERR_SSL_NO_USABLE_CIPHERSUITE            MBEDTLS_ERR_SSL_NO_USABLE_CIPHERSUITE
        #define POLARSSL_ERR_SSL_PEER_CLOSE_NOTIFY                MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY
        #define POLARSSL_ERR_SSL_PEER_VERIFY_FAILED               MBEDTLS_ERR_SSL_PEER_VERIFY_FAILED
        #define POLARSSL_ERR_SSL_PK_TYPE_MISMATCH                 MBEDTLS_ERR_SSL_PK_TYPE_MISMATCH
        #define POLARSSL_ERR_SSL_PRIVATE_KEY_REQUIRED             MBEDTLS_ERR_SSL_PRIVATE_KEY_REQUIRED
        #define POLARSSL_ERR_SSL_SESSION_TICKET_EXPIRED           MBEDTLS_ERR_SSL_SESSION_TICKET_EXPIRED
        #define POLARSSL_ERR_SSL_UNEXPECTED_MESSAGE               MBEDTLS_ERR_SSL_UNEXPECTED_MESSAGE
        #define POLARSSL_ERR_SSL_UNKNOWN_CIPHER                   MBEDTLS_ERR_SSL_UNKNOWN_CIPHER
        #define POLARSSL_ERR_SSL_UNKNOWN_IDENTITY                 MBEDTLS_ERR_SSL_UNKNOWN_IDENTITY
        #define POLARSSL_ERR_SSL_WAITING_SERVER_HELLO_RENEGO      MBEDTLS_ERR_SSL_WAITING_SERVER_HELLO_RENEGO
        #define POLARSSL_ERR_THREADING_BAD_INPUT_DATA             MBEDTLS_ERR_THREADING_BAD_INPUT_DATA
        #define POLARSSL_ERR_THREADING_FEATURE_UNAVAILABLE        MBEDTLS_ERR_THREADING_FEATURE_UNAVAILABLE
        #define POLARSSL_ERR_THREADING_MUTEX_ERROR                MBEDTLS_ERR_THREADING_MUTEX_ERROR
        #define POLARSSL_ERR_X509_BAD_INPUT_DATA                  MBEDTLS_ERR_X509_BAD_INPUT_DATA
        #define POLARSSL_ERR_X509_CERT_UNKNOWN_FORMAT             MBEDTLS_ERR_X509_CERT_UNKNOWN_FORMAT
        #define POLARSSL_ERR_X509_CERT_VERIFY_FAILED              MBEDTLS_ERR_X509_CERT_VERIFY_FAILED
        #define POLARSSL_ERR_X509_FEATURE_UNAVAILABLE             MBEDTLS_ERR_X509_FEATURE_UNAVAILABLE
        #define POLARSSL_ERR_X509_FILE_IO_ERROR                   MBEDTLS_ERR_X509_FILE_IO_ERROR
        #define POLARSSL_ERR_X509_INVALID_ALG                     MBEDTLS_ERR_X509_INVALID_ALG
        #define POLARSSL_ERR_X509_INVALID_DATE                    MBEDTLS_ERR_X509_INVALID_DATE
        #define POLARSSL_ERR_X509_INVALID_EXTENSIONS              MBEDTLS_ERR_X509_INVALID_EXTENSIONS
        #define POLARSSL_ERR_X509_INVALID_FORMAT                  MBEDTLS_ERR_X509_INVALID_FORMAT
        #define POLARSSL_ERR_X509_INVALID_NAME                    MBEDTLS_ERR_X509_INVALID_NAME
        #define POLARSSL_ERR_X509_INVALID_SERIAL                  MBEDTLS_ERR_X509_INVALID_SERIAL
        #define POLARSSL_ERR_X509_INVALID_SIGNATURE               MBEDTLS_ERR_X509_INVALID_SIGNATURE
        #define POLARSSL_ERR_X509_INVALID_VERSION                 MBEDTLS_ERR_X509_INVALID_VERSION
        #define POLARSSL_ERR_X509_MALLOC_FAILED                   MBEDTLS_ERR_X509_ALLOC_FAILED
        #define POLARSSL_ERR_X509_SIG_MISMATCH                    MBEDTLS_ERR_X509_SIG_MISMATCH
        #define POLARSSL_ERR_X509_UNKNOWN_OID                     MBEDTLS_ERR_X509_UNKNOWN_OID
        #define POLARSSL_ERR_X509_UNKNOWN_SIG_ALG                 MBEDTLS_ERR_X509_UNKNOWN_SIG_ALG
        #define POLARSSL_ERR_X509_UNKNOWN_VERSION                 MBEDTLS_ERR_X509_UNKNOWN_VERSION
        #define POLARSSL_ERR_XTEA_INVALID_INPUT_LENGTH            MBEDTLS_ERR_XTEA_INVALID_INPUT_LENGTH
        #define POLARSSL_GCM_H                                    MBEDTLS_GCM_H
        #define POLARSSL_HAVEGE_H                                 MBEDTLS_HAVEGE_H
        #define POLARSSL_HAVE_INT32                               MBEDTLS_HAVE_INT32
        #define POLARSSL_HAVE_INT64                               MBEDTLS_HAVE_INT64
        #define POLARSSL_HAVE_UDBL                                MBEDTLS_HAVE_UDBL
        #define POLARSSL_HAVE_X86                                 MBEDTLS_HAVE_X86
        #define POLARSSL_HAVE_X86_64                              MBEDTLS_HAVE_X86_64
        #define POLARSSL_HMAC_DRBG_H                              MBEDTLS_HMAC_DRBG_H
        #define POLARSSL_HMAC_DRBG_PR_OFF                         MBEDTLS_HMAC_DRBG_PR_OFF
        #define POLARSSL_HMAC_DRBG_PR_ON                          MBEDTLS_HMAC_DRBG_PR_ON
        #define POLARSSL_KEY_EXCHANGE_DHE_PSK                     MBEDTLS_KEY_EXCHANGE_DHE_PSK
        #define POLARSSL_KEY_EXCHANGE_DHE_RSA                     MBEDTLS_KEY_EXCHANGE_DHE_RSA
        #define POLARSSL_KEY_EXCHANGE_ECDHE_ECDSA                 MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA
        #define POLARSSL_KEY_EXCHANGE_ECDHE_PSK                   MBEDTLS_KEY_EXCHANGE_ECDHE_PSK
        #define POLARSSL_KEY_EXCHANGE_ECDHE_RSA                   MBEDTLS_KEY_EXCHANGE_ECDHE_RSA
        #define POLARSSL_KEY_EXCHANGE_ECDH_ECDSA                  MBEDTLS_KEY_EXCHANGE_ECDH_ECDSA
        #define POLARSSL_KEY_EXCHANGE_ECDH_RSA                    MBEDTLS_KEY_EXCHANGE_ECDH_RSA
        #define POLARSSL_KEY_EXCHANGE_NONE                        MBEDTLS_KEY_EXCHANGE_NONE
        #define POLARSSL_KEY_EXCHANGE_PSK                         MBEDTLS_KEY_EXCHANGE_PSK
        #define POLARSSL_KEY_EXCHANGE_RSA                         MBEDTLS_KEY_EXCHANGE_RSA
        #define POLARSSL_KEY_EXCHANGE_RSA_PSK                     MBEDTLS_KEY_EXCHANGE_RSA_PSK
        #define POLARSSL_KEY_EXCHANGE__SOME__ECDHE_ENABLED        MBEDTLS_KEY_EXCHANGE__SOME__ECDHE_ENABLED
        #define POLARSSL_KEY_EXCHANGE__SOME__PSK_ENABLED          MBEDTLS_KEY_EXCHANGE__SOME__PSK_ENABLED
        #define POLARSSL_KEY_EXCHANGE__WITH_CERT__ENABLED         MBEDTLS_KEY_EXCHANGE__WITH_CERT__ENABLED
        #define POLARSSL_KEY_LENGTH_DES                           MBEDTLS_KEY_LENGTH_DES
        #define POLARSSL_KEY_LENGTH_DES_EDE                       MBEDTLS_KEY_LENGTH_DES_EDE
        #define POLARSSL_KEY_LENGTH_DES_EDE3                      MBEDTLS_KEY_LENGTH_DES_EDE3
        #define POLARSSL_KEY_LENGTH_NONE                          MBEDTLS_KEY_LENGTH_NONE
        #define POLARSSL_MAX_BLOCK_LENGTH                         MBEDTLS_MAX_BLOCK_LENGTH
        #define POLARSSL_MAX_IV_LENGTH                            MBEDTLS_MAX_IV_LENGTH
        #define POLARSSL_MD2_H                                    MBEDTLS_MD2_H
        #define POLARSSL_MD4_H                                    MBEDTLS_MD4_H
        #define POLARSSL_MD5_H                                    MBEDTLS_MD5_H
        #define POLARSSL_MD_H                                     MBEDTLS_MD_H
        #define POLARSSL_MD_MAX_SIZE                              MBEDTLS_MD_MAX_SIZE
        #define POLARSSL_MD_MD2                                   MBEDTLS_MD_MD2
        #define POLARSSL_MD_MD4                                   MBEDTLS_MD_MD4
        #define POLARSSL_MD_MD5                                   MBEDTLS_MD_MD5
        #define POLARSSL_MD_NONE                                  MBEDTLS_MD_NONE
        #define POLARSSL_MD_RIPEMD160                             MBEDTLS_MD_RIPEMD160
        #define POLARSSL_MD_SHA1                                  MBEDTLS_MD_SHA1
        #define POLARSSL_MD_SHA224                                MBEDTLS_MD_SHA224
        #define POLARSSL_MD_SHA256                                MBEDTLS_MD_SHA256
        #define POLARSSL_MD_SHA384                                MBEDTLS_MD_SHA384
        #define POLARSSL_MD_SHA512                                MBEDTLS_MD_SHA512
        #define POLARSSL_MD_WRAP_H                                MBEDTLS_MD_WRAP_H
        #define POLARSSL_MEMORY_BUFFER_ALLOC_H                    MBEDTLS_MEMORY_BUFFER_ALLOC_H
        #define POLARSSL_MODE_CBC                                 MBEDTLS_MODE_CBC
        #define POLARSSL_MODE_CCM                                 MBEDTLS_MODE_CCM
        #define POLARSSL_MODE_CFB                                 MBEDTLS_MODE_CFB
        #define POLARSSL_MODE_CTR                                 MBEDTLS_MODE_CTR
        #define POLARSSL_MODE_ECB                                 MBEDTLS_MODE_ECB
        #define POLARSSL_MODE_GCM                                 MBEDTLS_MODE_GCM
        #define POLARSSL_MODE_NONE                                MBEDTLS_MODE_NONE
        #define POLARSSL_MODE_OFB                                 MBEDTLS_MODE_OFB
        #define POLARSSL_MODE_STREAM                              MBEDTLS_MODE_STREAM
        #define POLARSSL_MPI_MAX_BITS                             MBEDTLS_MPI_MAX_BITS
        #define POLARSSL_MPI_MAX_BITS_SCALE100                    MBEDTLS_MPI_MAX_BITS_SCALE100
        #define POLARSSL_MPI_MAX_LIMBS                            MBEDTLS_MPI_MAX_LIMBS
        #define POLARSSL_MPI_RW_BUFFER_SIZE                       MBEDTLS_MPI_RW_BUFFER_SIZE
        #define POLARSSL_NET_H                                    MBEDTLS_NET_SOCKETS_H
        #define POLARSSL_NET_LISTEN_BACKLOG                       MBEDTLS_NET_LISTEN_BACKLOG
        #define POLARSSL_OID_H                                    MBEDTLS_OID_H
        #define POLARSSL_OPERATION_NONE                           MBEDTLS_OPERATION_NONE
        #define POLARSSL_PADDING_NONE                             MBEDTLS_PADDING_NONE
        #define POLARSSL_PADDING_ONE_AND_ZEROS                    MBEDTLS_PADDING_ONE_AND_ZEROS
        #define POLARSSL_PADDING_PKCS7                            MBEDTLS_PADDING_PKCS7
        #define POLARSSL_PADDING_ZEROS                            MBEDTLS_PADDING_ZEROS
        #define POLARSSL_PADDING_ZEROS_AND_LEN                    MBEDTLS_PADDING_ZEROS_AND_LEN
        #define POLARSSL_PADLOCK_H                                MBEDTLS_PADLOCK_H
        #define POLARSSL_PEM_H                                    MBEDTLS_PEM_H
        #define POLARSSL_PKCS11_H                                 MBEDTLS_PKCS11_H
        #define POLARSSL_PKCS12_H                                 MBEDTLS_PKCS12_H
        #define POLARSSL_PKCS5_H                                  MBEDTLS_PKCS5_H
        #define POLARSSL_PK_DEBUG_ECP                             MBEDTLS_PK_DEBUG_ECP
        #define POLARSSL_PK_DEBUG_MAX_ITEMS                       MBEDTLS_PK_DEBUG_MAX_ITEMS
        #define POLARSSL_PK_DEBUG_MPI                             MBEDTLS_PK_DEBUG_MPI
        #define POLARSSL_PK_DEBUG_NONE                            MBEDTLS_PK_DEBUG_NONE
        #define POLARSSL_PK_ECDSA                                 MBEDTLS_PK_ECDSA
        #define POLARSSL_PK_ECKEY                                 MBEDTLS_PK_ECKEY
        #define POLARSSL_PK_ECKEY_DH                              MBEDTLS_PK_ECKEY_DH
        #define POLARSSL_PK_H                                     MBEDTLS_PK_H
        #define POLARSSL_PK_NONE                                  MBEDTLS_PK_NONE
        #define POLARSSL_PK_RSA                                   MBEDTLS_PK_RSA
        #define POLARSSL_PK_RSASSA_PSS                            MBEDTLS_PK_RSASSA_PSS
        #define POLARSSL_PK_RSA_ALT                               MBEDTLS_PK_RSA_ALT
        #define POLARSSL_PK_WRAP_H                                MBEDTLS_PK_WRAP_H
        #define POLARSSL_PLATFORM_H                               MBEDTLS_PLATFORM_H
        #define POLARSSL_PREMASTER_SIZE                           MBEDTLS_PREMASTER_SIZE
        #define POLARSSL_RIPEMD160_H                              MBEDTLS_RIPEMD160_H
        #define POLARSSL_RSA_H                                    MBEDTLS_RSA_H
        #define POLARSSL_SHA1_H                                   MBEDTLS_SHA1_H
        #define POLARSSL_SHA256_H                                 MBEDTLS_SHA256_H
        #define POLARSSL_SHA512_H                                 MBEDTLS_SHA512_H
        #define POLARSSL_SSL_CACHE_H                              MBEDTLS_SSL_CACHE_H
        #define POLARSSL_SSL_CIPHERSUITES_H                       MBEDTLS_SSL_CIPHERSUITES_H
        #define POLARSSL_SSL_COOKIE_H                             MBEDTLS_SSL_COOKIE_H
        #define POLARSSL_SSL_H                                    MBEDTLS_SSL_H
        #define POLARSSL_THREADING_H                              MBEDTLS_THREADING_H
        #define POLARSSL_THREADING_IMPL                           MBEDTLS_THREADING_IMPL
        #define POLARSSL_TIMING_H                                 MBEDTLS_TIMING_H
        #define POLARSSL_VERSION_H                                MBEDTLS_VERSION_H
        #define POLARSSL_VERSION_MAJOR                            MBEDTLS_VERSION_MAJOR
        #define POLARSSL_VERSION_MINOR                            MBEDTLS_VERSION_MINOR
        #define POLARSSL_VERSION_NUMBER                           MBEDTLS_VERSION_NUMBER
        #define POLARSSL_VERSION_PATCH                            MBEDTLS_VERSION_PATCH
        #define POLARSSL_VERSION_STRING                           MBEDTLS_VERSION_STRING
        #define POLARSSL_VERSION_STRING_FULL                      MBEDTLS_VERSION_STRING_FULL
        #define POLARSSL_X509_CRL_H                               MBEDTLS_X509_CRL_H
        #define POLARSSL_X509_CRT_H                               MBEDTLS_X509_CRT_H
        #define POLARSSL_X509_CSR_H                               MBEDTLS_X509_CSR_H
        #define POLARSSL_X509_H                                   MBEDTLS_X509_H
        #define POLARSSL_XTEA_H                                   MBEDTLS_XTEA_H
        #define RSA_CRYPT                                         MBEDTLS_RSA_CRYPT
        #define RSA_PKCS_V15                                      MBEDTLS_RSA_PKCS_V15
        #define RSA_PKCS_V21                                      MBEDTLS_RSA_PKCS_V21
        #define RSA_PRIVATE                                       MBEDTLS_RSA_PRIVATE
        #define RSA_PUBLIC                                        MBEDTLS_RSA_PUBLIC
        #define RSA_SALT_LEN_ANY                                  MBEDTLS_RSA_SALT_LEN_ANY
        #define RSA_SIGN                                          MBEDTLS_RSA_SIGN
        #define SSL_ALERT_LEVEL_FATAL                             MBEDTLS_SSL_ALERT_LEVEL_FATAL
        #define SSL_ALERT_LEVEL_WARNING                           MBEDTLS_SSL_ALERT_LEVEL_WARNING
        #define SSL_ALERT_MSG_ACCESS_DENIED                       MBEDTLS_SSL_ALERT_MSG_ACCESS_DENIED
        #define SSL_ALERT_MSG_BAD_CERT                            MBEDTLS_SSL_ALERT_MSG_BAD_CERT
        #define SSL_ALERT_MSG_BAD_RECORD_MAC                      MBEDTLS_SSL_ALERT_MSG_BAD_RECORD_MAC
        #define SSL_ALERT_MSG_CERT_EXPIRED                        MBEDTLS_SSL_ALERT_MSG_CERT_EXPIRED
        #define SSL_ALERT_MSG_CERT_REVOKED                        MBEDTLS_SSL_ALERT_MSG_CERT_REVOKED
        #define SSL_ALERT_MSG_CERT_UNKNOWN                        MBEDTLS_SSL_ALERT_MSG_CERT_UNKNOWN
        #define SSL_ALERT_MSG_CLOSE_NOTIFY                        MBEDTLS_SSL_ALERT_MSG_CLOSE_NOTIFY
        #define SSL_ALERT_MSG_DECODE_ERROR                        MBEDTLS_SSL_ALERT_MSG_DECODE_ERROR
        #define SSL_ALERT_MSG_DECOMPRESSION_FAILURE               MBEDTLS_SSL_ALERT_MSG_DECOMPRESSION_FAILURE
        #define SSL_ALERT_MSG_DECRYPTION_FAILED                   MBEDTLS_SSL_ALERT_MSG_DECRYPTION_FAILED
        #define SSL_ALERT_MSG_DECRYPT_ERROR                       MBEDTLS_SSL_ALERT_MSG_DECRYPT_ERROR
        #define SSL_ALERT_MSG_EXPORT_RESTRICTION                  MBEDTLS_SSL_ALERT_MSG_EXPORT_RESTRICTION
        #define SSL_ALERT_MSG_HANDSHAKE_FAILURE                   MBEDTLS_SSL_ALERT_MSG_HANDSHAKE_FAILURE
        #define SSL_ALERT_MSG_ILLEGAL_PARAMETER                   MBEDTLS_SSL_ALERT_MSG_ILLEGAL_PARAMETER
        #define SSL_ALERT_MSG_INAPROPRIATE_FALLBACK               MBEDTLS_SSL_ALERT_MSG_INAPROPRIATE_FALLBACK
        #define SSL_ALERT_MSG_INSUFFICIENT_SECURITY               MBEDTLS_SSL_ALERT_MSG_INSUFFICIENT_SECURITY
        #define SSL_ALERT_MSG_INTERNAL_ERROR                      MBEDTLS_SSL_ALERT_MSG_INTERNAL_ERROR
        #define SSL_ALERT_MSG_NO_APPLICATION_PROTOCOL             MBEDTLS_SSL_ALERT_MSG_NO_APPLICATION_PROTOCOL
        #define SSL_ALERT_MSG_NO_CERT                             MBEDTLS_SSL_ALERT_MSG_NO_CERT
        #define SSL_ALERT_MSG_NO_RENEGOTIATION                    MBEDTLS_SSL_ALERT_MSG_NO_RENEGOTIATION
        #define SSL_ALERT_MSG_PROTOCOL_VERSION                    MBEDTLS_SSL_ALERT_MSG_PROTOCOL_VERSION
        #define SSL_ALERT_MSG_RECORD_OVERFLOW                     MBEDTLS_SSL_ALERT_MSG_RECORD_OVERFLOW
        #define SSL_ALERT_MSG_UNEXPECTED_MESSAGE                  MBEDTLS_SSL_ALERT_MSG_UNEXPECTED_MESSAGE
        #define SSL_ALERT_MSG_UNKNOWN_CA                          MBEDTLS_SSL_ALERT_MSG_UNKNOWN_CA
        #define SSL_ALERT_MSG_UNKNOWN_PSK_IDENTITY                MBEDTLS_SSL_ALERT_MSG_UNKNOWN_PSK_IDENTITY
        #define SSL_ALERT_MSG_UNRECOGNIZED_NAME                   MBEDTLS_SSL_ALERT_MSG_UNRECOGNIZED_NAME
        #define SSL_ALERT_MSG_UNSUPPORTED_CERT                    MBEDTLS_SSL_ALERT_MSG_UNSUPPORTED_CERT
        #define SSL_ALERT_MSG_UNSUPPORTED_EXT                     MBEDTLS_SSL_ALERT_MSG_UNSUPPORTED_EXT
        #define SSL_ALERT_MSG_USER_CANCELED                       MBEDTLS_SSL_ALERT_MSG_USER_CANCELED
        #define SSL_ANTI_REPLAY_DISABLED                          MBEDTLS_SSL_ANTI_REPLAY_DISABLED
        #define SSL_ANTI_REPLAY_ENABLED                           MBEDTLS_SSL_ANTI_REPLAY_ENABLED
        #define SSL_ARC4_DISABLED                                 MBEDTLS_SSL_ARC4_DISABLED
        #define SSL_ARC4_ENABLED                                  MBEDTLS_SSL_ARC4_ENABLED
        #define SSL_BUFFER_LEN \
                ( ( ( MBEDTLS_SSL_IN_BUFFER_LEN ) < ( MBEDTLS_SSL_OUT_BUFFER_LEN ) ) \
                         ? ( MBEDTLS_SSL_IN_BUFFER_LEN ) : ( MBEDTLS_SSL_OUT_BUFFER_LEN ) )
        #define SSL_CACHE_DEFAULT_MAX_ENTRIES                     MBEDTLS_SSL_CACHE_DEFAULT_MAX_ENTRIES
        #define SSL_CACHE_DEFAULT_TIMEOUT                         MBEDTLS_SSL_CACHE_DEFAULT_TIMEOUT
        #define SSL_CBC_RECORD_SPLITTING_DISABLED                 MBEDTLS_SSL_CBC_RECORD_SPLITTING_DISABLED
        #define SSL_CBC_RECORD_SPLITTING_ENABLED                  MBEDTLS_SSL_CBC_RECORD_SPLITTING_ENABLED
        #define SSL_CERTIFICATE_REQUEST                           MBEDTLS_SSL_CERTIFICATE_REQUEST
        #define SSL_CERTIFICATE_VERIFY                            MBEDTLS_SSL_CERTIFICATE_VERIFY
        #define SSL_CERT_TYPE_ECDSA_SIGN                          MBEDTLS_SSL_CERT_TYPE_ECDSA_SIGN
        #define SSL_CERT_TYPE_RSA_SIGN                            MBEDTLS_SSL_CERT_TYPE_RSA_SIGN
        #define SSL_CHANNEL_INBOUND                               MBEDTLS_SSL_CHANNEL_INBOUND
        #define SSL_CHANNEL_OUTBOUND                              MBEDTLS_SSL_CHANNEL_OUTBOUND
        #define SSL_CIPHERSUITES                                  MBEDTLS_SSL_CIPHERSUITES
        #define SSL_CLIENT_CERTIFICATE                            MBEDTLS_SSL_CLIENT_CERTIFICATE
        #define SSL_CLIENT_CHANGE_CIPHER_SPEC                     MBEDTLS_SSL_CLIENT_CHANGE_CIPHER_SPEC
        #define SSL_CLIENT_FINISHED                               MBEDTLS_SSL_CLIENT_FINISHED
        #define SSL_CLIENT_HELLO                                  MBEDTLS_SSL_CLIENT_HELLO
        #define SSL_CLIENT_KEY_EXCHANGE                           MBEDTLS_SSL_CLIENT_KEY_EXCHANGE
        #define SSL_COMPRESSION_ADD                               MBEDTLS_SSL_COMPRESSION_ADD
        #define SSL_COMPRESS_DEFLATE                              MBEDTLS_SSL_COMPRESS_DEFLATE
        #define SSL_COMPRESS_NULL                                 MBEDTLS_SSL_COMPRESS_NULL
        #define SSL_DEBUG_BUF                                     MBEDTLS_SSL_DEBUG_BUF
        #define SSL_DEBUG_CRT                                     MBEDTLS_SSL_DEBUG_CRT
        #define SSL_DEBUG_ECP                                     MBEDTLS_SSL_DEBUG_ECP
        #define SSL_DEBUG_MPI                                     MBEDTLS_SSL_DEBUG_MPI
        #define SSL_DEBUG_MSG                                     MBEDTLS_SSL_DEBUG_MSG
        #define SSL_DEBUG_RET                                     MBEDTLS_SSL_DEBUG_RET
        #define SSL_DEFAULT_TICKET_LIFETIME                       MBEDTLS_SSL_DEFAULT_TICKET_LIFETIME
        #define SSL_DTLS_TIMEOUT_DFL_MAX                          MBEDTLS_SSL_DTLS_TIMEOUT_DFL_MAX
        #define SSL_DTLS_TIMEOUT_DFL_MIN                          MBEDTLS_SSL_DTLS_TIMEOUT_DFL_MIN
        #define SSL_EMPTY_RENEGOTIATION_INFO                      MBEDTLS_SSL_EMPTY_RENEGOTIATION_INFO
        #define SSL_ETM_DISABLED                                  MBEDTLS_SSL_ETM_DISABLED
        #define SSL_ETM_ENABLED                                   MBEDTLS_SSL_ETM_ENABLED
        #define SSL_EXTENDED_MS_DISABLED                          MBEDTLS_SSL_EXTENDED_MS_DISABLED
        #define SSL_EXTENDED_MS_ENABLED                           MBEDTLS_SSL_EXTENDED_MS_ENABLED
        #define SSL_FALLBACK_SCSV                                 MBEDTLS_SSL_FALLBACK_SCSV
        #define SSL_FLUSH_BUFFERS                                 MBEDTLS_SSL_FLUSH_BUFFERS
        #define SSL_HANDSHAKE_OVER                                MBEDTLS_SSL_HANDSHAKE_OVER
        #define SSL_HANDSHAKE_WRAPUP                              MBEDTLS_SSL_HANDSHAKE_WRAPUP
        #define SSL_HASH_MD5                                      MBEDTLS_SSL_HASH_MD5
        #define SSL_HASH_NONE                                     MBEDTLS_SSL_HASH_NONE
        #define SSL_HASH_SHA1                                     MBEDTLS_SSL_HASH_SHA1
        #define SSL_HASH_SHA224                                   MBEDTLS_SSL_HASH_SHA224
        #define SSL_HASH_SHA256                                   MBEDTLS_SSL_HASH_SHA256
        #define SSL_HASH_SHA384                                   MBEDTLS_SSL_HASH_SHA384
        #define SSL_HASH_SHA512                                   MBEDTLS_SSL_HASH_SHA512
        #define SSL_HELLO_REQUEST                                 MBEDTLS_SSL_HELLO_REQUEST
        #define SSL_HS_CERTIFICATE                                MBEDTLS_SSL_HS_CERTIFICATE
        #define SSL_HS_CERTIFICATE_REQUEST                        MBEDTLS_SSL_HS_CERTIFICATE_REQUEST
        #define SSL_HS_CERTIFICATE_VERIFY                         MBEDTLS_SSL_HS_CERTIFICATE_VERIFY
        #define SSL_HS_CLIENT_HELLO                               MBEDTLS_SSL_HS_CLIENT_HELLO
        #define SSL_HS_CLIENT_KEY_EXCHANGE                        MBEDTLS_SSL_HS_CLIENT_KEY_EXCHANGE
        #define SSL_HS_FINISHED                                   MBEDTLS_SSL_HS_FINISHED
        #define SSL_HS_HELLO_REQUEST                              MBEDTLS_SSL_HS_HELLO_REQUEST
        #define SSL_HS_HELLO_VERIFY_REQUEST                       MBEDTLS_SSL_HS_HELLO_VERIFY_REQUEST
        #define SSL_HS_NEW_SESSION_TICKET                         MBEDTLS_SSL_HS_NEW_SESSION_TICKET
        #define SSL_HS_SERVER_HELLO                               MBEDTLS_SSL_HS_SERVER_HELLO
        #define SSL_HS_SERVER_HELLO_DONE                          MBEDTLS_SSL_HS_SERVER_HELLO_DONE
        #define SSL_HS_SERVER_KEY_EXCHANGE                        MBEDTLS_SSL_HS_SERVER_KEY_EXCHANGE
        #define SSL_INITIAL_HANDSHAKE                             MBEDTLS_SSL_INITIAL_HANDSHAKE
        #define SSL_IS_CLIENT                                     MBEDTLS_SSL_IS_CLIENT
        #define SSL_IS_FALLBACK                                   MBEDTLS_SSL_IS_FALLBACK
        #define SSL_IS_NOT_FALLBACK                               MBEDTLS_SSL_IS_NOT_FALLBACK
        #define SSL_IS_SERVER                                     MBEDTLS_SSL_IS_SERVER
        #define SSL_LEGACY_ALLOW_RENEGOTIATION                    MBEDTLS_SSL_LEGACY_ALLOW_RENEGOTIATION
        #define SSL_LEGACY_BREAK_HANDSHAKE                        MBEDTLS_SSL_LEGACY_BREAK_HANDSHAKE
        #define SSL_LEGACY_NO_RENEGOTIATION                       MBEDTLS_SSL_LEGACY_NO_RENEGOTIATION
        #define SSL_LEGACY_RENEGOTIATION                          MBEDTLS_SSL_LEGACY_RENEGOTIATION
        #define SSL_MAC_ADD                                       MBEDTLS_SSL_MAC_ADD
        #define SSL_MAJOR_VERSION_3                               MBEDTLS_SSL_MAJOR_VERSION_3
        #define SSL_MAX_CONTENT_LEN                               MBEDTLS_SSL_MAX_CONTENT_LEN
        #define SSL_MAX_FRAG_LEN_1024                             MBEDTLS_SSL_MAX_FRAG_LEN_1024
        #define SSL_MAX_FRAG_LEN_2048                             MBEDTLS_SSL_MAX_FRAG_LEN_2048
        #define SSL_MAX_FRAG_LEN_4096                             MBEDTLS_SSL_MAX_FRAG_LEN_4096
        #define SSL_MAX_FRAG_LEN_512                              MBEDTLS_SSL_MAX_FRAG_LEN_512
        #define SSL_MAX_FRAG_LEN_INVALID                          MBEDTLS_SSL_MAX_FRAG_LEN_INVALID
        #define SSL_MAX_FRAG_LEN_NONE                             MBEDTLS_SSL_MAX_FRAG_LEN_NONE
        #define SSL_MAX_MAJOR_VERSION                             MBEDTLS_SSL_MAX_MAJOR_VERSION
        #define SSL_MAX_MINOR_VERSION                             MBEDTLS_SSL_MAX_MINOR_VERSION
        #define SSL_MINOR_VERSION_0                               MBEDTLS_SSL_MINOR_VERSION_0
        #define SSL_MINOR_VERSION_1                               MBEDTLS_SSL_MINOR_VERSION_1
        #define SSL_MINOR_VERSION_2                               MBEDTLS_SSL_MINOR_VERSION_2
        #define SSL_MINOR_VERSION_3                               MBEDTLS_SSL_MINOR_VERSION_3
        #define SSL_MIN_MAJOR_VERSION                             MBEDTLS_SSL_MIN_MAJOR_VERSION
        #define SSL_MIN_MINOR_VERSION                             MBEDTLS_SSL_MIN_MINOR_VERSION
        #define SSL_MSG_ALERT                                     MBEDTLS_SSL_MSG_ALERT
        #define SSL_MSG_APPLICATION_DATA                          MBEDTLS_SSL_MSG_APPLICATION_DATA
        #define SSL_MSG_CHANGE_CIPHER_SPEC                        MBEDTLS_SSL_MSG_CHANGE_CIPHER_SPEC
        #define SSL_MSG_HANDSHAKE                                 MBEDTLS_SSL_MSG_HANDSHAKE
        #define SSL_PADDING_ADD                                   MBEDTLS_SSL_PADDING_ADD
        #define SSL_RENEGOTIATION                                 MBEDTLS_SSL_RENEGOTIATION
        #define SSL_RENEGOTIATION_DISABLED                        MBEDTLS_SSL_RENEGOTIATION_DISABLED
        #define SSL_RENEGOTIATION_DONE                            MBEDTLS_SSL_RENEGOTIATION_DONE
        #define SSL_RENEGOTIATION_ENABLED                         MBEDTLS_SSL_RENEGOTIATION_ENABLED
        #define SSL_RENEGOTIATION_NOT_ENFORCED                    MBEDTLS_SSL_RENEGOTIATION_NOT_ENFORCED
        #define SSL_RENEGOTIATION_PENDING                         MBEDTLS_SSL_RENEGOTIATION_PENDING
        #define SSL_RENEGO_MAX_RECORDS_DEFAULT                    MBEDTLS_SSL_RENEGO_MAX_RECORDS_DEFAULT
        #define SSL_RETRANS_FINISHED                              MBEDTLS_SSL_RETRANS_FINISHED
        #define SSL_RETRANS_PREPARING                             MBEDTLS_SSL_RETRANS_PREPARING
        #define SSL_RETRANS_SENDING                               MBEDTLS_SSL_RETRANS_SENDING
        #define SSL_RETRANS_WAITING                               MBEDTLS_SSL_RETRANS_WAITING
        #define SSL_SECURE_RENEGOTIATION                          MBEDTLS_SSL_SECURE_RENEGOTIATION
        #define SSL_SERVER_CERTIFICATE                            MBEDTLS_SSL_SERVER_CERTIFICATE
        #define SSL_SERVER_CHANGE_CIPHER_SPEC                     MBEDTLS_SSL_SERVER_CHANGE_CIPHER_SPEC
        #define SSL_SERVER_FINISHED                               MBEDTLS_SSL_SERVER_FINISHED
        #define SSL_SERVER_HELLO                                  MBEDTLS_SSL_SERVER_HELLO
        #define SSL_SERVER_HELLO_DONE                             MBEDTLS_SSL_SERVER_HELLO_DONE
        #define SSL_SERVER_HELLO_VERIFY_REQUEST_SENT              MBEDTLS_SSL_SERVER_HELLO_VERIFY_REQUEST_SENT
        #define SSL_SERVER_KEY_EXCHANGE                           MBEDTLS_SSL_SERVER_KEY_EXCHANGE
        #define SSL_SERVER_NEW_SESSION_TICKET                     MBEDTLS_SSL_SERVER_NEW_SESSION_TICKET
        #define SSL_SESSION_TICKETS_DISABLED                      MBEDTLS_SSL_SESSION_TICKETS_DISABLED
        #define SSL_SESSION_TICKETS_ENABLED                       MBEDTLS_SSL_SESSION_TICKETS_ENABLED
        #define SSL_SIG_ANON                                      MBEDTLS_SSL_SIG_ANON
        #define SSL_SIG_ECDSA                                     MBEDTLS_SSL_SIG_ECDSA
        #define SSL_SIG_RSA                                       MBEDTLS_SSL_SIG_RSA
        #define SSL_TRANSPORT_DATAGRAM                            MBEDTLS_SSL_TRANSPORT_DATAGRAM
        #define SSL_TRANSPORT_STREAM                              MBEDTLS_SSL_TRANSPORT_STREAM
        #define SSL_TRUNCATED_HMAC_LEN                            MBEDTLS_SSL_TRUNCATED_HMAC_LEN
        #define SSL_TRUNC_HMAC_DISABLED                           MBEDTLS_SSL_TRUNC_HMAC_DISABLED
        #define SSL_TRUNC_HMAC_ENABLED                            MBEDTLS_SSL_TRUNC_HMAC_ENABLED
        #define SSL_VERIFY_DATA_MAX_LEN                           MBEDTLS_SSL_VERIFY_DATA_MAX_LEN
        #define SSL_VERIFY_NONE                                   MBEDTLS_SSL_VERIFY_NONE
        #define SSL_VERIFY_OPTIONAL                               MBEDTLS_SSL_VERIFY_OPTIONAL
        #define SSL_VERIFY_REQUIRED                               MBEDTLS_SSL_VERIFY_REQUIRED
        #define TLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA                 MBEDTLS_TLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA
        #define TLS_DHE_PSK_WITH_AES_128_CBC_SHA                  MBEDTLS_TLS_DHE_PSK_WITH_AES_128_CBC_SHA
        #define TLS_DHE_PSK_WITH_AES_128_CBC_SHA256               MBEDTLS_TLS_DHE_PSK_WITH_AES_128_CBC_SHA256
        #define TLS_DHE_PSK_WITH_AES_128_CCM                      MBEDTLS_TLS_DHE_PSK_WITH_AES_128_CCM
        #define TLS_DHE_PSK_WITH_AES_128_CCM_8                    MBEDTLS_TLS_DHE_PSK_WITH_AES_128_CCM_8
        #define TLS_DHE_PSK_WITH_AES_128_GCM_SHA256               MBEDTLS_TLS_DHE_PSK_WITH_AES_128_GCM_SHA256
        #define TLS_DHE_PSK_WITH_AES_256_CBC_SHA                  MBEDTLS_TLS_DHE_PSK_WITH_AES_256_CBC_SHA
        #define TLS_DHE_PSK_WITH_AES_256_CBC_SHA384               MBEDTLS_TLS_DHE_PSK_WITH_AES_256_CBC_SHA384
        #define TLS_DHE_PSK_WITH_AES_256_CCM                      MBEDTLS_TLS_DHE_PSK_WITH_AES_256_CCM
        #define TLS_DHE_PSK_WITH_AES_256_CCM_8                    MBEDTLS_TLS_DHE_PSK_WITH_AES_256_CCM_8
        #define TLS_DHE_PSK_WITH_AES_256_GCM_SHA384               MBEDTLS_TLS_DHE_PSK_WITH_AES_256_GCM_SHA384
        #define TLS_DHE_PSK_WITH_CAMELLIA_128_CBC_SHA256          MBEDTLS_TLS_DHE_PSK_WITH_CAMELLIA_128_CBC_SHA256
        #define TLS_DHE_PSK_WITH_CAMELLIA_128_GCM_SHA256          MBEDTLS_TLS_DHE_PSK_WITH_CAMELLIA_128_GCM_SHA256
        #define TLS_DHE_PSK_WITH_CAMELLIA_256_CBC_SHA384          MBEDTLS_TLS_DHE_PSK_WITH_CAMELLIA_256_CBC_SHA384
        #define TLS_DHE_PSK_WITH_CAMELLIA_256_GCM_SHA384          MBEDTLS_TLS_DHE_PSK_WITH_CAMELLIA_256_GCM_SHA384
        #define TLS_DHE_PSK_WITH_NULL_SHA                         MBEDTLS_TLS_DHE_PSK_WITH_NULL_SHA
        #define TLS_DHE_PSK_WITH_NULL_SHA256                      MBEDTLS_TLS_DHE_PSK_WITH_NULL_SHA256
        #define TLS_DHE_PSK_WITH_NULL_SHA384                      MBEDTLS_TLS_DHE_PSK_WITH_NULL_SHA384
        #define TLS_DHE_PSK_WITH_RC4_128_SHA                      MBEDTLS_TLS_DHE_PSK_WITH_RC4_128_SHA
        #define TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA                 MBEDTLS_TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA
        #define TLS_DHE_RSA_WITH_AES_128_CBC_SHA                  MBEDTLS_TLS_DHE_RSA_WITH_AES_128_CBC_SHA
        #define TLS_DHE_RSA_WITH_AES_128_CBC_SHA256               MBEDTLS_TLS_DHE_RSA_WITH_AES_128_CBC_SHA256
        #define TLS_DHE_RSA_WITH_AES_128_CCM                      MBEDTLS_TLS_DHE_RSA_WITH_AES_128_CCM
        #define TLS_DHE_RSA_WITH_AES_128_CCM_8                    MBEDTLS_TLS_DHE_RSA_WITH_AES_128_CCM_8
        #define TLS_DHE_RSA_WITH_AES_128_GCM_SHA256               MBEDTLS_TLS_DHE_RSA_WITH_AES_128_GCM_SHA256
        #define TLS_DHE_RSA_WITH_AES_256_CBC_SHA                  MBEDTLS_TLS_DHE_RSA_WITH_AES_256_CBC_SHA
        #define TLS_DHE_RSA_WITH_AES_256_CBC_SHA256               MBEDTLS_TLS_DHE_RSA_WITH_AES_256_CBC_SHA256
        #define TLS_DHE_RSA_WITH_AES_256_CCM                      MBEDTLS_TLS_DHE_RSA_WITH_AES_256_CCM
        #define TLS_DHE_RSA_WITH_AES_256_CCM_8                    MBEDTLS_TLS_DHE_RSA_WITH_AES_256_CCM_8
        #define TLS_DHE_RSA_WITH_AES_256_GCM_SHA384               MBEDTLS_TLS_DHE_RSA_WITH_AES_256_GCM_SHA384
        #define TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA             MBEDTLS_TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA
        #define TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA256          MBEDTLS_TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA256
        #define TLS_DHE_RSA_WITH_CAMELLIA_128_GCM_SHA256          MBEDTLS_TLS_DHE_RSA_WITH_CAMELLIA_128_GCM_SHA256
        #define TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA             MBEDTLS_TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA
        #define TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA256          MBEDTLS_TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA256
        #define TLS_DHE_RSA_WITH_CAMELLIA_256_GCM_SHA384          MBEDTLS_TLS_DHE_RSA_WITH_CAMELLIA_256_GCM_SHA384
        #define TLS_DHE_RSA_WITH_DES_CBC_SHA                      MBEDTLS_TLS_DHE_RSA_WITH_DES_CBC_SHA
        #define TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA             MBEDTLS_TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA
        #define TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA              MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA
        #define TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256           MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256
        #define TLS_ECDHE_ECDSA_WITH_AES_128_CCM                  MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CCM
        #define TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8                MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8
        #define TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256           MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256
        #define TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA              MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA
        #define TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384           MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384
        #define TLS_ECDHE_ECDSA_WITH_AES_256_CCM                  MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_CCM
        #define TLS_ECDHE_ECDSA_WITH_AES_256_CCM_8                MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_CCM_8
        #define TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384           MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384
        #define TLS_ECDHE_ECDSA_WITH_CAMELLIA_128_CBC_SHA256      MBEDTLS_TLS_ECDHE_ECDSA_WITH_CAMELLIA_128_CBC_SHA256
        #define TLS_ECDHE_ECDSA_WITH_CAMELLIA_128_GCM_SHA256      MBEDTLS_TLS_ECDHE_ECDSA_WITH_CAMELLIA_128_GCM_SHA256
        #define TLS_ECDHE_ECDSA_WITH_CAMELLIA_256_CBC_SHA384      MBEDTLS_TLS_ECDHE_ECDSA_WITH_CAMELLIA_256_CBC_SHA384
        #define TLS_ECDHE_ECDSA_WITH_CAMELLIA_256_GCM_SHA384      MBEDTLS_TLS_ECDHE_ECDSA_WITH_CAMELLIA_256_GCM_SHA384
        #define TLS_ECDHE_ECDSA_WITH_NULL_SHA                     MBEDTLS_TLS_ECDHE_ECDSA_WITH_NULL_SHA
        #define TLS_ECDHE_ECDSA_WITH_RC4_128_SHA                  MBEDTLS_TLS_ECDHE_ECDSA_WITH_RC4_128_SHA
        #define TLS_ECDHE_PSK_WITH_3DES_EDE_CBC_SHA               MBEDTLS_TLS_ECDHE_PSK_WITH_3DES_EDE_CBC_SHA
        #define TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA                MBEDTLS_TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA
        #define TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256             MBEDTLS_TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256
        #define TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA                MBEDTLS_TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA
        #define TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA384             MBEDTLS_TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA384
        #define TLS_ECDHE_PSK_WITH_CAMELLIA_128_CBC_SHA256        MBEDTLS_TLS_ECDHE_PSK_WITH_CAMELLIA_128_CBC_SHA256
        #define TLS_ECDHE_PSK_WITH_CAMELLIA_256_CBC_SHA384        MBEDTLS_TLS_ECDHE_PSK_WITH_CAMELLIA_256_CBC_SHA384
        #define TLS_ECDHE_PSK_WITH_NULL_SHA                       MBEDTLS_TLS_ECDHE_PSK_WITH_NULL_SHA
        #define TLS_ECDHE_PSK_WITH_NULL_SHA256                    MBEDTLS_TLS_ECDHE_PSK_WITH_NULL_SHA256
        #define TLS_ECDHE_PSK_WITH_NULL_SHA384                    MBEDTLS_TLS_ECDHE_PSK_WITH_NULL_SHA384
        #define TLS_ECDHE_PSK_WITH_RC4_128_SHA                    MBEDTLS_TLS_ECDHE_PSK_WITH_RC4_128_SHA
        #define TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA               MBEDTLS_TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA
        #define TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA                MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA
        #define TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256             MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256
        #define TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256             MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256
        #define TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA                MBEDTLS_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA
        #define TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384             MBEDTLS_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384
        #define TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384             MBEDTLS_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384
        #define TLS_ECDHE_RSA_WITH_CAMELLIA_128_CBC_SHA256        MBEDTLS_TLS_ECDHE_RSA_WITH_CAMELLIA_128_CBC_SHA256
        #define TLS_ECDHE_RSA_WITH_CAMELLIA_128_GCM_SHA256        MBEDTLS_TLS_ECDHE_RSA_WITH_CAMELLIA_128_GCM_SHA256
        #define TLS_ECDHE_RSA_WITH_CAMELLIA_256_CBC_SHA384        MBEDTLS_TLS_ECDHE_RSA_WITH_CAMELLIA_256_CBC_SHA384
        #define TLS_ECDHE_RSA_WITH_CAMELLIA_256_GCM_SHA384        MBEDTLS_TLS_ECDHE_RSA_WITH_CAMELLIA_256_GCM_SHA384
        #define TLS_ECDHE_RSA_WITH_NULL_SHA                       MBEDTLS_TLS_ECDHE_RSA_WITH_NULL_SHA
        #define TLS_ECDHE_RSA_WITH_RC4_128_SHA                    MBEDTLS_TLS_ECDHE_RSA_WITH_RC4_128_SHA
        #define TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA              MBEDTLS_TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA
        #define TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA               MBEDTLS_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA
        #define TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256            MBEDTLS_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256
        #define TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256            MBEDTLS_TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256
        #define TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA               MBEDTLS_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA
        #define TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384            MBEDTLS_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384
        #define TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384            MBEDTLS_TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384
        #define TLS_ECDH_ECDSA_WITH_CAMELLIA_128_CBC_SHA256       MBEDTLS_TLS_ECDH_ECDSA_WITH_CAMELLIA_128_CBC_SHA256
        #define TLS_ECDH_ECDSA_WITH_CAMELLIA_128_GCM_SHA256       MBEDTLS_TLS_ECDH_ECDSA_WITH_CAMELLIA_128_GCM_SHA256
        #define TLS_ECDH_ECDSA_WITH_CAMELLIA_256_CBC_SHA384       MBEDTLS_TLS_ECDH_ECDSA_WITH_CAMELLIA_256_CBC_SHA384
        #define TLS_ECDH_ECDSA_WITH_CAMELLIA_256_GCM_SHA384       MBEDTLS_TLS_ECDH_ECDSA_WITH_CAMELLIA_256_GCM_SHA384
        #define TLS_ECDH_ECDSA_WITH_NULL_SHA                      MBEDTLS_TLS_ECDH_ECDSA_WITH_NULL_SHA
        #define TLS_ECDH_ECDSA_WITH_RC4_128_SHA                   MBEDTLS_TLS_ECDH_ECDSA_WITH_RC4_128_SHA
        #define TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA                MBEDTLS_TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA
        #define TLS_ECDH_RSA_WITH_AES_128_CBC_SHA                 MBEDTLS_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA
        #define TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256              MBEDTLS_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256
        #define TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256              MBEDTLS_TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256
        #define TLS_ECDH_RSA_WITH_AES_256_CBC_SHA                 MBEDTLS_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA
        #define TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384              MBEDTLS_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384
        #define TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384              MBEDTLS_TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384
        #define TLS_ECDH_RSA_WITH_CAMELLIA_128_CBC_SHA256         MBEDTLS_TLS_ECDH_RSA_WITH_CAMELLIA_128_CBC_SHA256
        #define TLS_ECDH_RSA_WITH_CAMELLIA_128_GCM_SHA256         MBEDTLS_TLS_ECDH_RSA_WITH_CAMELLIA_128_GCM_SHA256
        #define TLS_ECDH_RSA_WITH_CAMELLIA_256_CBC_SHA384         MBEDTLS_TLS_ECDH_RSA_WITH_CAMELLIA_256_CBC_SHA384
        #define TLS_ECDH_RSA_WITH_CAMELLIA_256_GCM_SHA384         MBEDTLS_TLS_ECDH_RSA_WITH_CAMELLIA_256_GCM_SHA384
        #define TLS_ECDH_RSA_WITH_NULL_SHA                        MBEDTLS_TLS_ECDH_RSA_WITH_NULL_SHA
        #define TLS_ECDH_RSA_WITH_RC4_128_SHA                     MBEDTLS_TLS_ECDH_RSA_WITH_RC4_128_SHA
        #define TLS_EXT_ALPN                                      MBEDTLS_TLS_EXT_ALPN
        #define TLS_EXT_ENCRYPT_THEN_MAC                          MBEDTLS_TLS_EXT_ENCRYPT_THEN_MAC
        #define TLS_EXT_EXTENDED_MASTER_SECRET                    MBEDTLS_TLS_EXT_EXTENDED_MASTER_SECRET
        #define TLS_EXT_MAX_FRAGMENT_LENGTH                       MBEDTLS_TLS_EXT_MAX_FRAGMENT_LENGTH
        #define TLS_EXT_RENEGOTIATION_INFO                        MBEDTLS_TLS_EXT_RENEGOTIATION_INFO
        #define TLS_EXT_SERVERNAME                                MBEDTLS_TLS_EXT_SERVERNAME
        #define TLS_EXT_SERVERNAME_HOSTNAME                       MBEDTLS_TLS_EXT_SERVERNAME_HOSTNAME
        #define TLS_EXT_SESSION_TICKET                            MBEDTLS_TLS_EXT_SESSION_TICKET
        #define TLS_EXT_SIG_ALG                                   MBEDTLS_TLS_EXT_SIG_ALG
        #define TLS_EXT_SUPPORTED_ELLIPTIC_CURVES                 MBEDTLS_TLS_EXT_SUPPORTED_ELLIPTIC_CURVES
        #define TLS_EXT_SUPPORTED_POINT_FORMATS                   MBEDTLS_TLS_EXT_SUPPORTED_POINT_FORMATS
        #define TLS_EXT_SUPPORTED_POINT_FORMATS_PRESENT           MBEDTLS_TLS_EXT_SUPPORTED_POINT_FORMATS_PRESENT
        #define TLS_EXT_TRUNCATED_HMAC                            MBEDTLS_TLS_EXT_TRUNCATED_HMAC
        #define TLS_PSK_WITH_3DES_EDE_CBC_SHA                     MBEDTLS_TLS_PSK_WITH_3DES_EDE_CBC_SHA
        #define TLS_PSK_WITH_AES_128_CBC_SHA                      MBEDTLS_TLS_PSK_WITH_AES_128_CBC_SHA
        #define TLS_PSK_WITH_AES_128_CBC_SHA256                   MBEDTLS_TLS_PSK_WITH_AES_128_CBC_SHA256
        #define TLS_PSK_WITH_AES_128_CCM                          MBEDTLS_TLS_PSK_WITH_AES_128_CCM
        #define TLS_PSK_WITH_AES_128_CCM_8                        MBEDTLS_TLS_PSK_WITH_AES_128_CCM_8
        #define TLS_PSK_WITH_AES_128_GCM_SHA256                   MBEDTLS_TLS_PSK_WITH_AES_128_GCM_SHA256
        #define TLS_PSK_WITH_AES_256_CBC_SHA                      MBEDTLS_TLS_PSK_WITH_AES_256_CBC_SHA
        #define TLS_PSK_WITH_AES_256_CBC_SHA384                   MBEDTLS_TLS_PSK_WITH_AES_256_CBC_SHA384
        #define TLS_PSK_WITH_AES_256_CCM                          MBEDTLS_TLS_PSK_WITH_AES_256_CCM
        #define TLS_PSK_WITH_AES_256_CCM_8                        MBEDTLS_TLS_PSK_WITH_AES_256_CCM_8
        #define TLS_PSK_WITH_AES_256_GCM_SHA384                   MBEDTLS_TLS_PSK_WITH_AES_256_GCM_SHA384
        #define TLS_PSK_WITH_CAMELLIA_128_CBC_SHA256              MBEDTLS_TLS_PSK_WITH_CAMELLIA_128_CBC_SHA256
        #define TLS_PSK_WITH_CAMELLIA_128_GCM_SHA256              MBEDTLS_TLS_PSK_WITH_CAMELLIA_128_GCM_SHA256
        #define TLS_PSK_WITH_CAMELLIA_256_CBC_SHA384              MBEDTLS_TLS_PSK_WITH_CAMELLIA_256_CBC_SHA384
        #define TLS_PSK_WITH_CAMELLIA_256_GCM_SHA384              MBEDTLS_TLS_PSK_WITH_CAMELLIA_256_GCM_SHA384
        #define TLS_PSK_WITH_NULL_SHA                             MBEDTLS_TLS_PSK_WITH_NULL_SHA
        #define TLS_PSK_WITH_NULL_SHA256                          MBEDTLS_TLS_PSK_WITH_NULL_SHA256
        #define TLS_PSK_WITH_NULL_SHA384                          MBEDTLS_TLS_PSK_WITH_NULL_SHA384
        #define TLS_PSK_WITH_RC4_128_SHA                          MBEDTLS_TLS_PSK_WITH_RC4_128_SHA
        #define TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA                 MBEDTLS_TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA
        #define TLS_RSA_PSK_WITH_AES_128_CBC_SHA                  MBEDTLS_TLS_RSA_PSK_WITH_AES_128_CBC_SHA
        #define TLS_RSA_PSK_WITH_AES_128_CBC_SHA256               MBEDTLS_TLS_RSA_PSK_WITH_AES_128_CBC_SHA256
        #define TLS_RSA_PSK_WITH_AES_128_GCM_SHA256               MBEDTLS_TLS_RSA_PSK_WITH_AES_128_GCM_SHA256
        #define TLS_RSA_PSK_WITH_AES_256_CBC_SHA                  MBEDTLS_TLS_RSA_PSK_WITH_AES_256_CBC_SHA
        #define TLS_RSA_PSK_WITH_AES_256_CBC_SHA384               MBEDTLS_TLS_RSA_PSK_WITH_AES_256_CBC_SHA384
        #define TLS_RSA_PSK_WITH_AES_256_GCM_SHA384               MBEDTLS_TLS_RSA_PSK_WITH_AES_256_GCM_SHA384
        #define TLS_RSA_PSK_WITH_CAMELLIA_128_CBC_SHA256          MBEDTLS_TLS_RSA_PSK_WITH_CAMELLIA_128_CBC_SHA256
        #define TLS_RSA_PSK_WITH_CAMELLIA_128_GCM_SHA256          MBEDTLS_TLS_RSA_PSK_WITH_CAMELLIA_128_GCM_SHA256
        #define TLS_RSA_PSK_WITH_CAMELLIA_256_CBC_SHA384          MBEDTLS_TLS_RSA_PSK_WITH_CAMELLIA_256_CBC_SHA384
        #define TLS_RSA_PSK_WITH_CAMELLIA_256_GCM_SHA384          MBEDTLS_TLS_RSA_PSK_WITH_CAMELLIA_256_GCM_SHA384
        #define TLS_RSA_PSK_WITH_NULL_SHA                         MBEDTLS_TLS_RSA_PSK_WITH_NULL_SHA
        #define TLS_RSA_PSK_WITH_NULL_SHA256                      MBEDTLS_TLS_RSA_PSK_WITH_NULL_SHA256
        #define TLS_RSA_PSK_WITH_NULL_SHA384                      MBEDTLS_TLS_RSA_PSK_WITH_NULL_SHA384
        #define TLS_RSA_PSK_WITH_RC4_128_SHA                      MBEDTLS_TLS_RSA_PSK_WITH_RC4_128_SHA
        #define TLS_RSA_WITH_3DES_EDE_CBC_SHA                     MBEDTLS_TLS_RSA_WITH_3DES_EDE_CBC_SHA
        #define TLS_RSA_WITH_AES_128_CBC_SHA                      MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA
        #define TLS_RSA_WITH_AES_128_CBC_SHA256                   MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA256
        #define TLS_RSA_WITH_AES_128_CCM                          MBEDTLS_TLS_RSA_WITH_AES_128_CCM
        #define TLS_RSA_WITH_AES_128_CCM_8                        MBEDTLS_TLS_RSA_WITH_AES_128_CCM_8
        #define TLS_RSA_WITH_AES_128_GCM_SHA256                   MBEDTLS_TLS_RSA_WITH_AES_128_GCM_SHA256
        #define TLS_RSA_WITH_AES_256_CBC_SHA                      MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA
        #define TLS_RSA_WITH_AES_256_CBC_SHA256                   MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA256
        #define TLS_RSA_WITH_AES_256_CCM                          MBEDTLS_TLS_RSA_WITH_AES_256_CCM
        #define TLS_RSA_WITH_AES_256_CCM_8                        MBEDTLS_TLS_RSA_WITH_AES_256_CCM_8
        #define TLS_RSA_WITH_AES_256_GCM_SHA384                   MBEDTLS_TLS_RSA_WITH_AES_256_GCM_SHA384
        #define TLS_RSA_WITH_CAMELLIA_128_CBC_SHA                 MBEDTLS_TLS_RSA_WITH_CAMELLIA_128_CBC_SHA
        #define TLS_RSA_WITH_CAMELLIA_128_CBC_SHA256              MBEDTLS_TLS_RSA_WITH_CAMELLIA_128_CBC_SHA256
        #define TLS_RSA_WITH_CAMELLIA_128_GCM_SHA256              MBEDTLS_TLS_RSA_WITH_CAMELLIA_128_GCM_SHA256
        #define TLS_RSA_WITH_CAMELLIA_256_CBC_SHA                 MBEDTLS_TLS_RSA_WITH_CAMELLIA_256_CBC_SHA
        #define TLS_RSA_WITH_CAMELLIA_256_CBC_SHA256              MBEDTLS_TLS_RSA_WITH_CAMELLIA_256_CBC_SHA256
        #define TLS_RSA_WITH_CAMELLIA_256_GCM_SHA384              MBEDTLS_TLS_RSA_WITH_CAMELLIA_256_GCM_SHA384
        #define TLS_RSA_WITH_DES_CBC_SHA                          MBEDTLS_TLS_RSA_WITH_DES_CBC_SHA
        #define TLS_RSA_WITH_NULL_MD5                             MBEDTLS_TLS_RSA_WITH_NULL_MD5
        #define TLS_RSA_WITH_NULL_SHA                             MBEDTLS_TLS_RSA_WITH_NULL_SHA
        #define TLS_RSA_WITH_NULL_SHA256                          MBEDTLS_TLS_RSA_WITH_NULL_SHA256
        #define TLS_RSA_WITH_RC4_128_MD5                          MBEDTLS_TLS_RSA_WITH_RC4_128_MD5
        #define TLS_RSA_WITH_RC4_128_SHA                          MBEDTLS_TLS_RSA_WITH_RC4_128_SHA
        #define X509_CRT_VERSION_1                                MBEDTLS_X509_CRT_VERSION_1
        #define X509_CRT_VERSION_2                                MBEDTLS_X509_CRT_VERSION_2
        #define X509_CRT_VERSION_3                                MBEDTLS_X509_CRT_VERSION_3
        #define X509_FORMAT_DER                                   MBEDTLS_X509_FORMAT_DER
        #define X509_FORMAT_PEM                                   MBEDTLS_X509_FORMAT_PEM
        #define X509_MAX_DN_NAME_SIZE                             MBEDTLS_X509_MAX_DN_NAME_SIZE
        #define X509_RFC5280_MAX_SERIAL_LEN                       MBEDTLS_X509_RFC5280_MAX_SERIAL_LEN
        #define X509_RFC5280_UTC_TIME_LEN                         MBEDTLS_X509_RFC5280_UTC_TIME_LEN
        #define XTEA_DECRYPT                                      MBEDTLS_XTEA_DECRYPT
        #define XTEA_ENCRYPT                                      MBEDTLS_XTEA_ENCRYPT
        #define _asn1_bitstring                                   mbedtls_asn1_bitstring
        #define _asn1_buf                                         mbedtls_asn1_buf
        #define _asn1_named_data                                  mbedtls_asn1_named_data
        #define _asn1_sequence                                    mbedtls_asn1_sequence
        #define _ssl_cache_context                                mbedtls_ssl_cache_context
        #define _ssl_cache_entry                                  mbedtls_ssl_cache_entry
        #define _ssl_ciphersuite_t                                mbedtls_ssl_ciphersuite_t
        #define _ssl_context                                      mbedtls_ssl_context
        #define _ssl_flight_item                                  mbedtls_ssl_flight_item
        #define _ssl_handshake_params                             mbedtls_ssl_handshake_params
        #define _ssl_key_cert                                     mbedtls_ssl_key_cert
        #define _ssl_premaster_secret                             mbedtls_ssl_premaster_secret
        #define _ssl_session                                      mbedtls_ssl_session
        #define _ssl_transform                                    mbedtls_ssl_transform
        #define _x509_crl                                         mbedtls_x509_crl
        #define _x509_crl_entry                                   mbedtls_x509_crl_entry
        #define _x509_crt                                         mbedtls_x509_crt
        #define _x509_csr                                         mbedtls_x509_csr
        #define _x509_time                                        mbedtls_x509_time
        #define _x509write_cert                                   mbedtls_x509write_cert
        #define _x509write_csr                                    mbedtls_x509write_csr
        #define aes_context                                       mbedtls_aes_context
        #define aes_crypt_cbc                                     mbedtls_aes_crypt_cbc
        #define aes_crypt_cfb128                                  mbedtls_aes_crypt_cfb128
        #define aes_crypt_cfb8                                    mbedtls_aes_crypt_cfb8
        #define aes_crypt_ctr                                     mbedtls_aes_crypt_ctr
        #define aes_crypt_ecb                                     mbedtls_aes_crypt_ecb
        #define aes_free                                          mbedtls_aes_free
        #define aes_init                                          mbedtls_aes_init
        #define aes_self_test                                     mbedtls_aes_self_test
        #define aes_setkey_dec                                    mbedtls_aes_setkey_dec
        #define aes_setkey_enc                                    mbedtls_aes_setkey_enc
        #define aesni_crypt_ecb                                   mbedtls_aesni_crypt_ecb
        #define aesni_gcm_mult                                    mbedtls_aesni_gcm_mult
        #define aesni_inverse_key                                 mbedtls_aesni_inverse_key
        #define aesni_setkey_enc                                  mbedtls_aesni_setkey_enc
        #define aesni_supports                                    mbedtls_aesni_has_support
        #define alarmed                                           mbedtls_timing_alarmed
        #define arc4_context                                      mbedtls_arc4_context
        #define arc4_crypt                                        mbedtls_arc4_crypt
        #define arc4_free                                         mbedtls_arc4_free
        #define arc4_init                                         mbedtls_arc4_init
        #define arc4_self_test                                    mbedtls_arc4_self_test
        #define arc4_setup                                        mbedtls_arc4_setup
        #define asn1_bitstring                                    mbedtls_asn1_bitstring
        #define asn1_buf                                          mbedtls_asn1_buf
        #define asn1_find_named_data                              mbedtls_asn1_find_named_data
        #define asn1_free_named_data                              mbedtls_asn1_free_named_data
        #define asn1_free_named_data_list                         mbedtls_asn1_free_named_data_list
        #define asn1_get_alg                                      mbedtls_asn1_get_alg
        #define asn1_get_alg_null                                 mbedtls_asn1_get_alg_null
        #define asn1_get_bitstring                                mbedtls_asn1_get_bitstring
        #define asn1_get_bitstring_null                           mbedtls_asn1_get_bitstring_null
        #define asn1_get_bool                                     mbedtls_asn1_get_bool
        #define asn1_get_int                                      mbedtls_asn1_get_int
        #define asn1_get_len                                      mbedtls_asn1_get_len
        #define asn1_get_mpi                                      mbedtls_asn1_get_mpi
        #define asn1_get_sequence_of                              mbedtls_asn1_get_sequence_of
        #define asn1_get_tag                                      mbedtls_asn1_get_tag
        #define asn1_named_data                                   mbedtls_asn1_named_data
        #define asn1_sequence                                     mbedtls_asn1_sequence
        #define asn1_store_named_data                             mbedtls_asn1_store_named_data
        #define asn1_write_algorithm_identifier                   mbedtls_asn1_write_algorithm_identifier
        #define asn1_write_bitstring                              mbedtls_asn1_write_bitstring
        #define asn1_write_bool                                   mbedtls_asn1_write_bool
        #define asn1_write_ia5_string                             mbedtls_asn1_write_ia5_string
        #define asn1_write_int                                    mbedtls_asn1_write_int
        #define asn1_write_len                                    mbedtls_asn1_write_len
        #define asn1_write_mpi                                    mbedtls_asn1_write_mpi
        #define asn1_write_null                                   mbedtls_asn1_write_null
        #define asn1_write_octet_string                           mbedtls_asn1_write_octet_string
        #define asn1_write_oid                                    mbedtls_asn1_write_oid
        #define asn1_write_printable_string                       mbedtls_asn1_write_printable_string
        #define asn1_write_raw_buffer                             mbedtls_asn1_write_raw_buffer
        #define asn1_write_tag                                    mbedtls_asn1_write_tag
        #define base64_decode                                     mbedtls_base64_decode
        #define base64_encode                                     mbedtls_base64_encode
        #define base64_self_test                                  mbedtls_base64_self_test
        #define blowfish_context                                  mbedtls_blowfish_context
        #define blowfish_crypt_cbc                                mbedtls_blowfish_crypt_cbc
        #define blowfish_crypt_cfb64                              mbedtls_blowfish_crypt_cfb64
        #define blowfish_crypt_ctr                                mbedtls_blowfish_crypt_ctr
        #define blowfish_crypt_ecb                                mbedtls_blowfish_crypt_ecb
        #define blowfish_free                                     mbedtls_blowfish_free
        #define blowfish_init                                     mbedtls_blowfish_init
        #define blowfish_setkey                                   mbedtls_blowfish_setkey
        #define camellia_context                                  mbedtls_camellia_context
        #define camellia_crypt_cbc                                mbedtls_camellia_crypt_cbc
        #define camellia_crypt_cfb128                             mbedtls_camellia_crypt_cfb128
        #define camellia_crypt_ctr                                mbedtls_camellia_crypt_ctr
        #define camellia_crypt_ecb                                mbedtls_camellia_crypt_ecb
        #define camellia_free                                     mbedtls_camellia_free
        #define camellia_init                                     mbedtls_camellia_init
        #define camellia_self_test                                mbedtls_camellia_self_test
        #define camellia_setkey_dec                               mbedtls_camellia_setkey_dec
        #define camellia_setkey_enc                               mbedtls_camellia_setkey_enc
        #define ccm_auth_decrypt                                  mbedtls_ccm_auth_decrypt
        #define ccm_context                                       mbedtls_ccm_context
        #define ccm_encrypt_and_tag                               mbedtls_ccm_encrypt_and_tag
        #define ccm_free                                          mbedtls_ccm_free
        #define ccm_init                                          mbedtls_ccm_init
        #define ccm_self_test                                     mbedtls_ccm_self_test
        #define cipher_auth_decrypt                               mbedtls_cipher_auth_decrypt
        #define cipher_auth_encrypt                               mbedtls_cipher_auth_encrypt
        #define cipher_base_t                                     mbedtls_cipher_base_t
        #define cipher_check_tag                                  mbedtls_cipher_check_tag
        #define cipher_context_t                                  mbedtls_cipher_context_t
        #define cipher_crypt                                      mbedtls_cipher_crypt
        #define cipher_definition_t                               mbedtls_cipher_definition_t
        #define cipher_definitions                                mbedtls_cipher_definitions
        #define cipher_finish                                     mbedtls_cipher_finish
        #define cipher_free                                       mbedtls_cipher_free
        #define cipher_get_block_size                             mbedtls_cipher_get_block_size
        #define cipher_get_cipher_mode                            mbedtls_cipher_get_cipher_mode
        #define cipher_get_iv_size                                mbedtls_cipher_get_iv_size
        #define cipher_get_key_size                               mbedtls_cipher_get_key_bitlen
        #define cipher_get_name                                   mbedtls_cipher_get_name
        #define cipher_get_operation                              mbedtls_cipher_get_operation
        #define cipher_get_type                                   mbedtls_cipher_get_type
        #define cipher_id_t                                       mbedtls_cipher_id_t
        #define cipher_info_from_string                           mbedtls_cipher_info_from_string
        #define cipher_info_from_type                             mbedtls_cipher_info_from_type
        #define cipher_info_from_values                           mbedtls_cipher_info_from_values
        #define cipher_info_t                                     mbedtls_cipher_info_t
        #define cipher_init                                       mbedtls_cipher_init
        #define cipher_init_ctx                                   mbedtls_cipher_setup
        #define cipher_list                                       mbedtls_cipher_list
        #define cipher_mode_t                                     mbedtls_cipher_mode_t
        #define cipher_padding_t                                  mbedtls_cipher_padding_t
        #define cipher_reset                                      mbedtls_cipher_reset
        #define cipher_set_iv                                     mbedtls_cipher_set_iv
        #define cipher_set_padding_mode                           mbedtls_cipher_set_padding_mode
        #define cipher_setkey                                     mbedtls_cipher_setkey
        #define cipher_type_t                                     mbedtls_cipher_type_t
        #define cipher_update                                     mbedtls_cipher_update
        #define cipher_update_ad                                  mbedtls_cipher_update_ad
        #define cipher_write_tag                                  mbedtls_cipher_write_tag
        #define ctr_drbg_context                                  mbedtls_ctr_drbg_context
        #define ctr_drbg_free                                     mbedtls_ctr_drbg_free
        #define ctr_drbg_init                                     mbedtls_ctr_drbg_init
        #define ctr_drbg_random                                   mbedtls_ctr_drbg_random
        #define ctr_drbg_random_with_add                          mbedtls_ctr_drbg_random_with_add
        #define ctr_drbg_reseed                                   mbedtls_ctr_drbg_reseed
        #define ctr_drbg_self_test                                mbedtls_ctr_drbg_self_test
        #define ctr_drbg_set_entropy_len                          mbedtls_ctr_drbg_set_entropy_len
        #define ctr_drbg_set_prediction_resistance                mbedtls_ctr_drbg_set_prediction_resistance
        #define ctr_drbg_set_reseed_interval                      mbedtls_ctr_drbg_set_reseed_interval
        #define ctr_drbg_update                                   mbedtls_ctr_drbg_update
        #define ctr_drbg_update_seed_file                         mbedtls_ctr_drbg_update_seed_file
        #define ctr_drbg_write_seed_file                          mbedtls_ctr_drbg_write_seed_file
        #define debug_print_buf                                   mbedtls_debug_print_buf
        #define debug_print_crt                                   mbedtls_debug_print_crt
        #define debug_print_ecp                                   mbedtls_debug_print_ecp
        #define debug_print_mpi                                   mbedtls_debug_print_mpi
        #define debug_print_msg                                   mbedtls_debug_print_msg
        #define debug_print_ret                                   mbedtls_debug_print_ret
        #define debug_set_threshold                               mbedtls_debug_set_threshold
        #define des3_context                                      mbedtls_des3_context
        #define des3_crypt_cbc                                    mbedtls_des3_crypt_cbc
        #define des3_crypt_ecb                                    mbedtls_des3_crypt_ecb
        #define des3_free                                         mbedtls_des3_free
        #define des3_init                                         mbedtls_des3_init
        #define des3_set2key_dec                                  mbedtls_des3_set2key_dec
        #define des3_set2key_enc                                  mbedtls_des3_set2key_enc
        #define des3_set3key_dec                                  mbedtls_des3_set3key_dec
        #define des3_set3key_enc                                  mbedtls_des3_set3key_enc
        #define des_context                                       mbedtls_des_context
        #define des_crypt_cbc                                     mbedtls_des_crypt_cbc
        #define des_crypt_ecb                                     mbedtls_des_crypt_ecb
        #define des_free                                          mbedtls_des_free
        #define des_init                                          mbedtls_des_init
        #define des_key_check_key_parity                          mbedtls_des_key_check_key_parity
        #define des_key_check_weak                                mbedtls_des_key_check_weak
        #define des_key_set_parity                                mbedtls_des_key_set_parity
        #define des_self_test                                     mbedtls_des_self_test
        #define des_setkey_dec                                    mbedtls_des_setkey_dec
        #define des_setkey_enc                                    mbedtls_des_setkey_enc
        #define dhm_calc_secret                                   mbedtls_dhm_calc_secret
        #define dhm_context                                       mbedtls_dhm_context
        #define dhm_free                                          mbedtls_dhm_free
        #define dhm_init                                          mbedtls_dhm_init
        #define dhm_make_params                                   mbedtls_dhm_make_params
        #define dhm_make_public                                   mbedtls_dhm_make_public
        #define dhm_parse_dhm                                     mbedtls_dhm_parse_dhm
        #define dhm_parse_dhmfile                                 mbedtls_dhm_parse_dhmfile
        #define dhm_read_params                                   mbedtls_dhm_read_params
        #define dhm_read_public                                   mbedtls_dhm_read_public
        #define dhm_self_test                                     mbedtls_dhm_self_test
        #define ecdh_calc_secret                                  mbedtls_ecdh_calc_secret
        #define ecdh_compute_shared                               mbedtls_ecdh_compute_shared
        #define ecdh_context                                      mbedtls_ecdh_context
        #define ecdh_free                                         mbedtls_ecdh_free
        #define ecdh_gen_public                                   mbedtls_ecdh_gen_public
        #define ecdh_get_params                                   mbedtls_ecdh_get_params
        #define ecdh_init                                         mbedtls_ecdh_init
        #define ecdh_make_params                                  mbedtls_ecdh_make_params
        #define ecdh_make_public                                  mbedtls_ecdh_make_public
        #define ecdh_read_params                                  mbedtls_ecdh_read_params
        #define ecdh_read_public                                  mbedtls_ecdh_read_public
        #define ecdh_side                                         mbedtls_ecdh_side
        #define ecdsa_context                                     mbedtls_ecdsa_context
        #define ecdsa_free                                        mbedtls_ecdsa_free
        #define ecdsa_from_keypair                                mbedtls_ecdsa_from_keypair
        #define ecdsa_genkey                                      mbedtls_ecdsa_genkey
        #define ecdsa_info                                        mbedtls_ecdsa_info
        #define ecdsa_init                                        mbedtls_ecdsa_init
        #define ecdsa_read_signature                              mbedtls_ecdsa_read_signature
        #define ecdsa_sign                                        mbedtls_ecdsa_sign
        #define ecdsa_sign_det                                    mbedtls_ecdsa_sign_det
        #define ecdsa_verify                                      mbedtls_ecdsa_verify
        #define ecdsa_write_signature                             mbedtls_ecdsa_write_signature
        #define ecdsa_write_signature_det                         mbedtls_ecdsa_write_signature_det
        #define eckey_info                                        mbedtls_eckey_info
        #define eckeydh_info                                      mbedtls_eckeydh_info
        #define ecp_check_privkey                                 mbedtls_ecp_check_privkey
        #define ecp_check_pub_priv                                mbedtls_ecp_check_pub_priv
        #define ecp_check_pubkey                                  mbedtls_ecp_check_pubkey
        #define ecp_copy                                          mbedtls_ecp_copy
        #define ecp_curve_info                                    mbedtls_ecp_curve_info
        #define ecp_curve_info_from_grp_id                        mbedtls_ecp_curve_info_from_grp_id
        #define ecp_curve_info_from_name                          mbedtls_ecp_curve_info_from_name
        #define ecp_curve_info_from_tls_id                        mbedtls_ecp_curve_info_from_tls_id
        #define ecp_curve_list                                    mbedtls_ecp_curve_list
        #define ecp_gen_key                                       mbedtls_ecp_gen_key
        #define ecp_gen_keypair                                   mbedtls_ecp_gen_keypair
        #define ecp_group                                         mbedtls_ecp_group
        #define ecp_group_copy                                    mbedtls_ecp_group_copy
        #define ecp_group_free                                    mbedtls_ecp_group_free
        #define ecp_group_id                                      mbedtls_ecp_group_id
        #define ecp_group_init                                    mbedtls_ecp_group_init
        #define ecp_grp_id_list                                   mbedtls_ecp_grp_id_list
        #define ecp_is_zero                                       mbedtls_ecp_is_zero
        #define ecp_keypair                                       mbedtls_ecp_keypair
        #define ecp_keypair_free                                  mbedtls_ecp_keypair_free
        #define ecp_keypair_init                                  mbedtls_ecp_keypair_init
        #define ecp_mul                                           mbedtls_ecp_mul
        #define ecp_point                                         mbedtls_ecp_point
        #define ecp_point_free                                    mbedtls_ecp_point_free
        #define ecp_point_init                                    mbedtls_ecp_point_init
        #define ecp_point_read_binary                             mbedtls_ecp_point_read_binary
        #define ecp_point_read_string                             mbedtls_ecp_point_read_string
        #define ecp_point_write_binary                            mbedtls_ecp_point_write_binary
        #define ecp_self_test                                     mbedtls_ecp_self_test
        #define ecp_set_zero                                      mbedtls_ecp_set_zero
        #define ecp_tls_read_group                                mbedtls_ecp_tls_read_group
        #define ecp_tls_read_point                                mbedtls_ecp_tls_read_point
        #define ecp_tls_write_group                               mbedtls_ecp_tls_write_group
        #define ecp_tls_write_point                               mbedtls_ecp_tls_write_point
        #define ecp_use_known_dp                                  mbedtls_ecp_group_load
        #define entropy_add_source                                mbedtls_entropy_add_source
        #define entropy_context                                   mbedtls_entropy_context
        #define entropy_free                                      mbedtls_entropy_free
        #define entropy_func                                      mbedtls_entropy_func
        #define entropy_gather                                    mbedtls_entropy_gather
        #define entropy_init                                      mbedtls_entropy_init
        #define entropy_self_test                                 mbedtls_entropy_self_test
        #define entropy_update_manual                             mbedtls_entropy_update_manual
        #define entropy_update_seed_file                          mbedtls_entropy_update_seed_file
        #define entropy_write_seed_file                           mbedtls_entropy_write_seed_file
        #define error_strerror                                    mbedtls_strerror
        #define f_source_ptr                                      mbedtls_entropy_f_source_ptr
        #define gcm_auth_decrypt                                  mbedtls_gcm_auth_decrypt
        #define gcm_context                                       mbedtls_gcm_context
        #define gcm_crypt_and_tag                                 mbedtls_gcm_crypt_and_tag
        #define gcm_finish                                        mbedtls_gcm_finish
        #define gcm_free                                          mbedtls_gcm_free
        #define gcm_init                                          mbedtls_gcm_init
        #define gcm_self_test                                     mbedtls_gcm_self_test
        #define gcm_starts                                        mbedtls_gcm_starts
        #define gcm_update                                        mbedtls_gcm_update
        #define get_timer                                         mbedtls_timing_get_timer
        #define hardclock                                         mbedtls_timing_hardclock
        #define hardclock_poll                                    mbedtls_hardclock_poll
        #define havege_free                                       mbedtls_havege_free
        #define havege_init                                       mbedtls_havege_init
        #define havege_poll                                       mbedtls_havege_poll
        #define havege_random                                     mbedtls_havege_random
        #define havege_state                                      mbedtls_havege_state
        #define hmac_drbg_context                                 mbedtls_hmac_drbg_context
        #define hmac_drbg_free                                    mbedtls_hmac_drbg_free
        #define hmac_drbg_init                                    mbedtls_hmac_drbg_init
        #define hmac_drbg_random                                  mbedtls_hmac_drbg_random
        #define hmac_drbg_random_with_add                         mbedtls_hmac_drbg_random_with_add
        #define hmac_drbg_reseed                                  mbedtls_hmac_drbg_reseed
        #define hmac_drbg_self_test                               mbedtls_hmac_drbg_self_test
        #define hmac_drbg_set_entropy_len                         mbedtls_hmac_drbg_set_entropy_len
        #define hmac_drbg_set_prediction_resistance               mbedtls_hmac_drbg_set_prediction_resistance
        #define hmac_drbg_set_reseed_interval                     mbedtls_hmac_drbg_set_reseed_interval
        #define hmac_drbg_update                                  mbedtls_hmac_drbg_update
        #define hmac_drbg_update_seed_file                        mbedtls_hmac_drbg_update_seed_file
        #define hmac_drbg_write_seed_file                         mbedtls_hmac_drbg_write_seed_file
        #define hr_time                                           mbedtls_timing_hr_time
        #define key_exchange_type_t                               mbedtls_key_exchange_type_t
        #define md                                                mbedtls_md
        #define md2                                               mbedtls_md2
        #define md2_context                                       mbedtls_md2_context
        #define md2_finish                                        mbedtls_md2_finish
        #define md2_free                                          mbedtls_md2_free
        #define md2_info                                          mbedtls_md2_info
        #define md2_init                                          mbedtls_md2_init
        #define md2_process                                       mbedtls_md2_process
        #define md2_self_test                                     mbedtls_md2_self_test
        #define md2_starts                                        mbedtls_md2_starts
        #define md2_update                                        mbedtls_md2_update
        #define md4                                               mbedtls_md4
        #define md4_context                                       mbedtls_md4_context
        #define md4_finish                                        mbedtls_md4_finish
        #define md4_free                                          mbedtls_md4_free
        #define md4_info                                          mbedtls_md4_info
        #define md4_init                                          mbedtls_md4_init
        #define md4_process                                       mbedtls_md4_process
        #define md4_self_test                                     mbedtls_md4_self_test
        #define md4_starts                                        mbedtls_md4_starts
        #define md4_update                                        mbedtls_md4_update
        #define md5                                               mbedtls_md5
        #define md5_context                                       mbedtls_md5_context
        #define md5_finish                                        mbedtls_md5_finish
        #define md5_free                                          mbedtls_md5_free
        #define md5_info                                          mbedtls_md5_info
        #define md5_init                                          mbedtls_md5_init
        #define md5_process                                       mbedtls_md5_process
        #define md5_self_test                                     mbedtls_md5_self_test
        #define md5_starts                                        mbedtls_md5_starts
        #define md5_update                                        mbedtls_md5_update
        #define md_context_t                                      mbedtls_md_context_t
        #define md_file                                           mbedtls_md_file
        #define md_finish                                         mbedtls_md_finish
        #define md_free                                           mbedtls_md_free
        #define md_get_name                                       mbedtls_md_get_name
        #define md_get_size                                       mbedtls_md_get_size
        #define md_get_type                                       mbedtls_md_get_type
        #define md_hmac                                           mbedtls_md_hmac
        #define md_hmac_finish                                    mbedtls_md_hmac_finish
        #define md_hmac_reset                                     mbedtls_md_hmac_reset
        #define md_hmac_starts                                    mbedtls_md_hmac_starts
        #define md_hmac_update                                    mbedtls_md_hmac_update
        #define md_info_from_string                               mbedtls_md_info_from_string
        #define md_info_from_type                                 mbedtls_md_info_from_type
        #define md_info_t                                         mbedtls_md_info_t
        #define md_init                                           mbedtls_md_init
        #define md_init_ctx                                       mbedtls_md_init_ctx
        #define md_list                                           mbedtls_md_list
        #define md_process                                        mbedtls_md_process
        #define md_starts                                         mbedtls_md_starts
        #define md_type_t                                         mbedtls_md_type_t
        #define md_update                                         mbedtls_md_update
        #define memory_buffer_alloc_cur_get                       mbedtls_memory_buffer_alloc_cur_get
        #define memory_buffer_alloc_free                          mbedtls_memory_buffer_alloc_free
        #define memory_buffer_alloc_init                          mbedtls_memory_buffer_alloc_init
        #define memory_buffer_alloc_max_get                       mbedtls_memory_buffer_alloc_max_get
        #define memory_buffer_alloc_max_reset                     mbedtls_memory_buffer_alloc_max_reset
        #define memory_buffer_alloc_self_test                     mbedtls_memory_buffer_alloc_self_test
        #define memory_buffer_alloc_status                        mbedtls_memory_buffer_alloc_status
        #define memory_buffer_alloc_verify                        mbedtls_memory_buffer_alloc_verify
        #define memory_buffer_set_verify                          mbedtls_memory_buffer_set_verify
        #define mpi                                               mbedtls_mpi
        #define mpi_add_abs                                       mbedtls_mpi_add_abs
        #define mpi_add_int                                       mbedtls_mpi_add_int
        #define mpi_add_mpi                                       mbedtls_mpi_add_mpi
        #define mpi_cmp_abs                                       mbedtls_mpi_cmp_abs
        #define mpi_cmp_int                                       mbedtls_mpi_cmp_int
        #define mpi_cmp_mpi                                       mbedtls_mpi_cmp_mpi
        #define mpi_copy                                          mbedtls_mpi_copy
        #define mpi_div_int                                       mbedtls_mpi_div_int
        #define mpi_div_mpi                                       mbedtls_mpi_div_mpi
        #define mpi_exp_mod                                       mbedtls_mpi_exp_mod
        #define mpi_fill_random                                   mbedtls_mpi_fill_random
        #define mpi_free                                          mbedtls_mpi_free
        #define mpi_gcd                                           mbedtls_mpi_gcd
        #define mpi_gen_prime                                     mbedtls_mpi_gen_prime
        #define mpi_get_bit                                       mbedtls_mpi_get_bit
        #define mpi_grow                                          mbedtls_mpi_grow
        #define mpi_init                                          mbedtls_mpi_init
        #define mpi_inv_mod                                       mbedtls_mpi_inv_mod
        #define mpi_is_prime                                      mbedtls_mpi_is_prime
        #define mpi_lsb                                           mbedtls_mpi_lsb
        #define mpi_lset                                          mbedtls_mpi_lset
        #define mpi_mod_int                                       mbedtls_mpi_mod_int
        #define mpi_mod_mpi                                       mbedtls_mpi_mod_mpi
        #define mpi_msb                                           mbedtls_mpi_bitlen
        #define mpi_mul_int                                       mbedtls_mpi_mul_int
        #define mpi_mul_mpi                                       mbedtls_mpi_mul_mpi
        #define mpi_read_binary                                   mbedtls_mpi_read_binary
        #define mpi_read_file                                     mbedtls_mpi_read_file
        #define mpi_read_string                                   mbedtls_mpi_read_string
        #define mpi_safe_cond_assign                              mbedtls_mpi_safe_cond_assign
        #define mpi_safe_cond_swap                                mbedtls_mpi_safe_cond_swap
        #define mpi_self_test                                     mbedtls_mpi_self_test
        #define mpi_set_bit                                       mbedtls_mpi_set_bit
        #define mpi_shift_l                                       mbedtls_mpi_shift_l
        #define mpi_shift_r                                       mbedtls_mpi_shift_r
        #define mpi_shrink                                        mbedtls_mpi_shrink
        #define mpi_size                                          mbedtls_mpi_size
        #define mpi_sub_abs                                       mbedtls_mpi_sub_abs
        #define mpi_sub_int                                       mbedtls_mpi_sub_int
        #define mpi_sub_mpi                                       mbedtls_mpi_sub_mpi
        #define mpi_swap                                          mbedtls_mpi_swap
        #define mpi_write_binary                                  mbedtls_mpi_write_binary
        #define mpi_write_file                                    mbedtls_mpi_write_file
        #define mpi_write_string                                  mbedtls_mpi_write_string
        #define net_accept                                        mbedtls_net_accept
        #define net_bind                                          mbedtls_net_bind
        #define net_close                                         mbedtls_net_free
        #define net_connect                                       mbedtls_net_connect
        #define net_recv                                          mbedtls_net_recv
        #define net_recv_timeout                                  mbedtls_net_recv_timeout
        #define net_send                                          mbedtls_net_send
        #define net_set_block                                     mbedtls_net_set_block
        #define net_set_nonblock                                  mbedtls_net_set_nonblock
        #define net_usleep                                        mbedtls_net_usleep
        #define oid_descriptor_t                                  mbedtls_oid_descriptor_t
        #define oid_get_attr_short_name                           mbedtls_oid_get_attr_short_name
        #define oid_get_cipher_alg                                mbedtls_oid_get_cipher_alg
        #define oid_get_ec_grp                                    mbedtls_oid_get_ec_grp
        #define oid_get_extended_key_usage                        mbedtls_oid_get_extended_key_usage
        #define oid_get_md_alg                                    mbedtls_oid_get_md_alg
        #define oid_get_numeric_string                            mbedtls_oid_get_numeric_string
        #define oid_get_oid_by_ec_grp                             mbedtls_oid_get_oid_by_ec_grp
        #define oid_get_oid_by_md                                 mbedtls_oid_get_oid_by_md
        #define oid_get_oid_by_pk_alg                             mbedtls_oid_get_oid_by_pk_alg
        #define oid_get_oid_by_sig_alg                            mbedtls_oid_get_oid_by_sig_alg
        #define oid_get_pk_alg                                    mbedtls_oid_get_pk_alg
        #define oid_get_pkcs12_pbe_alg                            mbedtls_oid_get_pkcs12_pbe_alg
        #define oid_get_sig_alg                                   mbedtls_oid_get_sig_alg
        #define oid_get_sig_alg_desc                              mbedtls_oid_get_sig_alg_desc
        #define oid_get_x509_ext_type                             mbedtls_oid_get_x509_ext_type
        #define operation_t                                       mbedtls_operation_t
        #define padlock_supports                                  mbedtls_padlock_has_support
        #define padlock_xcryptcbc                                 mbedtls_padlock_xcryptcbc
        #define padlock_xcryptecb                                 mbedtls_padlock_xcryptecb
        #define pem_context                                       mbedtls_pem_context
        #define pem_free                                          mbedtls_pem_free
        #define pem_init                                          mbedtls_pem_init
        #define pem_read_buffer                                   mbedtls_pem_read_buffer
        #define pem_write_buffer                                  mbedtls_pem_write_buffer
        #define pk_can_do                                         mbedtls_pk_can_do
        #define pk_check_pair                                     mbedtls_pk_check_pair
        #define pk_context                                        mbedtls_pk_context
        #define pk_debug                                          mbedtls_pk_debug
        #define pk_debug_item                                     mbedtls_pk_debug_item
        #define pk_debug_type                                     mbedtls_pk_debug_type
        #define pk_decrypt                                        mbedtls_pk_decrypt
        #define pk_ec                                             mbedtls_pk_ec
        #define pk_encrypt                                        mbedtls_pk_encrypt
        #define pk_free                                           mbedtls_pk_free
        #define pk_get_len                                        mbedtls_pk_get_len
        #define pk_get_name                                       mbedtls_pk_get_name
        #define pk_get_size                                       mbedtls_pk_get_bitlen
        #define pk_get_type                                       mbedtls_pk_get_type
        #define pk_info_from_type                                 mbedtls_pk_info_from_type
        #define pk_info_t                                         mbedtls_pk_info_t
        #define pk_init                                           mbedtls_pk_init
        #define pk_init_ctx                                       mbedtls_pk_setup
        #define pk_init_ctx_rsa_alt                               mbedtls_pk_setup_rsa_alt
        #define pk_load_file                                      mbedtls_pk_load_file
        #define pk_parse_key                                      mbedtls_pk_parse_key
        #define pk_parse_keyfile                                  mbedtls_pk_parse_keyfile
        #define pk_parse_public_key                               mbedtls_pk_parse_public_key
        #define pk_parse_public_keyfile                           mbedtls_pk_parse_public_keyfile
        #define pk_parse_subpubkey                                mbedtls_pk_parse_subpubkey
        #define pk_rsa                                            mbedtls_pk_rsa
        #define pk_rsa_alt_decrypt_func                           mbedtls_pk_rsa_alt_decrypt_func
        #define pk_rsa_alt_key_len_func                           mbedtls_pk_rsa_alt_key_len_func
        #define pk_rsa_alt_sign_func                              mbedtls_pk_rsa_alt_sign_func
        #define pk_rsassa_pss_options                             mbedtls_pk_rsassa_pss_options
        #define pk_sign                                           mbedtls_pk_sign
        #define pk_type_t                                         mbedtls_pk_type_t
        #define pk_verify                                         mbedtls_pk_verify
        #define pk_verify_ext                                     mbedtls_pk_verify_ext
        #define pk_write_key_der                                  mbedtls_pk_write_key_der
        #define pk_write_key_pem                                  mbedtls_pk_write_key_pem
        #define pk_write_pubkey                                   mbedtls_pk_write_pubkey
        #define pk_write_pubkey_der                               mbedtls_pk_write_pubkey_der
        #define pk_write_pubkey_pem                               mbedtls_pk_write_pubkey_pem
        #define pkcs11_context                                    mbedtls_pkcs11_context
        #define pkcs11_decrypt                                    mbedtls_pkcs11_decrypt
        #define pkcs11_priv_key_free                              mbedtls_pkcs11_priv_key_free
        #define pkcs11_priv_key_init                              mbedtls_pkcs11_priv_key_bind
        #define pkcs11_sign                                       mbedtls_pkcs11_sign
        #define pkcs11_x509_cert_init                             mbedtls_pkcs11_x509_cert_bind
        #define pkcs12_derivation                                 mbedtls_pkcs12_derivation
        #define pkcs12_pbe                                        mbedtls_pkcs12_pbe
        #define pkcs12_pbe_sha1_rc4_128                           mbedtls_pkcs12_pbe_sha1_rc4_128
        #define pkcs5_pbes2                                       mbedtls_pkcs5_pbes2
        #define pkcs5_pbkdf2_hmac                                 mbedtls_pkcs5_pbkdf2_hmac
        #define pkcs5_self_test                                   mbedtls_pkcs5_self_test
        #define platform_entropy_poll                             mbedtls_platform_entropy_poll
        #define platform_set_exit                                 mbedtls_platform_set_exit
        #define platform_set_fprintf                              mbedtls_platform_set_fprintf
        #define platform_set_printf                               mbedtls_platform_set_printf
        #define platform_set_snprintf                             mbedtls_platform_set_snprintf
        #define polarssl_exit                                     mbedtls_exit
        #define polarssl_fprintf                                  mbedtls_fprintf
        #define polarssl_free                                     mbedtls_free
        #define polarssl_mutex_free                               mbedtls_mutex_free
        #define polarssl_mutex_init                               mbedtls_mutex_init
        #define polarssl_mutex_lock                               mbedtls_mutex_lock
        #define polarssl_mutex_unlock                             mbedtls_mutex_unlock
        #define polarssl_printf                                   mbedtls_printf
        #define polarssl_snprintf                                 mbedtls_snprintf
        #define polarssl_strerror                                 mbedtls_strerror
        #define ripemd160                                         mbedtls_ripemd160
        #define ripemd160_context                                 mbedtls_ripemd160_context
        #define ripemd160_finish                                  mbedtls_ripemd160_finish
        #define ripemd160_free                                    mbedtls_ripemd160_free
        #define ripemd160_info                                    mbedtls_ripemd160_info
        #define ripemd160_init                                    mbedtls_ripemd160_init
        #define ripemd160_process                                 mbedtls_ripemd160_process
        #define ripemd160_self_test                               mbedtls_ripemd160_self_test
        #define ripemd160_starts                                  mbedtls_ripemd160_starts
        #define ripemd160_update                                  mbedtls_ripemd160_update
        #define rsa_alt_context                                   mbedtls_rsa_alt_context
        #define rsa_alt_info                                      mbedtls_rsa_alt_info
        #define rsa_check_privkey                                 mbedtls_rsa_check_privkey
        #define rsa_check_pub_priv                                mbedtls_rsa_check_pub_priv
        #define rsa_check_pubkey                                  mbedtls_rsa_check_pubkey
        #define rsa_context                                       mbedtls_rsa_context
        #define rsa_copy                                          mbedtls_rsa_copy
        #define rsa_free                                          mbedtls_rsa_free
        #define rsa_gen_key                                       mbedtls_rsa_gen_key
        #define rsa_info                                          mbedtls_rsa_info
        #define rsa_init                                          mbedtls_rsa_init
        #define rsa_pkcs1_decrypt                                 mbedtls_rsa_pkcs1_decrypt
        #define rsa_pkcs1_encrypt                                 mbedtls_rsa_pkcs1_encrypt
        #define rsa_pkcs1_sign                                    mbedtls_rsa_pkcs1_sign
        #define rsa_pkcs1_verify                                  mbedtls_rsa_pkcs1_verify
        #define rsa_private                                       mbedtls_rsa_private
        #define rsa_public                                        mbedtls_rsa_public
        #define rsa_rsaes_oaep_decrypt                            mbedtls_rsa_rsaes_oaep_decrypt
        #define rsa_rsaes_oaep_encrypt                            mbedtls_rsa_rsaes_oaep_encrypt
        #define rsa_rsaes_pkcs1_v15_decrypt                       mbedtls_rsa_rsaes_pkcs1_v15_decrypt
        #define rsa_rsaes_pkcs1_v15_encrypt                       mbedtls_rsa_rsaes_pkcs1_v15_encrypt
        #define rsa_rsassa_pkcs1_v15_sign                         mbedtls_rsa_rsassa_pkcs1_v15_sign
        #define rsa_rsassa_pkcs1_v15_verify                       mbedtls_rsa_rsassa_pkcs1_v15_verify
        #define rsa_rsassa_pss_sign                               mbedtls_rsa_rsassa_pss_sign
        #define rsa_rsassa_pss_verify                             mbedtls_rsa_rsassa_pss_verify
        #define rsa_rsassa_pss_verify_ext                         mbedtls_rsa_rsassa_pss_verify_ext
        #define rsa_self_test                                     mbedtls_rsa_self_test
        #define rsa_set_padding                                   mbedtls_rsa_set_padding
        #define safer_memcmp                                      mbedtls_ssl_safer_memcmp
        #define set_alarm                                         mbedtls_set_alarm
        #define sha1                                              mbedtls_sha1
        #define sha1_context                                      mbedtls_sha1_context
        #define sha1_finish                                       mbedtls_sha1_finish
        #define sha1_free                                         mbedtls_sha1_free
        #define sha1_info                                         mbedtls_sha1_info
        #define sha1_init                                         mbedtls_sha1_init
        #define sha1_process                                      mbedtls_sha1_process
        #define sha1_self_test                                    mbedtls_sha1_self_test
        #define sha1_starts                                       mbedtls_sha1_starts
        #define sha1_update                                       mbedtls_sha1_update
        #define sha224_info                                       mbedtls_sha224_info
        #define sha256                                            mbedtls_sha256
        #define sha256_context                                    mbedtls_sha256_context
        #define sha256_finish                                     mbedtls_sha256_finish
        #define sha256_free                                       mbedtls_sha256_free
        #define sha256_info                                       mbedtls_sha256_info
        #define sha256_init                                       mbedtls_sha256_init
        #define sha256_process                                    mbedtls_sha256_process
        #define sha256_self_test                                  mbedtls_sha256_self_test
        #define sha256_starts                                     mbedtls_sha256_starts
        #define sha256_update                                     mbedtls_sha256_update
        #define sha384_info                                       mbedtls_sha384_info
        #define sha512                                            mbedtls_sha512
        #define sha512_context                                    mbedtls_sha512_context
        #define sha512_finish                                     mbedtls_sha512_finish
        #define sha512_free                                       mbedtls_sha512_free
        #define sha512_info                                       mbedtls_sha512_info
        #define sha512_init                                       mbedtls_sha512_init
        #define sha512_process                                    mbedtls_sha512_process
        #define sha512_self_test                                  mbedtls_sha512_self_test
        #define sha512_starts                                     mbedtls_sha512_starts
        #define sha512_update                                     mbedtls_sha512_update
        #define source_state                                      mbedtls_entropy_source_state
        #define ssl_cache_context                                 mbedtls_ssl_cache_context
        #define ssl_cache_entry                                   mbedtls_ssl_cache_entry
        #define ssl_cache_free                                    mbedtls_ssl_cache_free
        #define ssl_cache_get                                     mbedtls_ssl_cache_get
        #define ssl_cache_init                                    mbedtls_ssl_cache_init
        #define ssl_cache_set                                     mbedtls_ssl_cache_set
        #define ssl_cache_set_max_entries                         mbedtls_ssl_cache_set_max_entries
        #define ssl_cache_set_timeout                             mbedtls_ssl_cache_set_timeout
        #define ssl_check_cert_usage                              mbedtls_ssl_check_cert_usage
        #define ssl_ciphersuite_from_id                           mbedtls_ssl_ciphersuite_from_id
        #define ssl_ciphersuite_from_string                       mbedtls_ssl_ciphersuite_from_string
        #define ssl_ciphersuite_t                                 mbedtls_ssl_ciphersuite_t
        #define ssl_ciphersuite_uses_ec                           mbedtls_ssl_ciphersuite_uses_ec
        #define ssl_ciphersuite_uses_psk                          mbedtls_ssl_ciphersuite_uses_psk
        #define ssl_close_notify                                  mbedtls_ssl_close_notify
        #define ssl_context                                       mbedtls_ssl_context
        #define ssl_cookie_check                                  mbedtls_ssl_cookie_check
        #define ssl_cookie_check_t                                mbedtls_ssl_cookie_check_t
        #define ssl_cookie_ctx                                    mbedtls_ssl_cookie_ctx
        #define ssl_cookie_free                                   mbedtls_ssl_cookie_free
        #define ssl_cookie_init                                   mbedtls_ssl_cookie_init
        #define ssl_cookie_set_timeout                            mbedtls_ssl_cookie_set_timeout
        #define ssl_cookie_setup                                  mbedtls_ssl_cookie_setup
        #define ssl_cookie_write                                  mbedtls_ssl_cookie_write
        #define ssl_cookie_write_t                                mbedtls_ssl_cookie_write_t
        #define ssl_derive_keys                                   mbedtls_ssl_derive_keys
        #define ssl_dtls_replay_check                             mbedtls_ssl_dtls_replay_check
        #define ssl_dtls_replay_update                            mbedtls_ssl_dtls_replay_update
        #define ssl_fetch_input                                   mbedtls_ssl_fetch_input
        #define ssl_flight_item                                   mbedtls_ssl_flight_item
        #define ssl_flush_output                                  mbedtls_ssl_flush_output
        #define ssl_free                                          mbedtls_ssl_free
        #define ssl_get_alpn_protocol                             mbedtls_ssl_get_alpn_protocol
        #define ssl_get_bytes_avail                               mbedtls_ssl_get_bytes_avail
        #define ssl_get_ciphersuite                               mbedtls_ssl_get_ciphersuite
        #define ssl_get_ciphersuite_id                            mbedtls_ssl_get_ciphersuite_id
        #define ssl_get_ciphersuite_name                          mbedtls_ssl_get_ciphersuite_name
        #define ssl_get_ciphersuite_sig_pk_alg                    mbedtls_ssl_get_ciphersuite_sig_pk_alg
        #define ssl_get_peer_cert                                 mbedtls_ssl_get_peer_cert
        #define ssl_get_record_expansion                          mbedtls_ssl_get_record_expansion
        #define ssl_get_session                                   mbedtls_ssl_get_session
        #define ssl_get_verify_result                             mbedtls_ssl_get_verify_result
        #define ssl_get_version                                   mbedtls_ssl_get_version
        #define ssl_handshake                                     mbedtls_ssl_handshake
        #define ssl_handshake_client_step                         mbedtls_ssl_handshake_client_step
        #define ssl_handshake_free                                mbedtls_ssl_handshake_free
        #define ssl_handshake_params                              mbedtls_ssl_handshake_params
        #define ssl_handshake_server_step                         mbedtls_ssl_handshake_server_step
        #define ssl_handshake_step                                mbedtls_ssl_handshake_step
        #define ssl_handshake_wrapup                              mbedtls_ssl_handshake_wrapup
        #define ssl_hdr_len                                       mbedtls_ssl_hdr_len
        #define ssl_hs_hdr_len                                    mbedtls_ssl_hs_hdr_len
        #define ssl_hw_record_activate                            mbedtls_ssl_hw_record_activate
        #define ssl_hw_record_finish                              mbedtls_ssl_hw_record_finish
        #define ssl_hw_record_init                                mbedtls_ssl_hw_record_init
        #define ssl_hw_record_read                                mbedtls_ssl_hw_record_read
        #define ssl_hw_record_reset                               mbedtls_ssl_hw_record_reset
        #define ssl_hw_record_write                               mbedtls_ssl_hw_record_write
        #define ssl_init                                          mbedtls_ssl_init
        #define ssl_key_cert                                      mbedtls_ssl_key_cert
        #define ssl_legacy_renegotiation                          mbedtls_ssl_conf_legacy_renegotiation
        #define ssl_list_ciphersuites                             mbedtls_ssl_list_ciphersuites
        #define ssl_md_alg_from_hash                              mbedtls_ssl_md_alg_from_hash
        #define ssl_optimize_checksum                             mbedtls_ssl_optimize_checksum
        #define ssl_own_cert                                      mbedtls_ssl_own_cert
        #define ssl_own_key                                       mbedtls_ssl_own_key
        #define ssl_parse_certificate                             mbedtls_ssl_parse_certificate
        #define ssl_parse_change_cipher_spec                      mbedtls_ssl_parse_change_cipher_spec
        #define ssl_parse_finished                                mbedtls_ssl_parse_finished
        #define ssl_pk_alg_from_sig                               mbedtls_ssl_pk_alg_from_sig
        #define ssl_pkcs11_decrypt                                mbedtls_ssl_pkcs11_decrypt
        #define ssl_pkcs11_key_len                                mbedtls_ssl_pkcs11_key_len
        #define ssl_pkcs11_sign                                   mbedtls_ssl_pkcs11_sign
        #define ssl_psk_derive_premaster                          mbedtls_ssl_psk_derive_premaster
        #define ssl_read                                          mbedtls_ssl_read
        #define ssl_read_record                                   mbedtls_ssl_read_record
        #define ssl_read_version                                  mbedtls_ssl_read_version
        #define ssl_recv_flight_completed                         mbedtls_ssl_recv_flight_completed
        #define ssl_renegotiate                                   mbedtls_ssl_renegotiate
        #define ssl_resend                                        mbedtls_ssl_resend
        #define ssl_reset_checksum                                mbedtls_ssl_reset_checksum
        #define ssl_send_alert_message                            mbedtls_ssl_send_alert_message
        #define ssl_send_fatal_handshake_failure                  mbedtls_ssl_send_fatal_handshake_failure
        #define ssl_send_flight_completed                         mbedtls_ssl_send_flight_completed
        #define ssl_session                                       mbedtls_ssl_session
        #define ssl_session_free                                  mbedtls_ssl_session_free
        #define ssl_session_init                                  mbedtls_ssl_session_init
        #define ssl_session_reset                                 mbedtls_ssl_session_reset
        #define ssl_set_alpn_protocols                            mbedtls_ssl_conf_alpn_protocols
        #define ssl_set_arc4_support                              mbedtls_ssl_conf_arc4_support
        #define ssl_set_authmode                                  mbedtls_ssl_conf_authmode
        #define ssl_set_bio                                       mbedtls_ssl_set_bio
        #define ssl_set_ca_chain                                  mbedtls_ssl_conf_ca_chain
        #define ssl_set_cbc_record_splitting                      mbedtls_ssl_conf_cbc_record_splitting
        #define ssl_set_ciphersuites                              mbedtls_ssl_conf_ciphersuites
        #define ssl_set_ciphersuites_for_version                  mbedtls_ssl_conf_ciphersuites_for_version
        #define ssl_set_client_transport_id                       mbedtls_ssl_set_client_transport_id
        #define ssl_set_curves                                    mbedtls_ssl_conf_curves
        #define ssl_set_dbg                                       mbedtls_ssl_conf_dbg
        #define ssl_set_dh_param                                  mbedtls_ssl_conf_dh_param
        #define ssl_set_dh_param_ctx                              mbedtls_ssl_conf_dh_param_ctx
        #define ssl_set_dtls_anti_replay                          mbedtls_ssl_conf_dtls_anti_replay
        #define ssl_set_dtls_badmac_limit                         mbedtls_ssl_conf_dtls_badmac_limit
        #define ssl_set_dtls_cookies                              mbedtls_ssl_conf_dtls_cookies
        #define ssl_set_encrypt_then_mac                          mbedtls_ssl_conf_encrypt_then_mac
        #define ssl_set_endpoint                                  mbedtls_ssl_conf_endpoint
        #define ssl_set_extended_master_secret                    mbedtls_ssl_conf_extended_master_secret
        #define ssl_set_fallback                                  mbedtls_ssl_conf_fallback
        #define ssl_set_handshake_timeout                         mbedtls_ssl_conf_handshake_timeout
        #define ssl_set_hostname                                  mbedtls_ssl_set_hostname
        #define ssl_set_max_frag_len                              mbedtls_ssl_conf_max_frag_len
        #define ssl_set_max_version                               mbedtls_ssl_conf_max_version
        #define ssl_set_min_version                               mbedtls_ssl_conf_min_version
        #define ssl_set_own_cert                                  mbedtls_ssl_conf_own_cert
        #define ssl_set_psk                                       mbedtls_ssl_conf_psk
        #define ssl_set_psk_cb                                    mbedtls_ssl_conf_psk_cb
        #define ssl_set_renegotiation                             mbedtls_ssl_conf_renegotiation
        #define ssl_set_renegotiation_enforced                    mbedtls_ssl_conf_renegotiation_enforced
        #define ssl_set_renegotiation_period                      mbedtls_ssl_conf_renegotiation_period
        #define ssl_set_rng                                       mbedtls_ssl_conf_rng
        #define ssl_set_session                                   mbedtls_ssl_set_session
        #define ssl_set_session_cache                             mbedtls_ssl_conf_session_cache
        #define ssl_set_session_tickets                           mbedtls_ssl_conf_session_tickets
        #define ssl_set_sni                                       mbedtls_ssl_conf_sni
        #define ssl_set_transport                                 mbedtls_ssl_conf_transport
        #define ssl_set_truncated_hmac                            mbedtls_ssl_conf_truncated_hmac
        #define ssl_set_verify                                    mbedtls_ssl_conf_verify
        #define ssl_sig_from_pk                                   mbedtls_ssl_sig_from_pk
        #define ssl_states                                        mbedtls_ssl_states
        #define ssl_transform                                     mbedtls_ssl_transform
        #define ssl_transform_free                                mbedtls_ssl_transform_free
        #define ssl_write                                         mbedtls_ssl_write
        #define ssl_write_certificate                             mbedtls_ssl_write_certificate
        #define ssl_write_change_cipher_spec                      mbedtls_ssl_write_change_cipher_spec
        #define ssl_write_finished                                mbedtls_ssl_write_finished
        #define ssl_write_record                                  mbedtls_ssl_write_record
        #define ssl_write_version                                 mbedtls_ssl_write_version
        #define supported_ciphers                                 mbedtls_cipher_supported
        #define t_sint                                            mbedtls_mpi_sint
        #define t_udbl                                            mbedtls_t_udbl
        #define t_uint                                            mbedtls_mpi_uint
        #define test_ca_crt                                       mbedtls_test_ca_crt
        #define test_ca_crt_ec                                    mbedtls_test_ca_crt_ec
        #define test_ca_crt_rsa                                   mbedtls_test_ca_crt_rsa
        #define test_ca_key                                       mbedtls_test_ca_key
        #define test_ca_key_ec                                    mbedtls_test_ca_key_ec
        #define test_ca_key_rsa                                   mbedtls_test_ca_key_rsa
        #define test_ca_list                                      mbedtls_test_cas_pem
        #define test_ca_pwd                                       mbedtls_test_ca_pwd
        #define test_ca_pwd_ec                                    mbedtls_test_ca_pwd_ec
        #define test_ca_pwd_rsa                                   mbedtls_test_ca_pwd_rsa
        #define test_cli_crt                                      mbedtls_test_cli_crt
        #define test_cli_crt_ec                                   mbedtls_test_cli_crt_ec
        #define test_cli_crt_rsa                                  mbedtls_test_cli_crt_rsa
        #define test_cli_key                                      mbedtls_test_cli_key
        #define test_cli_key_ec                                   mbedtls_test_cli_key_ec
        #define test_cli_key_rsa                                  mbedtls_test_cli_key_rsa
        #define test_srv_crt                                      mbedtls_test_srv_crt
        #define test_srv_crt_ec                                   mbedtls_test_srv_crt_ec
        #define test_srv_crt_rsa                                  mbedtls_test_srv_crt_rsa
        #define test_srv_key                                      mbedtls_test_srv_key
        #define test_srv_key_ec                                   mbedtls_test_srv_key_ec
        #define test_srv_key_rsa                                  mbedtls_test_srv_key_rsa
        #define threading_mutex_t                                 mbedtls_threading_mutex_t
        #define threading_set_alt                                 mbedtls_threading_set_alt
        #define timing_self_test                                  mbedtls_timing_self_test
        #define version_check_feature                             mbedtls_version_check_feature
        #define version_get_number                                mbedtls_version_get_number
        #define version_get_string                                mbedtls_version_get_string
        #define version_get_string_full                           mbedtls_version_get_string_full
        #define x509_bitstring                                    mbedtls_x509_bitstring
        #define x509_buf                                          mbedtls_x509_buf
        #define x509_crl                                          mbedtls_x509_crl
        #define x509_crl_entry                                    mbedtls_x509_crl_entry
        #define x509_crl_free                                     mbedtls_x509_crl_free
        #define x509_crl_info                                     mbedtls_x509_crl_info
        #define x509_crl_init                                     mbedtls_x509_crl_init
        #define x509_crl_parse                                    mbedtls_x509_crl_parse
        #define x509_crl_parse_der                                mbedtls_x509_crl_parse_der
        #define x509_crl_parse_file                               mbedtls_x509_crl_parse_file
        #define x509_crt                                          mbedtls_x509_crt
        #define x509_crt_check_extended_key_usage                 mbedtls_x509_crt_check_extended_key_usage
        #define x509_crt_check_key_usage                          mbedtls_x509_crt_check_key_usage
        #define x509_crt_free                                     mbedtls_x509_crt_free
        #define x509_crt_info                                     mbedtls_x509_crt_info
        #define x509_crt_init                                     mbedtls_x509_crt_init
        #define x509_crt_parse                                    mbedtls_x509_crt_parse
        #define x509_crt_parse_der                                mbedtls_x509_crt_parse_der
        #define x509_crt_parse_file                               mbedtls_x509_crt_parse_file
        #define x509_crt_parse_path                               mbedtls_x509_crt_parse_path
        #define x509_crt_revoked                                  mbedtls_x509_crt_is_revoked
        #define x509_crt_verify                                   mbedtls_x509_crt_verify
        #define x509_csr                                          mbedtls_x509_csr
        #define x509_csr_free                                     mbedtls_x509_csr_free
        #define x509_csr_info                                     mbedtls_x509_csr_info
        #define x509_csr_init                                     mbedtls_x509_csr_init
        #define x509_csr_parse                                    mbedtls_x509_csr_parse
        #define x509_csr_parse_der                                mbedtls_x509_csr_parse_der
        #define x509_csr_parse_file                               mbedtls_x509_csr_parse_file
        #define x509_dn_gets                                      mbedtls_x509_dn_gets
        #define x509_get_alg                                      mbedtls_x509_get_alg
        #define x509_get_alg_null                                 mbedtls_x509_get_alg_null
        #define x509_get_ext                                      mbedtls_x509_get_ext
        #define x509_get_name                                     mbedtls_x509_get_name
        #define x509_get_rsassa_pss_params                        mbedtls_x509_get_rsassa_pss_params
        #define x509_get_serial                                   mbedtls_x509_get_serial
        #define x509_get_sig                                      mbedtls_x509_get_sig
        #define x509_get_sig_alg                                  mbedtls_x509_get_sig_alg
        #define x509_get_time                                     mbedtls_x509_get_time
        #define x509_key_size_helper                              mbedtls_x509_key_size_helper
        #define x509_name                                         mbedtls_x509_name
        #define x509_self_test                                    mbedtls_x509_self_test
        #define x509_sequence                                     mbedtls_x509_sequence
        #define x509_serial_gets                                  mbedtls_x509_serial_gets
        #define x509_set_extension                                mbedtls_x509_set_extension
        #define x509_sig_alg_gets                                 mbedtls_x509_sig_alg_gets
        #define x509_string_to_names                              mbedtls_x509_string_to_names
        #define x509_time                                         mbedtls_x509_time
        #define x509_time_expired                                 mbedtls_x509_time_is_past
        #define x509_time_future                                  mbedtls_x509_time_is_future
        #define x509_write_extensions                             mbedtls_x509_write_extensions
        #define x509_write_names                                  mbedtls_x509_write_names
        #define x509_write_sig                                    mbedtls_x509_write_sig
        #define x509write_cert                                    mbedtls_x509write_cert
        #define x509write_crt_der                                 mbedtls_x509write_crt_der
        #define x509write_crt_free                                mbedtls_x509write_crt_free
        #define x509write_crt_init                                mbedtls_x509write_crt_init
        #define x509write_crt_pem                                 mbedtls_x509write_crt_pem
        #define x509write_crt_set_authority_key_identifier        mbedtls_x509write_crt_set_authority_key_identifier
        #define x509write_crt_set_basic_constraints               mbedtls_x509write_crt_set_basic_constraints
        #define x509write_crt_set_extension                       mbedtls_x509write_crt_set_extension
        #define x509write_crt_set_issuer_key                      mbedtls_x509write_crt_set_issuer_key
        #define x509write_crt_set_issuer_name                     mbedtls_x509write_crt_set_issuer_name
        #define x509write_crt_set_key_usage                       mbedtls_x509write_crt_set_key_usage
        #define x509write_crt_set_md_alg                          mbedtls_x509write_crt_set_md_alg
        #define x509write_crt_set_ns_cert_type                    mbedtls_x509write_crt_set_ns_cert_type
        #define x509write_crt_set_serial                          mbedtls_x509write_crt_set_serial
        #define x509write_crt_set_subject_key                     mbedtls_x509write_crt_set_subject_key
        #define x509write_crt_set_subject_key_identifier          mbedtls_x509write_crt_set_subject_key_identifier
        #define x509write_crt_set_subject_name                    mbedtls_x509write_crt_set_subject_name
        #define x509write_crt_set_validity                        mbedtls_x509write_crt_set_validity
        #define x509write_crt_set_version                         mbedtls_x509write_crt_set_version
        #define x509write_csr                                     mbedtls_x509write_csr
        #define x509write_csr_der                                 mbedtls_x509write_csr_der
        #define x509write_csr_free                                mbedtls_x509write_csr_free
        #define x509write_csr_init                                mbedtls_x509write_csr_init
        #define x509write_csr_pem                                 mbedtls_x509write_csr_pem
        #define x509write_csr_set_extension                       mbedtls_x509write_csr_set_extension
        #define x509write_csr_set_key                             mbedtls_x509write_csr_set_key
        #define x509write_csr_set_key_usage                       mbedtls_x509write_csr_set_key_usage
        #define x509write_csr_set_md_alg                          mbedtls_x509write_csr_set_md_alg
        #define x509write_csr_set_ns_cert_type                    mbedtls_x509write_csr_set_ns_cert_type
        #define x509write_csr_set_subject_name                    mbedtls_x509write_csr_set_subject_name
        #define xtea_context                                      mbedtls_xtea_context
        #define xtea_crypt_cbc                                    mbedtls_xtea_crypt_cbc
        #define xtea_crypt_ecb                                    mbedtls_xtea_crypt_ecb
        #define xtea_free                                         mbedtls_xtea_free
        #define xtea_init                                         mbedtls_xtea_init
        #define xtea_self_test                                    mbedtls_xtea_self_test
        #define xtea_setup                                        mbedtls_xtea_setup

    #endif /* compat-1.3.h */
#endif /* MBEDTLS_DEPRECATED_REMOVED */
