//**********************************************************************;
// Copyright (c) 2015, Intel Corporation
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// 3. Neither the name of Intel Corporation nor the names of its contributors
// may be used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//**********************************************************************;

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <getopt.h>

#include <sapi/tpm20.h>

#include "files.h"
#include "log.h"
#include "main.h"
#include "options.h"
#include "tpm_hash.h"
#include "tpm2_alg_util.h"
#include "tpm2_util.h"

typedef struct tpm_hash_ctx tpm_hash_ctx;
struct tpm_hash_ctx {
    TPMI_RH_HIERARCHY hierarchyValue;
    FILE *input_file;
    TPMI_ALG_HASH  halg;
    char *outHashFilePath;
    char *outTicketFilePath;
    TSS2_SYS_CONTEXT *sapi_context;
};

static bool get_hierarchy_value(const char *hiearchy_code,
        TPMI_RH_HIERARCHY *hierarchy_value) {

    size_t len = strlen(hiearchy_code);
    if (len != 1) {
        LOG_ERR("Hierarchy Values are single characters, got: %s",
                hiearchy_code);
        return false;
    }

    switch (hiearchy_code[0]) {
    case 'e':
        *hierarchy_value = TPM_RH_ENDORSEMENT;
        break;
    case 'o':
        *hierarchy_value = TPM_RH_OWNER;
        break;
    case 'p':
        *hierarchy_value = TPM_RH_PLATFORM;
        break;
    case 'n':
        *hierarchy_value = TPM_RH_NULL;
        break;
    default:
        LOG_ERR("Unknown hierarchy value: %s", hiearchy_code);
        return false;
    }
    return true;
}

static bool hash_and_save(tpm_hash_ctx *ctx) {

    TPM2B_DIGEST outHash = TPM2B_TYPE_INIT(TPM2B_DIGEST, buffer);
    TPMT_TK_HASHCHECK validation;

    TPM_RC rval = tpm_hash_file(ctx->sapi_context, ctx->halg, ctx->hierarchyValue, ctx->input_file, &outHash, &validation);
    if (rval != TPM_RC_SUCCESS) {
        LOG_ERR("tpm_hash_files() failed with error: 0x%X", rval);
        return false;
    }

    if (outHash.t.size) {
        UINT16 i;
        TOOL_OUTPUT("hash(%s):", tpm2_alg_util_algtostr(ctx->halg));
        for (i = 0; i < outHash.t.size; i++) {
            TOOL_OUTPUT("%02x", outHash.t.buffer[i]);
        }
        TOOL_OUTPUT("\n");
    }

    if (validation.digest.t.size) {
        UINT16 i;
        TOOL_OUTPUT("ticket:");
        for (i = 0; i < validation.digest.t.size; i++) {
            TOOL_OUTPUT("%02x", validation.digest.t.buffer[i]);
        }
        TOOL_OUTPUT("\n");
    }

    if (ctx->outHashFilePath) {
        bool result = files_save_bytes_to_file(ctx->outHashFilePath, (UINT8 *) &outHash,
                sizeof(outHash));
        if (!result) {
            return false;
        }
    }

    if (ctx->outTicketFilePath) {
        return files_save_bytes_to_file(ctx->outTicketFilePath, (UINT8 *) &validation,
                sizeof(validation));
    }

    return true;
}

static bool init(int argc, char *argv[], tpm_hash_ctx *ctx) {

    static struct option long_options[] = {
        {"hierachy", required_argument, NULL, 'H'},
        {"halg",     required_argument, NULL, 'g'},
        {"outfile",  required_argument, NULL, 'o'},
        {"ticket",   required_argument, NULL, 't'},
        {NULL,       no_argument,       NULL, '\0'}
    };

    int opt;
    bool res;
    while ((opt = getopt_long(argc, argv, "H:g:o:t:", long_options, NULL))
            != -1) {
        switch (opt) {
        case 'H':
            res = get_hierarchy_value(optarg, &ctx->hierarchyValue);
            if (!res) {
                return false;
            }
            break;
        case 'g':
            ctx->halg = tpm2_alg_util_from_optarg(optarg);
            if (ctx->halg == TPM_ALG_ERROR) {
                showArgError(optarg, argv[0]);
                return false;
            }
            break;
        case 'o':
            ctx->outHashFilePath = optarg;
            break;
        case 't':
            ctx->outTicketFilePath = optarg;
            break;
        case ':':
            LOG_ERR("Argument %c needs a value!", optopt);
            return false;
        case '?':
            LOG_ERR("Unknown Argument: %c", optopt);
            return false;
        default:
            LOG_ERR("?? getopt returned character code 0%o ??", opt);
            return false;
        }
    }

    int cnt = argc - optind;
    if (cnt == 0) {
        return true;
    }

    if (cnt > 1) {
        LOG_ERR("Only supports one hash input file, got: %d", cnt);
        return false;
    }

    ctx->input_file = fopen(argv[optind], "rb");
    if (!ctx->input_file) {
        LOG_ERR("Could not open input file \"%s\", error: %s",
                argv[optind], strerror(errno));
        return false;
    }

    return true;
}

int execute_tool(int argc, char *argv[],  common_opts_t *opts,
        TSS2_SYS_CONTEXT *sapi_context) {

    /* opts is unused, avoid compiler warning */
    UNUSED(opts);

    int rc = 1;
    tpm_hash_ctx ctx = {
            .outHashFilePath = NULL,
            .outTicketFilePath = NULL,
            .input_file = stdin,
            .hierarchyValue = TPM_RH_NULL,
            .halg = TPM_ALG_SHA1,
            .sapi_context = sapi_context,
    };

    bool res = init(argc, argv, &ctx);
    if (!res) {
        goto out;
    }

    res = hash_and_save(&ctx);
    if (!res) {
        goto out;
    }

    rc = 0;

out:
    if (ctx.input_file) {
        fclose(ctx.input_file);
    }

    return rc;
}
