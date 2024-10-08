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
 *    Simon Bernard - initial API and implementation
 *    Christian Renz - Please refer to git log
 *
 *******************************************************************************/

#ifndef DTLS_CONNECTION_H_
#define DTLS_CONNECTION_H_

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdbool.h>
#include "liblwm2m.h"

#define LWM2M_STANDARD_PORT_STR "5683"
#define LWM2M_STANDARD_PORT      5683
#define LWM2M_DTLS_PORT_STR     "5684"
#define LWM2M_DTLS_PORT          5684
#define LWM2M_BSSERVER_PORT_STR "5685"
#define LWM2M_BSSERVER_PORT      5685

// after 40sec of inactivity we rehandshake
#define DTLS_NAT_TIMEOUT 40

char * security_get_uri(lwm2m_context_t * lwm2mH, lwm2m_object_t * obj, int instanceId, char * uriBuffer, int bufferSize);

int64_t security_get_mode(lwm2m_context_t * lwm2mH, lwm2m_object_t * obj, int instanceId);

char * security_get_public_id(lwm2m_context_t * lwm2mH, lwm2m_object_t * obj, int instanceId, int * length);

char * security_get_secret_key(lwm2m_context_t * lwm2mH, lwm2m_object_t * obj, int instanceId, int * length);
#endif
