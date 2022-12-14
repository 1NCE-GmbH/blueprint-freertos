/*******************************************************************************
 *
 * Copyright (c) 2013, 2014 Intel Corporation and others.
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
 *    Ricky Liu - Please refer to git log
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
#include "nce_demo_config.h"
#if defined(CONFIG_LwM2M_DEMO_ENABLED)
#include "lwm2mclient.h"
#include "liblwm2m.h"
#include "commandline.h"
#ifdef WITH_TINYDTLS
#include "dtlsconnection.h"
#else
#include "connection.h"
#endif

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_PACKET_SIZE 2048
#define DEFAULT_SERVER_IPV6 "[::1]"
#define DEFAULT_SERVER_IPV4 "127.0.0.1"

int g_reboot = 0;
static int g_quit = 0;

#define OBJ_COUNT 9
lwm2m_object_t * objArray[OBJ_COUNT];

// only backup security and server objects
# define BACKUP_OBJECT_COUNT 2
lwm2m_object_t * backupObjectArray[BACKUP_OBJECT_COUNT];

typedef struct
{
    lwm2m_object_t * securityObjP;
    lwm2m_object_t * serverObject;
    int sock;
#ifdef WITH_TINYDTLS
    dtls_connection_t * connList;
    lwm2m_context_t * lwm2mH;
#else
    connection_t * connList;
#endif
    int addressFamily;
} client_data_t;

static void prv_quit(lwm2m_context_t * lwm2mH,
                     char * buffer,
                     void * user_data)
{
    /* unused parameters */
    (void)lwm2mH;
    (void)buffer;
    (void)user_data;

    g_quit = 1;
}

void handle_sigint(int signum)
{
    g_quit = 2;
}

void handle_value_changed(lwm2m_context_t * lwm2mH,
                          lwm2m_uri_t * uri,
                          const char * value,
                          size_t valueLength)
{
    lwm2m_object_t * object = (lwm2m_object_t *)LWM2M_LIST_FIND(lwm2mH->objectList, uri->objectId);

    if (NULL != object)
    {
        if (object->writeFunc != NULL)
        {
            lwm2m_data_t * dataP;
            int result;

            dataP = lwm2m_data_new(1);
            if (dataP == NULL)
            {
                IotLogError( "Internal allocation failure !\n");
                return;
            }
            dataP->id = uri->resourceId;

#ifndef LWM2M_VERSION_1_0
            if (LWM2M_URI_IS_SET_RESOURCE_INSTANCE(uri))
            {
                lwm2m_data_t *subDataP = lwm2m_data_new(1);
                if (subDataP == NULL)
                {
                    IotLogError( "Internal allocation failure !\n");
                    lwm2m_data_free(1, dataP);
                    return;
                }
                subDataP->id = uri->resourceInstanceId;
                lwm2m_data_encode_nstring(value, valueLength, subDataP);
                lwm2m_data_encode_instances(subDataP, 1, dataP);
            }
            else
#endif
            {
                lwm2m_data_encode_nstring(value, valueLength, dataP);
            }

            result = object->writeFunc(lwm2mH, uri->instanceId, 1, dataP, object, LWM2M_WRITE_PARTIAL_UPDATE);
            if (COAP_405_METHOD_NOT_ALLOWED == result)
            {
                switch (uri->objectId)
                {
                case LWM2M_DEVICE_OBJECT_ID:
                    result = device_change(dataP, object);
                    break;
                default:
                    break;
                }
            }

            if (COAP_204_CHANGED != result)
            {
                IotLogError( "Failed to change value!\n");
            }
            else
            {
                IotLogInfo( "value changed!\n");
                lwm2m_resource_value_changed(lwm2mH, uri);
            }
            lwm2m_data_free(1, dataP);
            return;
        }
        else
        {
            IotLogError( "write not supported for specified resource!\n");
        }
        return;
    }
    else
    {
        IotLogError( "Object not found !\n");
    }
}

