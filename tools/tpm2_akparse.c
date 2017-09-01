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
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <getopt.h>
#include <sapi/tpm20.h>

#include "tpm2_options.h"
#include "files.h"
#include "log.h"
#include "tpm2_tool.h"
#include "tpm2_util.h"

typedef struct tpm_akparse_ctx tpm_akparse_ctx;
struct tpm_akparse_ctx {
    char *ak_data_file_path;
    char *ak_key_file_path;
};

static bool save_alg_and_key_to_file(const char *key_file, UINT16 alg_type,
        TPM2B **key_data, size_t key_data_len) {

    FILE *f = fopen(key_file, "wb+");
    if (!f) {
        LOG_ERR("Could not open file \"%s\" due to error: \"%s\".", key_file,
                strerror(errno));
        return false;
    }

    bool res = files_write_16(f, alg_type);
    if (!res) {
        /* write_be_convert prints error */
        goto out;
    }

    /*
     * For each TPM2B buffer in the list, save its size in be format, write it
     * to the output file and output it to stdout.
     */
    unsigned i;
    for (i=0; i < key_data_len; i++) {
        TPM2B *tmp = key_data[i];

        res = files_write_16(f, tmp->size);
        if (!res) {
            /* write_be_convert prints error */
            goto out;
        }

        size_t count = fwrite(tmp->buffer, sizeof(BYTE), tmp->size, f);
        if (count != tmp->size) {
            if (ferror(f)) {
                LOG_ERR("Error writing to file \"%s\", error: \"%s\"", key_file,
                        strerror(errno));
            } else {
                LOG_ERR("Did not write all bytes to file, got %zu expected %u",
                        count, tmp->size);
            }
            goto out;
        }

        tpm2_util_print_tpm2b(tmp);
    }
out:
    fclose(f);

    return true;
}

static bool parse_and_save_ak_public(tpm_akparse_ctx *ctx) {

    TPM2B_PUBLIC outPublic;
    UINT16 size = sizeof(outPublic);

    bool result = files_load_bytes_from_path(ctx->ak_data_file_path, (UINT8 *)&outPublic, &size);
    if (!result) {
        /* loadDataFromFile prints error */
        return false;
    }

    size_t key_data_len = 1;
    TPM2B *key_data[2];
    switch (outPublic.t.publicArea.type) {
    case TPM_ALG_RSA:
        key_data[0] = &outPublic.t.publicArea.unique.rsa.b;
        break;
    case TPM_ALG_KEYEDHASH:
        key_data[0] = &outPublic.t.publicArea.unique.keyedHash.b;
        break;
    case TPM_ALG_SYMCIPHER:
        key_data[0] = &outPublic.t.publicArea.unique.sym.b;
        break;
    case TPM_ALG_ECC:
        key_data_len = 2;
        key_data[0] = &outPublic.t.publicArea.unique.ecc.x.b;
        key_data[1] = &outPublic.t.publicArea.unique.ecc.y.b;
        break;
    default:
        LOG_ERR("The algorithm type(0x%4.4x) is not supported",
                outPublic.t.publicArea.type);
        return false;
    }

    return save_alg_and_key_to_file(ctx->ak_key_file_path, outPublic.t.publicArea.type,
            key_data, key_data_len);
}

static bool init(int argc, char *argv[], tpm_akparse_ctx *ctx) {

    struct option options[] = {
            { "file",    required_argument, NULL, 'f' },
            { "keyFile", required_argument, NULL, 'k' },
            { NULL,      no_argument,       NULL, '\0' }
    };

    if (argc <=1 || argc > (int) (2 * sizeof(options) / sizeof(struct option))) {
        showArgMismatch(argv[0]);
        return false;
    }

    int opt;
    while ((opt = getopt_long(argc, argv, "f:k:hv", options, NULL)) != -1) {
        switch (opt) {
        case 'f':
            if (!optarg) {
                LOG_ERR("Please input the file that used to be parsed.");
                return false;
            }
            ctx->ak_data_file_path = optarg;
            break;
        case 'k':
            if (!optarg) {
                LOG_ERR("Please input the file that used to save ak key.");
                return false;
            }
            ctx->ak_key_file_path = optarg;
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
    return true;
}

int execute_tool(int argc, char *argv[], char *envp[], common_opts_t *opts,
        TSS2_SYS_CONTEXT *sapi_context) {

    /* opts is unused, avoid compiler warning */
    (void)opts;
    (void)sapi_context;
    (void)envp;

    struct tpm_akparse_ctx ctx;

    bool result = init(argc, argv, &ctx);
    if (!result) {
        return 1;
    }

    /* 0 on success 1 on error */
    return parse_and_save_ak_public(&ctx) != true;
}
