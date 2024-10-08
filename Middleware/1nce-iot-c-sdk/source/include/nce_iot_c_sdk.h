/*
 * MIT License
 *
 * Copyright (c) 2022 1NCE
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file nce_iot_c_sdk.h
 * @brief File containing the necessary information to connect to 1NCE services.
 *
 * @author  Hatim Jamali & Mohamed Abdelmaksoud
 * @date 1 Mar 2022
 */

#ifndef NCE_IOT_C_SDK_H_
#define NCE_IOT_C_SDK_H_
/* Standard includes. */
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include "udp_interface.h"

/**
 * @brief definition of return codes.
 */
enum
{
    NCE_SDK_SUCCESS = 0,              /**< The operation was successful. */
    NCE_SDK_CONNECT_ERROR = -1,       /**< Connection error. */
    NCE_SDK_SEND_ERROR = -2,          /**< Packet sending error. */
    NCE_SDK_RECEIVE_ERROR = -3,       /**< Packet reception error. */
    NCE_SDK_PARSING_ERROR = -4,       /**< Response parsing error. */
    NCE_SDK_BINARY_PAYLOAD_ERROR = -5 /**< Binary payload conversion error. */
};


#ifndef __ZEPHYR__

/**
 * @brief the maximum number of onboarding attempts.
 */
    #define NCE_SDK_ATTEMPTS    5

/**
 * @brief Enable 1NCE Device Authenticator.
 */
    #define  NCE_DEVICE_AUTHENTICATOR

/**
 * @brief Enable 1NCE Energy Saver.
 */
    #define  NCE_ENERGY_SAVER

/**
 * @brief the maximum string size in the payload before conversion.
 */
    #define NCE_SDK_MAX_STRING_SIZE    50
#endif /* ifndef __ZEPHYR__ */


#ifdef NCE_DEVICE_AUTHENTICATOR

/**
 * @brief Contains the credentials necessary for DTLS connection setup.
 */
typedef struct DtlsKey
{
    /**
     * @brief String representing a trusted secret key for authentication.
     */
    char Psk[ 100 ];

    /**
     * @brief String representing an identity indicated to the server (which key to use).
     */
    char PskIdentity[ 100 ];
} DtlsKey_t;

/**
 * @brief 1NCE onboarding endpoint.
 * @brief 1NCE onboarding port.
 */
static const OSEndPoint_t NceOnboard =
{
    .host = "coap.os.1nce.com",
    .port = 5683
};


/**
 * @brief Communicate with 1NCE Device Authenticator to get DTLS credentials
 *
 * @param[in] osNetwork: UDP interface object.
 * @param[in] nceKey: new DTLS credential required.
 *
 * @return The status of the onboarding.
 */
int os_auth( os_network_ops_t * osNetwork,
             DtlsKey_t * nceKey );


#endif /* ifdef NCE_DEVICE_AUTHENTICATOR */

#ifdef NCE_ENERGY_SAVER

/**
 * @brief Enums representing different type of 1NCE OS variables in the message.
 */
enum E_Type
{
    E_CHAR,
    E_FLOAT,
    E_INTEGER,
    E_STRING
};

/**
 * @brief Represents value send to our service.
 */
typedef struct Element_Gen
{
    /**
     * @brief Enums representing different type of 1NCE OS variables in the message.
     */
    enum E_Type type;

    /**
     * @brief Union type to access the byte message.
     * @note If you have float=f; string=s; int=i;
     */
    union E_Value_gen
    {
        char c;                                /**< if the variable is character */
        float f;                               /**< if the variable is float */
        int i;                                 /**< if the variable is integer */
        char s[ NCE_SDK_MAX_STRING_SIZE ];     /**< if the variable is string */
        char bytes[ NCE_SDK_MAX_STRING_SIZE ]; /**< for returning bytes */
    }

    /**
     * @brief value of the variable in message.
     */
    value;

    /**
     * @brief Length of the chunk in bytes.
     * @note Must be the same in the given template (1NCE portal).
     */
    int template_length;
}
Element2byte_gen_t;

/**
 * @brief Translation service feature: translates message to binary payload with a given template in 1NCE Portal.
 *
 * @param[in] packet: Packet to send to the endpoint
 * @param[in] selector: Select data used in control statement.
 * @param[in] num_args: Number of arguments in function.
 *
 * @return location in memory.
 */
int os_energy_save( char * packet,
                    int selector,
                    int num_args,
                    ... );

#endif /* ifdef NCE_ENERGY_SAVER */


#endif /* ifndef NCE_IOT_C_SDK_H_ */