#ifdef WITH_TINYDTLS
void * lwm2m_connect_server(uint16_t secObjInstID,
                            void * userData)
{
  client_data_t * dataP;
  lwm2m_list_t * instance;
  dtls_connection_t * newConnP = NULL;
  dataP = (client_data_t *)userData;
  lwm2m_object_t  * securityObj = dataP->securityObjP;

  instance = LWM2M_LIST_FIND(dataP->securityObjP->instanceList, secObjInstID);
  if (instance == NULL) return NULL;


  newConnP = connection_create(dataP->connList, dataP->sock, securityObj, instance->id, dataP->lwm2mH, dataP->addressFamily);
  if (newConnP == NULL)
  {
      IotLogError( "Connection creation failed.\n");
      return NULL;
  }

  dataP->connList = newConnP;
  return (void *)newConnP;
}
#else
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

    // parse uri in the form "coaps://[host]:[port]"
    if (0==strncmp(uri, "coaps://", strlen("coaps://"))) {
        host = uri+strlen("coaps://");
    }
    else if (0==strncmp(uri, "coap://",  strlen("coap://"))) {
        host = uri+strlen("coap://");
    }
    else {
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

    IotLogInfo("Opening connection to server at %s:%s\r\n", host, port);
    newConnP = connection_create(dataP->connList, dataP->sock, host, port, dataP->addressFamily);
    if (newConnP == NULL) {
        IotLogError( "Connection creation failed.\r\n");
    }
    else {
        dataP->connList = newConnP;
    }

exit:
    lwm2m_free(uri);
    return (void *)newConnP;
}
#endif

