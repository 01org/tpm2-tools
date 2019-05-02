/* SPDX-License-Identifier: BSD-3-Clause */
//**********************************************************************;
// Copyright (c) 2017, Intel Corporation
// All rights reserved.
//
//**********************************************************************;

#include <stdlib.h>

#include <tss2/tss2_esys.h>

#include "log.h"
#include "tpm2_alg_util.h"
#include "tpm2_options.h"
#include "tpm2_tool.h"
#include "tpm2_util.h"

typedef struct tpm_pcr_extend_ctx tpm_pcr_extend_ctx;
struct tpm_pcr_extend_ctx {
    size_t digest_spec_len;
    tpm2_pcr_digest_spec *digest_spec;
};

static tpm_pcr_extend_ctx ctx;

static bool pcr_extend_one(ESYS_CONTEXT *ectx,
        TPMI_DH_PCR pcr_index, TPML_DIGEST_VALUES *digests) {;

    TSS2_RC rval = Esys_PCR_Extend(ectx, pcr_index,
                            ESYS_TR_PASSWORD, ESYS_TR_NONE, ESYS_TR_NONE,
                            digests);
    if (rval != TSS2_RC_SUCCESS) {
        LOG_ERR("Could not extend pcr index: 0x%X", pcr_index);
        LOG_PERR(Esys_PCR_Extend, rval);
        return false;
    }

    return true;
}

static bool pcr_extend(ESYS_CONTEXT *ectx) {

    size_t i;
    for (i = 0; i < ctx.digest_spec_len; i++) {
        tpm2_pcr_digest_spec *dspec = &ctx.digest_spec[i];
        bool result = pcr_extend_one(ectx, dspec->pcr_index,
                &dspec->digests);
        if (!result) {
            return false;
        }
    }

    return true;
}

static bool on_arg(int argc, char **argv) {

    if (argc < 1) {
        LOG_ERR("Expected at least one PCR Digest specification,"
                "ie: <pcr index>:<hash alg>=<hash value>, got: 0");
        return false;
    }

    /* this can never be negative */
    ctx.digest_spec_len = (size_t) argc;

    ctx.digest_spec = calloc(ctx.digest_spec_len, sizeof(*ctx.digest_spec));
    if (!ctx.digest_spec) {
        LOG_ERR("oom");
        return false;
    }

    return pcr_parse_digest_list(argv, ctx.digest_spec_len, ctx.digest_spec);
}

bool tpm2_tool_onstart(tpm2_options **opts) {

    *opts = tpm2_options_new(NULL, 0, NULL, NULL, on_arg, 0);

    return *opts != NULL;
}

int tpm2_tool_onrun(ESYS_CONTEXT *ectx, tpm2_option_flags flags) {

    UNUSED(flags);

    return pcr_extend(ectx) != true;
}

void tpm2_tool_onexit(void) {

    free(ctx.digest_spec);
}
