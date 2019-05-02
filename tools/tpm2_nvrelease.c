/* SPDX-License-Identifier: BSD-3-Clause */
//**********************************************************************;
// Copyright (c) 2015-2018, Intel Corporation
// All rights reserved.
//
//**********************************************************************;

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <tss2/tss2_esys.h>

#include "log.h"
#include "tpm2_auth_util.h"
#include "tpm2_hierarchy.h"
#include "tpm2_options.h"
#include "tpm2_session.h"
#include "tpm2_tool.h"
#include "tpm2_util.h"

typedef struct tpm_nvrelease_ctx tpm_nvrelease_ctx;
struct tpm_nvrelease_ctx {
    UINT32 nv_index;
    struct {
        TPMS_AUTH_COMMAND session_data;
        tpm2_session *session;
        TPMI_RH_PROVISION hierarchy;
    } auth;
    struct {
        UINT8 P : 1;
        UINT8 unused : 7;
    } flags;
    char *passwd_auth_str;
};

static tpm_nvrelease_ctx ctx = {
    .auth = { .session_data = TPMS_AUTH_COMMAND_INIT(TPM2_RS_PW),
              .hierarchy = TPM2_RH_OWNER },
};

static bool nv_space_release(ESYS_CONTEXT *ectx) {

    ESYS_TR nv_handle;
    TSS2_RC rval = Esys_TR_FromTPMPublic(ectx, ctx.nv_index,
                        ESYS_TR_NONE, ESYS_TR_NONE, ESYS_TR_NONE,
                        &nv_handle);
    if (rval != TPM2_RC_SUCCESS) {
        LOG_PERR(Esys_TR_FromTPMPublic, rval);
        return false;
    }

    ESYS_TR hierarchy = tpm2_tpmi_hierarchy_to_esys_tr(ctx.auth.hierarchy);
    ESYS_TR shandle1 = tpm2_auth_util_get_shandle(ectx, hierarchy,
                            &ctx.auth.session_data, ctx.auth.session);
    if (shandle1 == ESYS_TR_NONE) {
        LOG_ERR("Couldn't get shandle");
        return false;
    }

    rval = Esys_NV_UndefineSpace(ectx, hierarchy, nv_handle,
                shandle1, ESYS_TR_NONE, ESYS_TR_NONE);
    if (rval != TPM2_RC_SUCCESS) {
        LOG_ERR("Failed to release NV area at index 0x%X", ctx.nv_index);
        LOG_PERR(Esys_NV_UndefineSpace, rval);
        return false;
    }

    LOG_INFO("Success to release NV area at index 0x%x (%d).", ctx.nv_index,
            ctx.nv_index);

    return true;
}

static bool on_option(char key, char *value) {

    bool result;

    switch (key) {
    case 'x':
        result = tpm2_util_string_to_uint32(value, &ctx.nv_index);
        if (!result) {
            LOG_ERR("Could not convert NV index to number, got: \"%s\"",
                    value);
            return false;
        }

        if (ctx.nv_index == 0) {
            LOG_ERR("NV Index cannot be 0");
            return false;
        }
        break;
    case 'a':
        result = tpm2_hierarchy_from_optarg(value, &ctx.auth.hierarchy,
                TPM2_HIERARCHY_FLAGS_O|TPM2_HIERARCHY_FLAGS_P);
        if (!result) {
            return false;
        }

        break;
    case 'P':
        ctx.flags.P = 1;
        ctx.passwd_auth_str = value;
        break;
    }

    return true;
}

bool tpm2_tool_onstart(tpm2_options **opts) {

    const struct option topts[] = {
        { "index",          required_argument, NULL, 'x' },
        { "hierarchy",      required_argument, NULL, 'a' },
        { "auth-hierarchy", required_argument, NULL, 'P' },
    };

    *opts = tpm2_options_new("x:a:P:", ARRAY_LEN(topts), topts, on_option,
                             NULL, 0);

    return *opts != NULL;
}

int tpm2_tool_onrun(ESYS_CONTEXT *ectx, tpm2_option_flags flags) {

    UNUSED(flags);

    int rc = 1;
    bool result;

    if (ctx.flags.P) {
        result = tpm2_auth_util_from_optarg(ectx, ctx.passwd_auth_str,
                &ctx.auth.session_data, &ctx.auth.session);
        if (!result) {
            LOG_ERR("Invalid handle authorization, got \"%s\"",
                ctx.passwd_auth_str);
            goto out;
        }
    }

    result = nv_space_release(ectx);
    if (!result) {
        goto out;
    }

    rc = 0;

out:
    result = tpm2_session_save(ectx, ctx.auth.session, NULL);
    if (!result) {
        rc = 1;
    }

    return rc;
}

void tpm2_onexit(void) {

    tpm2_session_free(&ctx.auth.session);
}
