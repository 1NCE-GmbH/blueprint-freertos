/*******************************************************************************
 *
 * Copyright (c) 2015 Intel Corporation and others.
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
 *    Christian Renz - Please refer to git log
 *    Scott Bertin, AMETEK, Inc. - Please refer to git log
 *
 *******************************************************************************/

#include "liblwm2m.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <inttypes.h>

#include "commandline.h"
#include "connection.h"
#include "bootstrap_info.h"

#define CMD_STATUS_NEW  0
#define CMD_STATUS_SENT 1
#define CMD_STATUS_OK   2
#define CMD_STATUS_FAIL 3

typedef struct _endpoint_
{
    struct _endpoint_ * next;
    char *             name;
    lwm2m_media_type_t format;
    lwm2m_version_t    version;
    void *             handle;
    bs_command_t *     cmdList;
    uint8_t            status;
} endpoint_t;

typedef struct
{
    Socket_t          sock;
    connection_t *    connList;
    bs_info_t *       bsInfo;
    endpoint_t *      endpointList;
    int               addressFamily;
} internal_data_t;

#define MAX_PACKET_SIZE 2048

static int g_quit = 0;

static void prv_quit(lwm2m_context_t * lwm2mH,
                     char * buffer,
                     void * user_data)
{
    g_quit = 1;
}

void handle_sigint(int signum)
{
    prv_quit(NULL, NULL, NULL);
}

static void prv_print_uri(FILE * fd,
                          lwm2m_uri_t * uriP)
{
    fprintf(fd, "/");
    if (uriP != NULL)
    {
        fprintf(fd, "%hu", uriP->objectId);
        if (LWM2M_URI_IS_SET_INSTANCE(uriP))
        {
            fprintf(fd, "/%d", uriP->instanceId);
            if (LWM2M_URI_IS_SET_RESOURCE(uriP))
            {
                fprintf(fd, "/%d", uriP->resourceId);
#ifndef LWM2M_VERSION_1_0
                if (LWM2M_URI_IS_SET_RESOURCE_INSTANCE(uriP))
                    fprintf(fd, "/%d", uriP->resourceInstanceId);
#endif
            }
        }
    }
}
static void prv_endpoint_free(endpoint_t * endP)
{
    if (endP != NULL)
    {
        if (endP->name != NULL) free(endP->name);
        free(endP);
    }
}

static endpoint_t * prv_endpoint_find(internal_data_t * dataP,
                                      void * sessionH)
{
    endpoint_t * endP;

    endP = dataP->endpointList;

    while (endP != NULL
        && endP->handle != sessionH)
    {
        endP = endP->next;
    }

    return endP;
}

static endpoint_t * prv_endpoint_new(internal_data_t * dataP,
                                     void * sessionH)
{
    endpoint_t * endP;

    endP = prv_endpoint_find(dataP, sessionH);
    if (endP != NULL)
    {
        // delete previous state for the endpoint
        endpoint_t * parentP;

        parentP = dataP->endpointList;
        while (parentP != NULL
            && parentP->next != endP)
        {
            parentP = parentP->next;
        }
        if (parentP != NULL)
        {
            parentP->next = endP->next;
        }
        else
        {
            dataP->endpointList = endP->next;
        }
        prv_endpoint_free(endP);
    }

    endP = (endpoint_t *)malloc(sizeof(endpoint_t));
    return endP;
}

static void prv_endpoint_clean(internal_data_t * dataP)
{
    endpoint_t * endP;
    endpoint_t * parentP;

    while (dataP->endpointList != NULL
        && (dataP->endpointList->cmdList == NULL
         || dataP->endpointList->status == CMD_STATUS_FAIL))
    {
        endP = dataP->endpointList->next;
        prv_endpoint_free(dataP->endpointList);
        dataP->endpointList = endP;
    }

    parentP = dataP->endpointList;
    if (parentP != NULL)
    {
        endP = dataP->endpointList->next;
        while(endP != NULL)
        {
            endpoint_t * nextP;

            nextP = endP->next;
            if (endP->cmdList == NULL
            || endP->status == CMD_STATUS_FAIL)
            {
                prv_endpoint_free(endP);
                parentP->next = nextP;
            }
            else
            {
                parentP = endP;
            }
            endP = nextP;
        }
    }
}

