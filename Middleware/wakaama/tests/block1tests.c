/*******************************************************************************
 *
 * Copyright (c) 2015 Bosch Software Innovations GmbH, Germany.
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
 *   Julien Vermillard - Please refer to git log
 *   Tuve Nordius, Husqvarna Group - Please refer to git log
 *
 *******************************************************************************/

#include "tests.h"
#include "CUnit/Basic.h"
#include "internals.h"
#include "liblwm2m.h"


static void handle_12345(lwm2m_block_data_t ** blk1,
                                  const char * uri) {
    uint8_t *buffer = (uint8_t *)"12345";
    size_t bsize;
    uint8_t *resultBuffer = NULL;

    uint8_t st = coap_block1_handler(blk1, uri, buffer, 5, 5, 0, true, &resultBuffer, &bsize);
    CU_ASSERT_EQUAL(st, COAP_231_CONTINUE)
    CU_ASSERT_PTR_NULL(resultBuffer)
}

static void handle_67(lwm2m_block_data_t ** blk1,
                                  const char * uri) {
    uint8_t *buffer = (uint8_t *)"67";
    size_t bsize;
    uint8_t *resultBuffer = NULL;

    uint8_t st = coap_block1_handler(blk1, uri, buffer, 2, 5, 1, false, &resultBuffer, &bsize);
    CU_ASSERT_EQUAL(st, NO_ERROR)
    CU_ASSERT_PTR_NOT_NULL(resultBuffer)
    CU_ASSERT_EQUAL(bsize, 7)
    CU_ASSERT_NSTRING_EQUAL(resultBuffer, "1234567", 7)
}


static void test_block1_nominal(void)
{
    lwm2m_block_data_t * blk1 = NULL;

    handle_12345(&blk1, "/1/2/3");
    handle_67(&blk1, "/1/2/3");

    free_block_data(blk1);
}

// This test needs rework...
/*
static void test_block1_retransmit(void)
{
    lwm2m_block_data_t * blk1 = NULL;

    handle_12345(&blk1, "/1/2/3/");
    handle_12345(&blk1, "/1/2/3/");
    handle_67(&blk1, "/1/2/3/");
    handle_67(&blk1, "/1/2/3/");
    handle_67(&blk1, "/1/2/3/");

    free_block_data(blk1);
}
*/

static struct TestTable table[] = {
        { "test of test_block1_nominal()", test_block1_nominal },
        //{ "test of test_block1_retransmit()", test_block1_retransmit },
        { NULL, NULL },
};

CU_ErrorCode create_block1_suit() {
    CU_pSuite pSuite = NULL;
    pSuite = CU_add_suite("Suite_block1", NULL, NULL);

    if (NULL == pSuite) {
        return CU_get_error();
    }
    return add_tests(pSuite, table);
}
