/*******************************************************************************
 *
 * Copyright (c) 2013, 2014, 2015 Intel Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v20.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    David Navarro, Intel Corporation - initial API and implementation
 *    Benjamin Cabé - Please refer to git log
 *    Fabien Fleutot - Please refer to git log
 *    Simon Bernard - Please refer to git log
 *    Julien Vermillard - Please refer to git log
 *    Axel Lorente - Please refer to git log
 *    Toby Jaffey - Please refer to git log
 *    Bosch Software Innovations GmbH - Please refer to git log
 *    Pascal Rieux - Please refer to git log
 *    Christian Renz - Please refer to git log
 *
 *******************************************************************************/

/*
 Copyright (c) 2013, 2014 Intel Corporation

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

     * Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
     * Neither the name of Intel Corporation nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.

 David Navarro <david.navarro@intel.com>
 Bosch Software Innovations GmbH - Please refer to git log

*/

#include "liblwm2m.h"
#include "connection.h"

//#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
//#include <netdb.h>
//#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
//#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

extern lwm2m_object_t * get_object_device(void);
extern void free_object_device(lwm2m_object_t * objectP);
extern lwm2m_object_t * get_server_object(void);
extern void free_server_object(lwm2m_object_t * object);
extern lwm2m_object_t * get_security_object(void);
extern void free_security_object(lwm2m_object_t * objectP);
extern char * get_server_uri(lwm2m_object_t * objectP, uint16_t secObjInstID);
extern lwm2m_object_t * get_test_object(void);
extern void free_test_object(lwm2m_object_t * object);

#define MAX_PACKET_SIZE 2048

int g_reboot = 0;
static int g_quit = 0;

typedef struct
{
    lwm2m_object_t * securityObjP;
    int sock;
    connection_t * connList;
    int addressFamily;
} client_data_t;


void handle_sigint(int signum)
{
    g_quit = 1;
}

void * lwm2m_connect_server(uint16_t secObjInstID,
                            void * userData)
{
    client_data_t * dataP;
    char * uri;
    char * host;
    char * port;
    connection_t * newConnP = NULL;

    dataP = (client_data_t *)userData;

    uri = get_server_uri(dataP->securityObjP, secObjInstID);

    if (uri == NULL) return NULL;

    fprintf(stdout, "Connecting to %s\r\n", uri);

    // parse uri in the form "coaps://[host]:[port]"
    if (0 == strncmp(uri, "coaps://", strlen("coaps://")))
    {
        host = uri+strlen("coaps://");
    }
    else if (0 == strncmp(uri, "coap://", strlen("coap://")))
    {
        host = uri+strlen("coap://");
    }
    else
    {
        goto exit;
    }
    port = strrchr(host, ':');
    if (port == NULL) goto exit;
    // remove brackets
    if (host[0] == '[')
    {
        host++;
        if (*(port - 1) == ']')
        {
            *(port - 1) = 0;
        }
        else goto exit;
    }
    // split strings
    *port = 0;
    port++;

    newConnP = connection_create(dataP->connList, dataP->sock, host, port, dataP->addressFamily);
    if (newConnP == NULL) {
        IotLogError("Connection creation failed.\r\n");
    }
    else {
        dataP->connList = newConnP;
    }

exit:
    lwm2m_free(uri);
    return (void *)newConnP;
}

void lwm2m_close_connection(void * sessionH,
                            void * userData)
{
    client_data_t * app_data;
    connection_t * targetP;

    app_data = (client_data_t *)userData;
    targetP = (connection_t *)sessionH;

    if (targetP == app_data->connList)
    {
        app_data->connList = targetP->next;
        lwm2m_free(targetP);
    }
    else
    {
        connection_t * parentP;

        parentP = app_data->connList;
        while (parentP != NULL && parentP->next != targetP)
        {
            parentP = parentP->next;
        }
        if (parentP != NULL)
        {
            parentP->next = targetP->next;
            lwm2m_free(targetP);
        }
    }
}

void print_usage(void)
{
    fprintf(stdout, "Usage: lwm2mclient [OPTION]\r\n");
    fprintf(stdout, "Launch a LWM2M client.\r\n");
    fprintf(stdout, "Options:\r\n");
    fprintf(stdout, "  -n NAME\tSet the endpoint name of the Client. Default: testlightclient\r\n");
    fprintf(stdout, "  -l PORT\tSet the local UDP port of the Client. Default: 56830\r\n");
    fprintf(stdout, "  -4\t\tUse IPv4 connection. Default: IPv6 connection\r\n");
//    fprintf(stdout, "  -S BYTES\tCoAP block size. Options: 16, 32, 64, 128, 256, 512, 1024. Default: %" PRIu16 "\r\n",
//            LWM2M_COAP_DEFAULT_BLOCK_SIZE);
    fprintf(stdout, "\r\n");
}

