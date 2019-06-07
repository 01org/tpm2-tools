/* SPDX-License-Identifier: BSD-3-Clause */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <tss2/tss2_esys.h>

#include "files.h"
#include "log.h"
#include "object.h"
#include "pcr.h"
#include "tpm2_auth_util.h"
#include "tpm2_hash.h"
#include "tpm2_policy.h"
#include "tpm2_tool.h"
#include "tpm2_session.h"
#include "tpm2_util.h"

typedef struct tpm_unseal_ctx tpm_unseal_ctx;
struct tpm_unseal_ctx {
    const char *auth_str;
    tpm2_session *session;
    char *outFilePath;
    const char *context_arg;
    tpm2_loaded_object context_object;
};

static tpm_unseal_ctx ctx;

tool_rc unseal_and_save(ESYS_CONTEXT *ectx) {

    TPM2B_SENSITIVE_DATA *outData = NULL;

    ESYS_TR shandle1 = ESYS_TR_NONE;
    tool_rc rc = tpm2_auth_util_get_shandle(ectx,
                            ctx.context_object.tr_handle,
                            ctx.session, &shandle1);
    if (rc != tool_rc_success) {
        return rc;
    }

    TSS2_RC rval = Esys_Unseal(ectx, ctx.context_object.tr_handle,
                shandle1, ESYS_TR_NONE, ESYS_TR_NONE,
                &outData);
    if (rval != TPM2_RC_SUCCESS) {
        LOG_PERR(Esys_Unseal, rval);
        rc = tool_rc_from_tpm(rval);
        goto out;
    }

    if (ctx.outFilePath) {
        bool ret = files_save_bytes_to_file(ctx.outFilePath, (UINT8 *)
                                        outData->buffer, outData->size);
        if (!ret) {
            rc = tool_rc_general_error;
            goto out;
        }
    } else {
        bool ret = files_write_bytes(stdout, (UINT8 *) outData->buffer,
                                 outData->size);
        if (!ret) {
            rc = tool_rc_general_error;
            goto out;
        }
    }

    rc = tool_rc_success;

out:
    free(outData);

    return rc;
}

static tool_rc init(ESYS_CONTEXT *ectx) {

    if (!ctx.context_arg) {
        LOG_ERR("Expected option c");
        return tool_rc_option_error;
    }

    tool_rc rc = tpm2_auth_util_from_optarg(ectx, ctx.auth_str,
            &ctx.session, false);
    if (rc != tool_rc_success) {
        LOG_ERR("Invalid item handle authorization, got\"%s\"", ctx.auth_str);
        return rc;
    }

    return tpm2_util_object_load(ectx, ctx.context_arg,
            &ctx.context_object);
}

static bool on_option(char key, char *value) {

    switch (key) {
    case 'c':
        ctx.context_arg = value;
        break;
    case 'p': {
        ctx.auth_str = value;
    }
        break;
    case 'o':
        ctx.outFilePath = value;
        break;
        /* no default */
    }

    return true;
}

bool tpm2_tool_onstart(tpm2_options **opts) {

    static const struct option topts[] = {
      { "auth-key",             required_argument, NULL, 'p' },
      { "out-file",             required_argument, NULL, 'o' },
      { "context-object",       required_argument, NULL, 'c' },
    };

    *opts = tpm2_options_new("p:o:c:", ARRAY_LEN(topts), topts,
                             on_option, NULL, 0);

    return *opts != NULL;
}

tool_rc tpm2_tool_onrun(ESYS_CONTEXT *ectx, tpm2_option_flags flags) {

    UNUSED(flags);

    tool_rc rc = init(ectx);
    if (rc != tool_rc_success) {
        return rc;
    }

    return unseal_and_save(ectx);
}

tool_rc tpm2_tool_onstop(ESYS_CONTEXT *ectx) {
    UNUSED(ectx);
    return tpm2_session_close(&ctx.session);
}