static void prv_send_command(lwm2m_context_t *lwm2mH,
                             internal_data_t * dataP,
                             endpoint_t * endP)
{
    int res;

    if (endP->cmdList == NULL) return;

    switch (endP->cmdList->operation)
    {
    case BS_DISCOVER:
        IotLogInfo( "Sending DISCOVER ");
        prv_print_uri(stdout, endP->cmdList->uri);
        IotLogInfo( " to \"%s\"", endP->name);
        res = lwm2m_bootstrap_discover(lwm2mH, endP->handle, endP->cmdList->uri);
        break;

#ifndef LWM2M_VERSION_1_0
        case BS_READ:
        IotLogInfo("Sending READ ");
        prv_print_uri(stdout, endP->cmdList->uri);
        IotLogInfo( " to \"%s\"", endP->name);
        res = lwm2m_bootstrap_read(lwm2mH, endP->handle, endP->cmdList->uri);
        break;
#endif

    case BS_DELETE:
        IotLogInfo( "Sending DELETE ");
        prv_print_uri(stdout, endP->cmdList->uri);
        IotLogInfo( " to \"%s\"", endP->name);
        res = lwm2m_bootstrap_delete(lwm2mH, endP->handle, endP->cmdList->uri);
        break;

    case BS_WRITE_SECURITY:
    {
        lwm2m_uri_t uri;
        bs_server_data_t * serverP;
        uint8_t *securityData;
        lwm2m_media_type_t format = endP->format;

        serverP = (bs_server_data_t *)LWM2M_LIST_FIND(dataP->bsInfo->serverList, endP->cmdList->serverId);
        if (serverP == NULL
         || serverP->securityData == NULL)
        {
            endP->status = CMD_STATUS_FAIL;
            return;
        }

        LWM2M_URI_RESET(&uri);
        uri.objectId = LWM2M_SECURITY_OBJECT_ID;
        uri.instanceId = endP->cmdList->serverId;

        IotLogInfo( "Sending WRITE ");
        prv_print_uri(stdout, &uri);
        IotLogInfo( " to \"%s\"", endP->name);

        res = lwm2m_data_serialize(&uri, serverP->securitySize, serverP->securityData, &format, &securityData);
        if (res > 0)
        {
            res = lwm2m_bootstrap_write(lwm2mH, endP->handle, &uri, format, securityData, res);
            lwm2m_free(securityData);
        }
    }
        break;

    case BS_WRITE_SERVER:
    {
        lwm2m_uri_t uri;
        bs_server_data_t * serverP;
        uint8_t *serverData;
        lwm2m_media_type_t format = endP->format;

        serverP = (bs_server_data_t *)LWM2M_LIST_FIND(dataP->bsInfo->serverList, endP->cmdList->serverId);
        if (serverP == NULL
         || serverP->serverData == NULL)
        {
            endP->status = CMD_STATUS_FAIL;
            return;
        }

        LWM2M_URI_RESET(&uri);
        uri.objectId = LWM2M_SERVER_OBJECT_ID;
        uri.instanceId = endP->cmdList->serverId;

        IotLogInfo( "Sending WRITE ");
        prv_print_uri(stdout, &uri);
        IotLogInfo( " to \"%s\"", endP->name);

        res = lwm2m_data_serialize(&uri, serverP->serverSize, serverP->serverData, &format, &serverData);
        if (res > 0)
        {
            res = lwm2m_bootstrap_write(lwm2mH, endP->handle, &uri, format, serverData, res);
            lwm2m_free(serverData);
        }
    }
        break;

    case BS_FINISH:
        IotLogInfo( "Sending BOOTSTRAP FINISH ");
        IotLogInfo( " to \"%s\"", endP->name);

        res = lwm2m_bootstrap_finish(lwm2mH, endP->handle);
        break;

    default:
        return;
    }

    if (res == COAP_NO_ERROR)
    {
        IotLogInfo( " OK.\r\n");

        endP->status = CMD_STATUS_SENT;
    }
    else
    {
        IotLogInfo( " failed!\r\n");

        endP->status = CMD_STATUS_FAIL;
    }
}

