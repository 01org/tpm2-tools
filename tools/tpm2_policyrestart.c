/* SPDX-License-Identifier: BSD-3-Clause */
//**********************************************************************;
// Copyright (c) 2018, Intel Corporation
// All rights reserved.
//
//**********************************************************************;

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

#include <tss2/tss2_esys.h>

#include "files.h"
#include "log.h"
#include "pcr.h"
#include "tpm2_options.h"
#include "tpm2_policy.h"
#include "tpm2_session.h"
#include "tpm2_tool.h"
#include "tpm2_util.h"

typedef struct tpm2_policyreset_ctx tpm2_policyreset_ctx;
struct tpm2_policyreset_ctx {
   struct {
       char *path;
   } session;
};

static tpm2_policyreset_ctx ctx;

static bool on_option(char key, char *value) {

    switch (key) {
    case 'S':
        ctx.session.path = value;
    break;
    }
    return true;
}

bool tpm2_tool_onstart(tpm2_options **opts) {

    static struct option topts[] = {
        { "session",        required_argument,  NULL,   'S' },
    };

    *opts = tpm2_options_new("S:", ARRAY_LEN(topts), topts, on_option,
                             NULL, 0);

    return *opts != NULL;
}

int tpm2_tool_onrun(ESYS_CONTEXT *ectx, tpm2_option_flags flags) {

    UNUSED(flags);

    int rc = 1;
    bool result;

    tpm2_session *s = tpm2_session_restore(ectx, ctx.session.path, false);
    if (!s) {
        return rc;
    }

    result = tpm2_session_restart(ectx, s);
    if (!result) {
        goto out;
    }

    rc = 0;

out:
    result = tpm2_session_close(&s);
    if (!result) {
        rc = 1;
    }

    return rc;
}
