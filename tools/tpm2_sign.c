//**********************************************************************;
// Copyright (c) 2015-2018, Intel Corporation
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

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <getopt.h>
#include <tss2/tss2_esys.h>

#include "files.h"
#include "log.h"
#include "tpm2_auth_util.h"
#include "tpm2_convert.h"
#include "tpm2_hash.h"
#include "tpm2_alg_util.h"
#include "tpm2_options.h"
#include "tpm2_session.h"
#include "tpm2_tool.h"
#include "tpm2_util.h"

typedef struct tpm_sign_ctx tpm_sign_ctx;
struct tpm_sign_ctx {
    TPMT_TK_HASHCHECK validation;
    struct {
        TPMS_AUTH_COMMAND session_data;
        tpm2_session *session;
    } auth;
    TPMI_ALG_HASH halg;
    TPMI_ALG_SIG_SCHEME sig_scheme;
    TPMT_SIG_SCHEME in_scheme;
    TPM2B_DIGEST *digest;
    char *outFilePath;
    BYTE *msg;
    UINT16 length;
    const char *context_arg;
    tpm2_loaded_object key_context;
    char *inMsgFileName;
    tpm2_convert_sig_fmt sig_format;
    struct {
        UINT16 p : 1;
        UINT16 g : 1;
        UINT16 s : 1;
        UINT16 m : 1;
        UINT16 t : 1;
        UINT16 o : 1;
        UINT16 f : 1;
        UINT16 D : 1;
    } flags;
    char *key_auth_str;
};

static tpm_sign_ctx ctx = {
        .auth = { .session_data = TPMS_AUTH_COMMAND_INIT(TPM2_RS_PW) },
        .halg = TPM2_ALG_SHA1,
        .digest = NULL,
        .sig_scheme = TPM2_ALG_NULL
};

static bool sign_and_save(ESYS_CONTEXT *ectx) {

    TPMT_SIGNATURE *signature;
    bool result;

    if (!ctx.flags.D) {
      bool res = tpm2_hash_compute_data(ectx, ctx.halg, TPM2_RH_NULL,
              ctx.msg, ctx.length, &ctx.digest, NULL);
      if (!res) {
          LOG_ERR("Compute message hash failed!");
          return false;
      }
    }

    ESYS_TR shandle1 = tpm2_auth_util_get_shandle(ectx,
                            ctx.key_context.tr_handle, &ctx.auth.session_data,
                            ctx.auth.session);
    if (shandle1 == ESYS_TR_NONE) {
        return false;
    }

    TSS2_RC rval = Esys_Sign(ectx, ctx.key_context.tr_handle,
                    shandle1, ESYS_TR_NONE, ESYS_TR_NONE,
                    ctx.digest, &ctx.in_scheme, &ctx.validation, &signature);
    if (rval != TPM2_RC_SUCCESS) {
        LOG_PERR(Eys_Sign, rval);
        result = false;
    } else {
        result = tpm2_convert_sig_save(signature, ctx.sig_format,
                    ctx.outFilePath);
    }

    free(signature);

    return result;
}

static bool init(ESYS_CONTEXT *ectx) {

    bool option_fail = false;

    if (!ctx.context_arg) {
        LOG_ERR("Expected option c");
        option_fail = true;
    }

    if (!ctx.flags.m && !ctx.flags.D) {
        LOG_ERR("Expected options m or D");
        option_fail = true;
    }

    if (!ctx.flags.o) {
        LOG_ERR("Expected option o");
        option_fail = true;
    }

    if (option_fail) {
        return false;
    }

    if (ctx.flags.D && (ctx.flags.t || ctx.flags.m)) {
        LOG_WARN("Option D provided, options m and t are ignored.");
    }

    if (ctx.flags.D || !ctx.flags.t) {
        ctx.validation.tag = TPM2_ST_HASHCHECK;
        ctx.validation.hierarchy = TPM2_RH_NULL;
        memset(&ctx.validation.digest, 0, sizeof(ctx.validation.digest));
    }

    /*
     * load tpm context from a file if -c is provided
     */
    bool result = tpm2_util_object_load(ectx, ctx.context_arg,
                                &ctx.key_context);
    if (!result) {
        return false;
    }

    /*
     * Set signature scheme for key type, or validate chosen scheme is allowed for key type.
     */
    result = get_signature_scheme(ectx, ctx.key_context.tr_handle,
                    ctx.halg, ctx.sig_scheme, &ctx.in_scheme);
    if (!result) {
        LOG_ERR("bad signature scheme for key type!");
        return false;
    }

    /*
     * Process the msg file if needed
     */
    if (ctx.flags.m && !ctx.flags.D) {
      unsigned long file_size;
      result = files_get_file_size_path(ctx.inMsgFileName, &file_size);
      if (!result) {
          return false;
      }
      if (file_size == 0) {
          LOG_ERR("The message file \"%s\" is empty!", ctx.inMsgFileName);
          return false;
      }

      if (file_size > UINT16_MAX) {
          LOG_ERR(
                  "The message file \"%s\" is too large, got: %lu bytes, expected less than: %u bytes!",
                  ctx.inMsgFileName, file_size, UINT16_MAX + 1);
          return false;
      }

      ctx.msg = (BYTE*) calloc(required_argument, file_size);
      if (!ctx.msg) {
          LOG_ERR("oom");
          return false;
      }

      ctx.length = file_size;
      result = files_load_bytes_from_path(ctx.inMsgFileName, ctx.msg, &ctx.length);
      if (!result) {
          free(ctx.msg);
          return false;
      }
    }

    return true;
}