static int prv_bootstrap_callback(lwm2m_context_t * lwm2mH,
                                  void * sessionH,
                                  uint8_t status,
                                  lwm2m_uri_t * uriP,
                                  char * name,
                                  lwm2m_media_type_t format,
                                  uint8_t * data,
                                  uint16_t dataLength,
                                  void * userData)
{
    internal_data_t * dataP = (internal_data_t *)userData;
    endpoint_t * endP;

    switch (status)
    {
    case COAP_NO_ERROR:
    {
        bs_endpoint_info_t * endInfoP;

        // Display
        IotLogInfo( "\r\nBootstrap request from \"%s\"\r\n", name);

        // find Bootstrap Info for this endpoint
        endInfoP = dataP->bsInfo->endpointList;
        while (endInfoP != NULL
            && endInfoP->name != NULL
            && strcmp(name, endInfoP->name) != 0)
        {
            endInfoP = endInfoP->next;
        }
        if (endInfoP == NULL)
        {
            // find default Bootstrap Info
            endInfoP = dataP->bsInfo->endpointList;
            while (endInfoP != NULL
                && endInfoP->name != NULL)
            {
                endInfoP = endInfoP->next;
            }
        }
        // Nothing found, discard the request
        if (endInfoP == NULL)return COAP_IGNORE;

        endP = prv_endpoint_new(dataP, sessionH);
        if (endP == NULL) return COAP_500_INTERNAL_SERVER_ERROR;

        endP->cmdList = endInfoP->commandList;
        endP->handle = sessionH;
        endP->name = strdup(name);
        endP->format = 0 == format ? LWM2M_CONTENT_TLV : format;
        endP->version = VERSION_MISSING;
        endP->status = CMD_STATUS_NEW;
        endP->next = dataP->endpointList;
        dataP->endpointList = endP;

        return COAP_204_CHANGED;
    }

    default:
        // Display
        IotLogInfo( "\r\n Received status ");
        print_status(stdout, status);
        IotLogInfo( " for URI ");
        prv_print_uri(stdout, uriP);

        endP = prv_endpoint_find(dataP, sessionH);
        if (endP == NULL)
        {
            IotLogInfo( " from unknown endpoint.\r\n");
            return COAP_NO_ERROR;
        }
        IotLogInfo( " from endpoint %s.\r\n", endP->name);

        // should not happen
        if (endP->cmdList == NULL) return COAP_NO_ERROR;
        if (endP->status != CMD_STATUS_SENT) return COAP_NO_ERROR;

        switch (endP->cmdList->operation)
        {
        case BS_DISCOVER:
            if (status == COAP_205_CONTENT)
            {
                uint8_t *start = data;
                if (dataLength > 4 && !memcmp(data, "</>;", 4))
                {
                    start += 4;
                }
                if (dataLength - (start-data) >= 10
                 && start[9] == ','
                 && start[7] == '.'
                 && !memcmp(start, "lwm2m=", 6))
                {
                    endP->version = VERSION_UNRECOGNIZED;
                    if (start[6] == '1')
                    {
                        if (start[8] == '0')
                        {
                            endP->version = VERSION_1_0;
                        }
                        else if (start[8] == '1')
                        {
                            endP->version = VERSION_1_1;
                        }
                    }
                }
                output_data(stdout, NULL, format, data, dataLength, 1);
            }
            // Can continue even if discover failed.
            endP->status = CMD_STATUS_OK;
            break;

#ifndef LWM2M_VERSION_1_0
        case BS_READ:
            if (status == COAP_205_CONTENT)
            {
                output_data(stdout, NULL, format, data, dataLength, 1);
            }
            // Can continue even if read failed.
            endP->status = CMD_STATUS_OK;
            break;
#endif

        case BS_DELETE:
            if (status == COAP_202_DELETED)
            {
                endP->status = CMD_STATUS_OK;
            }
            else
            {
                endP->status = CMD_STATUS_FAIL;
            }
            break;

        case BS_WRITE_SECURITY:
        case BS_WRITE_SERVER:
            if (status == COAP_204_CHANGED)
            {
                endP->status = CMD_STATUS_OK;
            }
            else
            {
                endP->status = CMD_STATUS_FAIL;
            }
            break;

        default:
            endP->status = CMD_STATUS_FAIL;
            break;
        }
        break;
    }

    return COAP_NO_ERROR;
}

static void prv_bootstrap_client(lwm2m_context_t *lwm2mH,
                                 char * buffer,
                                 void * user_data)
{
    internal_data_t * dataP = (internal_data_t *)user_data;
    char * uri;
    char * name = "";
    char * end = NULL;
    char * host;
    char * port;
    connection_t * newConnP = NULL;

    uri = buffer;
    end = get_end_of_arg(buffer);
    if (end[0] != 0)
    {
        *end = 0;
        buffer = end + 1;
        name = get_next_arg(buffer, &end);
    }
    if (!check_end_of_args(end)) goto syntax_error;

    // parse uri in the form "coaps://[host]:[port]"
    if (0==strncmp(uri, "coaps://", strlen("coaps://"))) {
        host = uri+strlen("coaps://");
    }
    else if (0==strncmp(uri, "coap://",  strlen("coap://"))) {
        host = uri+strlen("coap://");
    }
    else {
        goto syntax_error;
    }
    port = strrchr(host, ':');
    if (port == NULL) goto syntax_error;
    // remove brackets
    if (host[0] == '[')
    {
        host++;
        if (*(port - 1) == ']')
        {
            *(port - 1) = 0;
        }
        else goto syntax_error;
    }
    // split strings
    *port = 0;
    port++;

    IotLogInfo( "Trying to connect to LWM2M CLient at %s:%s\r\n", host, port);
    newConnP = connection_create(dataP->connList, dataP->sock, host, port, dataP->addressFamily);
    if (newConnP == NULL) {
        IotLogInfo( "Connection creation failed.\r\n");
        return;
    }
    dataP->connList = newConnP;

    // simulate a client bootstrap request.
    // Only LWM2M 1.0 clients support this method of bootstrap. For them, TLV
    // support is mandatory.
    if (COAP_204_CHANGED == prv_bootstrap_callback(lwm2mH, newConnP, COAP_NO_ERROR, NULL, name, LWM2M_CONTENT_TLV, NULL, 0, user_data))
    {
        IotLogInfo( "OK");
    }
    else
    {
        IotLogInfo( "Error");
    }
    return;

syntax_error:
    IotLogInfo( "Syntax error !");
}