void print_state(lwm2m_context_t * lwm2mH)
{
    lwm2m_server_t * targetP;

    IotLogInfo("State: ");
    switch(lwm2mH->state)
    {
    case STATE_INITIAL:
        IotLogInfo("STATE_INITIAL");
        break;
    case STATE_BOOTSTRAP_REQUIRED:
        IotLogInfo("STATE_BOOTSTRAP_REQUIRED");
        break;
    case STATE_BOOTSTRAPPING:
        IotLogInfo("STATE_BOOTSTRAPPING");
        break;
    case STATE_REGISTER_REQUIRED:
        IotLogInfo("STATE_REGISTER_REQUIRED");
        break;
    case STATE_REGISTERING:
        IotLogInfo("STATE_REGISTERING");
        break;
    case STATE_READY:
        IotLogInfo("STATE_READY");
        break;
    default:
        IotLogInfo("Unknown !");
        break;
    }
    IotLogInfo("\r\n");

    targetP = lwm2mH->bootstrapServerList;

    if (lwm2mH->bootstrapServerList == NULL)
    {
        IotLogError("No Bootstrap Server.\r\n");
    }
    else
    {
        IotLogInfo("Bootstrap Servers:\r\n");
        for (targetP = lwm2mH->bootstrapServerList ; targetP != NULL ; targetP = targetP->next)
        {
            IotLogInfo(" - Security Object ID %d", targetP->secObjInstID);
            IotLogInfo("\tHold Off Time: %lu s", (unsigned long)targetP->lifetime);
            IotLogInfo("\tstatus: ");
            switch(targetP->status)
            {
            case STATE_DEREGISTERED:
                IotLogInfo("DEREGISTERED\r\n");
                break;
            case STATE_BS_HOLD_OFF:
                IotLogInfo("CLIENT HOLD OFF\r\n");
                break;
            case STATE_BS_INITIATED:
                IotLogInfo("BOOTSTRAP INITIATED\r\n");
                break;
            case STATE_BS_PENDING:
                IotLogInfo("BOOTSTRAP PENDING\r\n");
                break;
            case STATE_BS_FINISHED:
                IotLogInfo("BOOTSTRAP FINISHED\r\n");
                break;
            case STATE_BS_FAILED:
                IotLogInfo("BOOTSTRAP FAILED\r\n");
                break;
            default:
                IotLogInfo("INVALID (%d)\r\n", (int)targetP->status);
            }
            IotLogInfo("\r\n");
        }
    }

    if (lwm2mH->serverList == NULL)
    {
        IotLogInfo("No LWM2M Server.\r\n");
    }
    else
    {
        IotLogInfo("LWM2M Servers:\r\n");
        for (targetP = lwm2mH->serverList ; targetP != NULL ; targetP = targetP->next)
        {
            IotLogInfo(" - Server ID %d", targetP->shortID);
            IotLogInfo("\tstatus: ");
            switch(targetP->status)
            {
            case STATE_DEREGISTERED:
                IotLogInfo("DEREGISTERED\r\n");
                break;
            case STATE_REG_PENDING:
                IotLogInfo("REGISTRATION PENDING\r\n");
                break;
            case STATE_REGISTERED:
                IotLogInfo("REGISTERED\tlocation: \"%s\"\tLifetime: %lus\r\n", targetP->location, (unsigned long)targetP->lifetime);
                break;
            case STATE_REG_UPDATE_PENDING:
                IotLogInfo("REGISTRATION UPDATE PENDING\r\n");
                break;
            case STATE_REG_UPDATE_NEEDED:
                IotLogInfo("REGISTRATION UPDATE REQUIRED\r\n");
                break;
            case STATE_DEREG_PENDING:
                IotLogInfo("DEREGISTRATION PENDING\r\n");
                break;
            case STATE_REG_FAILED:
                IotLogInfo("REGISTRATION FAILED\r\n");
                break;
            default:
                IotLogInfo("INVALID (%d)\r\n", (int)targetP->status);
            }
            IotLogInfo("\r\n");
        }
    }
}

#define OBJ_COUNT 4