static bool on_option(char key, char *value) {

    switch (key) {
    case 'c':
        ctx.context_arg = value;
        break;
    case 'p':
        ctx.flags.p = 1;
        ctx.key_auth_str = value;
        break;
    case 'g': {
        ctx.halg = tpm2_alg_util_from_optarg(value, tpm2_alg_util_flags_hash);
        if (ctx.halg == TPM2_ALG_ERROR) {
            LOG_ERR("Could not convert to number or lookup algorithm, got: \"%s\"",
                    value);
            return false;
        }
        ctx.flags.g = 1;
    }
        break;
    case 's': {
        ctx.sig_scheme = tpm2_alg_util_from_optarg(value, tpm2_alg_util_flags_sig);
        if (ctx.sig_scheme == TPM2_ALG_ERROR) {
            LOG_ERR("Unknown signing scheme, got: \"%s\"", value);
            return false;
        }
        ctx.flags.s = 1;
    }
        break;
    case 'D': {
        ctx.digest = malloc(sizeof(TPM2B_DIGEST));
        ctx.digest->size = sizeof(TPM2B_DIGEST);
        if (!files_load_bytes_from_path(value, ctx.digest->buffer, &ctx.digest->size)) {
            LOG_ERR("Could not load digest from file \"%s\"!", value);
            return false;
        }
        ctx.flags.D = 1;
    }
        break;
    case 'm':
        ctx.inMsgFileName = value;
        ctx.flags.m = 1;
        break;
    case 't': {
        bool result = files_load_validation(value, &ctx.validation);
        if (!result) {
            return false;
        }
        ctx.flags.t = 1;
    }
        break;
    case 'o': {
        ctx.outFilePath = value;
        ctx.flags.o = 1;
    }
        break;
    case 'f':
        ctx.flags.f = 1;
        ctx.sig_format = tpm2_convert_sig_fmt_from_optarg(value);

        if (ctx.sig_format == signature_format_err) {
            return false;
        }
    /* no default */
    }

    return true;
}

bool tpm2_tool_onstart(tpm2_options **opts) {

    static const struct option topts[] = {
      { "auth-key",             required_argument, NULL, 'p' },
      { "halg",                 required_argument, NULL, 'g' },
      { "sig-scheme",           required_argument, NULL, 's' },
      { "message",              required_argument, NULL, 'm' },
      { "digest",               required_argument, NULL, 'D' },
      { "out-sig",              required_argument, NULL, 'o' },
      { "ticket",               required_argument, NULL, 't' },
      { "key-context",          required_argument, NULL, 'c' },
      { "format",               required_argument, NULL, 'f' }
    };

    *opts = tpm2_options_new("p:g:m:D:t:o:c:f:s:", ARRAY_LEN(topts), topts,
                             on_option, NULL, 0);

    return *opts != NULL;
}

int tpm2_tool_onrun(ESYS_CONTEXT *ectx, tpm2_option_flags flags) {

    UNUSED(flags);

    int rc = 1;
    bool result = init(ectx);
    if (!result) {
        goto out;
    }

    if (ctx.flags.p) {
        result = tpm2_auth_util_from_optarg(ectx, ctx.key_auth_str,
                &ctx.auth.session_data, &ctx.auth.session);
        if (!result) {
            LOG_ERR("Invalid key authorization, got\"%s\"", ctx.key_auth_str);
            goto out;
        }
    }

    result = sign_and_save(ectx);
    if (!result) {
        goto out;
    }

    rc = 0;
out:

    result = tpm2_session_save(ectx, ctx.auth.session, NULL);
    if (!result) {
        rc = 1;
    }

    tpm2_session_free(&ctx.auth.session);

    return rc;
}

void tpm2_tool_onexit(void) {

    if (ctx.digest) {
        free(ctx.digest);
    }
    free(ctx.msg);
}