void lwm2m_close_connection(void * sessionH,
                            void * userData)
{
    client_data_t * app_data;
#ifdef WITH_TINYDTLS
    dtls_connection_t * targetP;
#else
    connection_t * targetP;
#endif

    app_data = (client_data_t *)userData;
#ifdef WITH_TINYDTLS
    targetP = (dtls_connection_t *)sessionH;
#else
    targetP = (connection_t *)sessionH;
#endif

    if (targetP == app_data->connList)
    {
        app_data->connList = targetP->next;
        lwm2m_free(targetP);
    }
    else
    {
#ifdef WITH_TINYDTLS
        dtls_connection_t * parentP;
#else
        connection_t * parentP;
#endif

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

static void prv_output_servers(lwm2m_context_t * lwm2mH,
                               char * buffer,
                               void * user_data)
{
    lwm2m_server_t * targetP;

    /* unused parameter */
    (void)user_data;

    targetP = lwm2mH->bootstrapServerList;

    if (lwm2mH->bootstrapServerList == NULL)
    {
        IotLogError( "No Bootstrap Server.\r\n");
    }
    else
    {
        IotLogInfo( "Bootstrap Servers:\r\n");
        for (targetP = lwm2mH->bootstrapServerList ; targetP != NULL ; targetP = targetP->next)
        {
            IotLogInfo( " - Security Object ID %d", targetP->secObjInstID);
            IotLogInfo( "\tHold Off Time: %lu s", (unsigned long)targetP->lifetime);
            IotLogInfo("\tstatus: ");
            switch(targetP->status)
            {
            case STATE_DEREGISTERED:
                IotLogInfo( "DEREGISTERED\r\n");
                break;
            case STATE_BS_HOLD_OFF:
                IotLogInfo( "CLIENT HOLD OFF\r\n");
                break;
            case STATE_BS_INITIATED:
                IotLogInfo( "BOOTSTRAP INITIATED\r\n");
                break;
            case STATE_BS_PENDING:
                IotLogInfo( "BOOTSTRAP PENDING\r\n");
                break;
            case STATE_BS_FINISHED:
                IotLogInfo( "BOOTSTRAP FINISHED\r\n");
                break;
            case STATE_BS_FAILED:
                IotLogInfo( "BOOTSTRAP FAILED\r\n");
                break;
            default:
                IotLogInfo( "INVALID (%d)\r\n", (int)targetP->status);
            }
        }
    }

    if (lwm2mH->serverList == NULL)
    {
        IotLogError( "No LWM2M Server.\r\n");
    }
    else
    {
        IotLogInfo( "LWM2M Servers:\r\n");
        for (targetP = lwm2mH->serverList ; targetP != NULL ; targetP = targetP->next)
        {
            IotLogInfo( " - Server ID %d", targetP->shortID);
            IotLogInfo( "\tstatus: ");
            switch(targetP->status)
            {
            case STATE_DEREGISTERED:
                IotLogInfo( "DEREGISTERED\r\n");
                break;
            case STATE_REG_PENDING:
                IotLogInfo( "REGISTRATION PENDING\r\n");
                break;
            case STATE_REGISTERED:
                IotLogInfo( "REGISTERED\tlocation: \"%s\"\tLifetime: %lus\r\n", targetP->location, (unsigned long)targetP->lifetime);
                break;
            case STATE_REG_UPDATE_PENDING:
                IotLogInfo( "REGISTRATION UPDATE PENDING\r\n");
                break;
            case STATE_DEREG_PENDING:
                IotLogInfo( "DEREGISTRATION PENDING\r\n");
                break;
            case STATE_REG_FAILED:
                IotLogInfo( "REGISTRATION FAILED\r\n");
                break;
            default:
                IotLogInfo( "INVALID (%d)\r\n", (int)targetP->status);
            }
        }
    }
}

static void prv_change(lwm2m_context_t * lwm2mH,
                       char * buffer,
                       void * user_data)
{
    lwm2m_uri_t uri;
    char * end = NULL;
    int result;

    /* unused parameter */
    (void)user_data;

    end = get_end_of_arg(buffer);
    if (end[0] == 0) goto syntax_error;

    result = lwm2m_stringToUri(buffer, end - buffer, &uri);
    if (result == 0) goto syntax_error;

    buffer = get_next_arg(end, &end);

    if (buffer[0] == 0)
    {
        IotLogInfo("report change!\n");
        lwm2m_resource_value_changed(lwm2mH, &uri);
    }
    else
    {
        handle_value_changed(lwm2mH, &uri, buffer, end - buffer);
    }
    return;

syntax_error:
    IotLogError( "Syntax error !\n");
}

static void prv_object_list(lwm2m_context_t * lwm2mH,
                            char * buffer,
                            void * user_data)
{
    lwm2m_object_t * objectP;

    /* unused parameter */
    (void)user_data;

    for (objectP = lwm2mH->objectList; objectP != NULL; objectP = objectP->next)
    {
        if (objectP->instanceList == NULL)
        {
            IotLogInfo( "/%d ", objectP->objID);
        }
        else
        {
            lwm2m_list_t * instanceP;

            for (instanceP = objectP->instanceList; instanceP != NULL ; instanceP = instanceP->next)
            {
                IotLogInfo( "/%d/%d  ", objectP->objID, instanceP->id);
            }
        }
        IotLogInfo( "\r\n");
    }
}

static void prv_instance_dump(lwm2m_context_t * lwm2mH,
                              lwm2m_object_t * objectP,
                              uint16_t id)
{
    int numData;
    lwm2m_data_t * dataArray;
    uint16_t res;

    numData = 0;
    res = objectP->readFunc(lwm2mH, id, &numData, &dataArray, objectP);
    if (res != COAP_205_CONTENT)
    {
        printf("Error ");
        print_status(stdout, res);
        printf("\r\n");
        return;
    }

    dump_tlv(stdout, numData, dataArray, 0);
}


static void prv_object_dump(lwm2m_context_t * lwm2mH,
                            char * buffer,
                            void * user_data)
{
    lwm2m_uri_t uri;
    char * end = NULL;
    int result;
    lwm2m_object_t * objectP;

    /* unused parameter */
    (void)user_data;

    end = get_end_of_arg(buffer);
    if (end[0] == 0) goto syntax_error;

    result = lwm2m_stringToUri(buffer, end - buffer, &uri);
    if (result == 0) goto syntax_error;
    if (LWM2M_URI_IS_SET_RESOURCE(&uri)) goto syntax_error;

    objectP = (lwm2m_object_t *)LWM2M_LIST_FIND(lwm2mH->objectList, uri.objectId);
    if (objectP == NULL)
    {
        IotLogError( "Object not found.\n");
        return;
    }

    if (LWM2M_URI_IS_SET_INSTANCE(&uri))
    {
        prv_instance_dump(lwm2mH, objectP, uri.instanceId);
    }
    else
    {
        lwm2m_list_t * instanceP;

        for (instanceP = objectP->instanceList; instanceP != NULL ; instanceP = instanceP->next)
        {
            IotLogInfo( "Instance %d:\r\n", instanceP->id);
            prv_instance_dump(lwm2mH, objectP, instanceP->id);
            IotLogInfo( "\r\n");
        }
    }

    return;

syntax_error:
    IotLogError( "Syntax error !\n");
}

static void prv_update(lwm2m_context_t * lwm2mH,
                       char * buffer,
                       void * user_data)
{
    /* unused parameter */
    (void)user_data;

    if (buffer[0] == 0) goto syntax_error;

    uint16_t serverId = (uint16_t) atoi(buffer);
    int res = lwm2m_update_registration(lwm2mH, serverId, false);
    if (res != 0)
    {
        IotLogError("Registration update error: ");
        print_status(stdout, res);
        IotLogError( "\r\n");
    }
    return;

syntax_error:
    IotLogError("Syntax error !\n");
}

static void update_battery_level(lwm2m_context_t * context)
{
    static time_t next_change_time = 0;
    time_t tv_sec;

    tv_sec = lwm2m_gettime();
    if (tv_sec < 0) return;

    if (next_change_time < tv_sec)
    {
        char value[15];
        int valueLength;
        lwm2m_uri_t uri;
        int level = rand() % 100;

        if (0 > level) level = -level;
        if (lwm2m_stringToUri("/3/0/9", 6, &uri))
        {
            valueLength = sprintf(value, "%d", level);
            IotLogInfo( "New Battery Level: %d\n", level);
            handle_value_changed(context, &uri, value, valueLength);
        }
        level = rand() % 20;
        if (0 > level) level = -level;
        next_change_time = tv_sec + level + 10;
    }
}

static void prv_add(lwm2m_context_t * lwm2mH,
                    char * buffer,
                    void * user_data)
{
    lwm2m_object_t * objectP;
    int res;

    /* unused parameter */
    (void)user_data;

    objectP = get_test_object();
    if (objectP == NULL)
    {
        IotLogError( "Creating object 31024 failed.\r\n");
        return;
    }
    res = lwm2m_add_object(lwm2mH, objectP);
    if (res != 0)
    {
        IotLogError("Adding object 31024 failed: ");
        print_status(stdout, res);
        IotLogError( "\r\n");
    }
    else
    {
        IotLogInfo( "Object 31024 added.\r\n");
    }
    return;
}

static void prv_remove(lwm2m_context_t * lwm2mH,
                       char * buffer,
                       void * user_data)
{
    int res;

    /* unused parameter */
    (void)user_data;

    res = lwm2m_remove_object(lwm2mH, 31024);
    if (res != 0)
    {
        IotLogError( "Removing object 31024 failed: ");
        print_status(stdout, res);
        IotLogError( "\r\n");
    }
    else
    {
        IotLogError( "Object 31024 removed.\r\n");
    }
    return;
}

#ifdef LWM2M_BOOTSTRAP

static void prv_initiate_bootstrap(lwm2m_context_t * lwm2mH,
                                   char * buffer,
                                   void * user_data)
{
    lwm2m_server_t * targetP;

    /* unused parameter */
    (void)user_data;

    // HACK !!!
    lwm2mH->state = STATE_BOOTSTRAP_REQUIRED;
    targetP = lwm2mH->bootstrapServerList;
    while (targetP != NULL)
    {
        targetP->lifetime = 0;
        targetP = targetP->next;
    }
}

static void prv_display_backup(lwm2m_context_t * lwm2mH,
                               char * buffer,
                               void * user_data)
{
   int i;

   /* unused parameters */
   (void)lwm2mH;
   (void)buffer;
   (void)user_data;

   for (i = 0 ; i < BACKUP_OBJECT_COUNT ; i++) {
       lwm2m_object_t * object = backupObjectArray[i];
       if (NULL != object) {
           switch (object->objID)
           {
           case LWM2M_SECURITY_OBJECT_ID:
               display_security_object(object);
               break;
           case LWM2M_SERVER_OBJECT_ID:
               display_server_object(object);
               break;
           default:
               break;
           }
       }
   }
}

static void prv_backup_objects(lwm2m_context_t * context)
{
    uint16_t i;

    for (i = 0; i < BACKUP_OBJECT_COUNT; i++) {
        if (NULL != backupObjectArray[i]) {
            switch (backupObjectArray[i]->objID)
            {
            case LWM2M_SECURITY_OBJECT_ID:
                clean_security_object(backupObjectArray[i]);
                lwm2m_free(backupObjectArray[i]);
                break;
            case LWM2M_SERVER_OBJECT_ID:
                clean_server_object(backupObjectArray[i]);
                lwm2m_free(backupObjectArray[i]);
                break;
            default:
                break;
            }
        }
        backupObjectArray[i] = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));
        memset(backupObjectArray[i], 0, sizeof(lwm2m_object_t));
    }

    /*
     * Backup content of objects 0 (security) and 1 (server)
     */
    copy_security_object(backupObjectArray[0], (lwm2m_object_t *)LWM2M_LIST_FIND(context->objectList, LWM2M_SECURITY_OBJECT_ID));
    copy_server_object(backupObjectArray[1], (lwm2m_object_t *)LWM2M_LIST_FIND(context->objectList, LWM2M_SERVER_OBJECT_ID));
}

