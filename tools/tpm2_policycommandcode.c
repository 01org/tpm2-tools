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

#include <tss2/tss2_sys.h>

#include "files.h"
#include "log.h"
#include "pcr.h"
#include "tpm2_options.h"
#include "tpm2_policy.h"
#include "tpm2_session.h"
#include "tpm2_tool.h"
#include "tpm2_util.h"

typedef struct tpm2_policycommandcode_ctx tpm2_policycommandcode_ctx;
struct tpm2_policycommandcode_ctx {
   const char *session_path;
   TPM2_CC command_code;
   const char *out_policy_dgst_path;
};

static tpm2_policycommandcode_ctx ctx;

static bool on_option(char key, char *value) {

    switch (key) {
    case 'S':
        ctx.session_path = value;
        break;
    case 'o':
        ctx.out_policy_dgst_path = value;
        break;
    }
    return true;
}

bool is_input_option_args_valid(void) {

    if (!ctx.session_path) {
        LOG_ERR("Must specify -S session file.");
        return false;
    }

    return true;
}

bool on_arg (int argc, char **argv) {

    if (argc > 1) {
        LOG_ERR("Specify only the TPM2 command code.");
        return false;
    }

    if (!argc) {
        LOG_ERR("TPM2 command code must be specified.");
        return false;
    }

    bool result = tpm2_util_string_to_uint32(argv[0], &ctx.command_code);
    if (!result) {
        LOG_ERR("Could not convert command-code to number, got: \"%s\"",
                argv[0]);
        return false;
    }

    return true;
}

bool tpm2_tool_onstart(tpm2_options **opts) {

    static struct option topts[] = {
        { "session",        required_argument,  NULL,   'S' },
        { "out-policy-file",    required_argument,  NULL,   'o' },
    };

    *opts = tpm2_options_new("S:o:", ARRAY_LEN(topts), topts, on_option,
                             on_arg, 0);

    return *opts != NULL;
}

int tpm2_tool_onrun(ESYS_CONTEXT *ectx, tpm2_option_flags flags) {

    UNUSED(flags);

    TPM2B_DIGEST *policy_digest = NULL;
    bool retval = is_input_option_args_valid();
    if (!retval) {
        return -1;
    }

    int rc = 1;
    tpm2_session *s = tpm2_session_restore(ectx, ctx.session_path);
    if (!s) {
        return rc;
    }

    bool result = tpm2_policy_build_policycommandcode(ectx, s,
        ctx.command_code);
    if (!result) {
        LOG_ERR("Could not build TPM policy_command_code");
        goto out;
    }

    result = tpm2_policy_get_digest(ectx, s, &policy_digest);
    if (!result) {
        LOG_ERR("Could not build tpm policy");
        goto out;
    }

    tpm2_util_hexdump(policy_digest->buffer, policy_digest->size);
    tpm2_tool_output("\n");

    if (ctx.out_policy_dgst_path) {
        result = files_save_bytes_to_file(ctx.out_policy_dgst_path,
                    policy_digest->buffer, policy_digest->size);
        if (!result) {
            LOG_ERR("Failed to save policy digest into file \"%s\"",
                    ctx.out_policy_dgst_path);
            goto out;
        }
    }

    result = tpm2_session_save(ectx, s, ctx.session_path);
    if (!result) {
        LOG_ERR("Failed to save policy to file \"%s\"", ctx.session_path);
        goto out;
    }

    rc = 0;

out:
    free(policy_digest);
    tpm2_session_free(&s);
    return rc;
}