static void prv_restore_objects(lwm2m_context_t * context)
{
    lwm2m_object_t * targetP;

    /*
     * Restore content  of objects 0 (security) and 1 (server)
     */
    targetP = (lwm2m_object_t *)LWM2M_LIST_FIND(context->objectList, LWM2M_SECURITY_OBJECT_ID);
    // first delete internal content
    clean_security_object(targetP);
    // then restore previous object
    copy_security_object(targetP, backupObjectArray[0]);

    targetP = (lwm2m_object_t *)LWM2M_LIST_FIND(context->objectList, LWM2M_SERVER_OBJECT_ID);
    // first delete internal content
    clean_server_object(targetP);
    // then restore previous object
    copy_server_object(targetP, backupObjectArray[1]);

    // restart the old servers
    IotLogDebug("[BOOTSTRAP] ObjectList restored\r\n");
}

void update_bootstrap_info(lwm2m_client_state_t * previousBootstrapState,
        lwm2m_context_t * context)
{
    if (*previousBootstrapState != context->state)
    {
        *previousBootstrapState = context->state;
        switch(context->state)
        {
            case STATE_BOOTSTRAPPING:
#ifdef LWM2M_WITH_LOGS
                IotLogInfo( "[BOOTSTRAP] backup security and server objects\r\n");
#endif
                prv_backup_objects(context);
                break;
            default:
                break;
        }
    }
}

static void close_backup_object()
{
    int i;
    for (i = 0; i < BACKUP_OBJECT_COUNT; i++) {
        if (NULL != backupObjectArray[i]) {
            switch (backupObjectArray[i]->objID)
            {
            case LWM2M_SECURITY_OBJECT_ID:
                clean_security_object(backupObjectArray[i]);
                lwm2m_free(backupObjectArray[i]);
                break;
            case LWM2M_SERVER_OBJECT_ID:
                clean_server_object(backupObjectArray[i]);
                lwm2m_free(backupObjectArray[i]);
                break;
            default:
                break;
            }
        }
    }
}
#endif

static void prv_display_objects(lwm2m_context_t *lwm2mH, char *buffer, void *user_data) {
    lwm2m_object_t *object;

    /* unused parameter */
    (void)user_data;

    for (object = lwm2mH->objectList; object != NULL; object = object->next) {
        if (NULL != object) {
            switch (object->objID) {
            case LWM2M_SECURITY_OBJECT_ID:
                display_security_object(object);
                break;
            case LWM2M_SERVER_OBJECT_ID:
                display_server_object(object);
                break;
            case LWM2M_ACL_OBJECT_ID:
                break;
            case LWM2M_DEVICE_OBJECT_ID:
                display_device_object(object);
                break;
            case LWM2M_CONN_MONITOR_OBJECT_ID:
                break;
            case LWM2M_FIRMWARE_UPDATE_OBJECT_ID:
                display_firmware_object(object);
                break;
            case LWM2M_LOCATION_OBJECT_ID:
                display_location_object(object);
                break;
            case LWM2M_CONN_STATS_OBJECT_ID:
                break;
            case TEST_OBJECT_ID:
                display_test_object(object);
                break;
            }
        }
    }
}
#endif
