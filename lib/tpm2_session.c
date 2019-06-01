/* SPDX-License-Identifier: BSD-3-Clause */

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tss2/tss2_esys.h>

#include "files.h"
#include "log.h"
#include "tpm2.h"
#include "tpm2_kdfa.h"
#include "tpm2_alg_util.h"
#include "tpm2_session.h"
#include "tpm2_util.h"

struct tpm2_session_data {
    ESYS_TR key;
    ESYS_TR bind;
    TPM2_SE session_type;
    TPMT_SYM_DEF symmetric;
    TPMI_ALG_HASH authHash;
    TPM2B_NONCE nonce_caller;
    TPMA_SESSION attrs;
    TPM2B_AUTH auth_data;
    const char *path;
};

struct tpm2_session {

    tpm2_session_data* input;

    struct {
        ESYS_TR session_handle;
    } output;

    struct {
        char *path;
        ESYS_CONTEXT *ectx;
        bool is_final;
    } internal;
};

tpm2_session_data *tpm2_session_data_new(TPM2_SE type) {
    tpm2_session_data * d = calloc(1, sizeof(tpm2_session_data));
    if (d) {
        d->symmetric.algorithm = TPM2_ALG_NULL;
        d->key = ESYS_TR_NONE;
        d->bind = ESYS_TR_NONE;
        d->session_type = type;
        d->authHash = TPM2_ALG_SHA256;
    }
    return d;
}

void tpm2_session_set_key(tpm2_session_data *data, ESYS_TR key) {
    data->key = key;
}

void tpm2_session_set_attrs(tpm2_session_data *data, TPMA_SESSION attrs) {
    data->attrs = attrs;
}

void tpm2_session_set_auth_value(tpm2_session *session, TPM2B_AUTH *auth) {

    if (auth == NULL) {
        session->input->auth_data.size = 0;
        memset(session->input->auth_data.buffer, 0xBA,
                sizeof(session->input->auth_data.buffer));
    }
    else {
        memcpy(&session->input->auth_data, auth, sizeof(*auth));
    }
}

void tpm2_session_set_nonce_caller(tpm2_session_data *data, TPM2B_NONCE *nonce) {
    data->nonce_caller = *nonce;
}

void tpm2_session_set_bind(tpm2_session_data *data, ESYS_TR bind) {
    data->bind = bind;
}

void tpm2_session_set_type(tpm2_session_data *data, TPM2_SE type) {
    data->session_type = type;
}

void tpm2_session_set_symmetric(tpm2_session_data *data,
        TPMT_SYM_DEF *symmetric) {
    data->symmetric = *symmetric;
}

void tpm2_session_set_authhash(tpm2_session_data *data, TPMI_ALG_HASH auth_hash) {
    data->authHash = auth_hash;
}

void tpm2_session_set_path(tpm2_session_data *data, const char *path) {
    data->path = path;
}

TPMI_ALG_HASH tpm2_session_get_authhash(tpm2_session *session) {
    return session->input->authHash;
}

ESYS_TR tpm2_session_get_handle(tpm2_session *session) {
    return session->output.session_handle;
}

TPM2_SE tpm2_session_get_type(tpm2_session *session) {
    return session->input->session_type;
}

const TPM2B_AUTH *tpm2_session_get_auth_value(tpm2_session *session) {
    return &session->input->auth_data;
}

//
// This is a wrapper function around the Esys_StartAuthSession command.
// It performs the command, calculates the session key, and updates a
// SESSION structure.
//
static bool start_auth_session(tpm2_session *session) {

    tpm2_session_data *d = session->input;

    TPM2B_NONCE *nonce = session->input->nonce_caller.size > 0 ?
            &session->input->nonce_caller : NULL;

    TSS2_RC rval = Esys_StartAuthSession(session->internal.ectx, d->key, d->bind,
                        ESYS_TR_NONE, ESYS_TR_NONE, ESYS_TR_NONE,
                        nonce, d->session_type,
                        &d->symmetric, d->authHash,
                        &session->output.session_handle);
    if (rval != TPM2_RC_SUCCESS) {
        LOG_PERR(Esys_StartAuthSession, rval);
        return false;
    }

    if (d->attrs) {
        rval = Esys_TRSess_SetAttributes(session->internal.ectx, session->output.session_handle, d->attrs,
                                          0xff);
        if (rval != TSS2_RC_SUCCESS) {
            LOG_PERR(Esys_TRSess_SetAttributes, rval);
            rval = Esys_FlushContext(session->internal.ectx, session->output.session_handle);
            if (rval != TSS2_RC_SUCCESS) {
                LOG_WARN("Esys_FlushContext: 0x%x", rval);
            }
            return false;
        }
    }

    return true;
}

static void tpm2_session_free(tpm2_session **session) {

    tpm2_session *s = *session;

    if (s) {
        free(s->input);
        if (s->internal.path) {
            free(s->internal.path);
        }
        free(s);
        *session = NULL;
    }
}

tpm2_session *tpm2_session_open(ESYS_CONTEXT *context,
        tpm2_session_data *data) {

    tpm2_session *session = calloc(1, sizeof(tpm2_session));
    if (!session) {
        free(data);
        LOG_ERR("oom");
        return NULL;
    }

    if (data->path) {
        session->internal.path = strdup(data->path);
        if (!session->internal.path) {
            LOG_ERR("oom");
            tpm2_session_free(&session);
            return NULL;
        }
    }

    session->input = data;
    session->internal.ectx = context;

    if (!context) {
        session->output.session_handle = ESYS_TR_PASSWORD;
        return session;
    }

    bool result = start_auth_session(session);
    if (!result) {
        tpm2_session_free(&session);
        return NULL;
    }

    return session;
}

/* SESSION_VERSION 1 was used prior to the switch to ESAPI. As the types of
 * several of the tpm2_session_data object members have changed the version is
 * bumped.
 */
#define SESSION_VERSION 2

/*
 * Checks that two types are equal in size.
 *
 * It works by leveraging the fact that C does not allow negative array sizes.
 * If the sizes are equal, the boolean equality operator will return 1, thus
 * a subtraction of 1 yields 0, which is a legal array size in C. In the false
 * case (ie sizes not equal), 0 - 1 is -1, which will cause the compiler to
 * complain.
 */
#define COMPILE_ASSERT_SIZE(a, b) \
    typedef char WRONG_SIZE_##a[(sizeof(a) == sizeof(b)) - 1]

// We check that the TSS library does not change sizes unbeknownst to us.
COMPILE_ASSERT_SIZE(ESYS_TR, UINT32);
COMPILE_ASSERT_SIZE(TPMI_ALG_HASH, UINT16);
COMPILE_ASSERT_SIZE(TPM2_SE, UINT8);

tpm2_session *tpm2_session_restore(ESYS_CONTEXT *ctx, const char *path, bool is_final) {

    tpm2_session *s = NULL;

    /*
     * Copy the string internally so callers need
     * not worry about it.
     */
    char *dup_path = strdup(path);
    if (!dup_path) {
        LOG_ERR("oom");
        return NULL;
    }

    FILE *f = fopen(dup_path, "rb");
    if (!f) {
        LOG_ERR("Could not open path \"%s\", due to error: \"%s\"",
                dup_path, strerror(errno));
        return NULL;
    }

    uint32_t version;
    bool result = files_read_header(f, &version);

    TPM2_SE type;
    result = files_read_bytes(f, &type, sizeof(type));
    if (!result) {
        LOG_ERR("Could not read session type");
        goto out;
    }

    TPMI_ALG_HASH auth_hash;
    result = files_read_16(f, &auth_hash);
    if (!result) {
        LOG_ERR("Could not read session digest algorithm");
        goto out;
    }

    ESYS_TR handle;
    result = files_load_tpm_context_from_file(ctx,
                    &handle, f);
    if (!result) {
        LOG_ERR("Could not load session context");
        goto out;
    }

    tpm2_session_data *d = tpm2_session_data_new(type);
    if (!d) {
        LOG_ERR("oom");
        goto out;
    }

    tpm2_session_set_authhash(d, auth_hash);

    s = tpm2_session_open(NULL, d);
    if (!s) {
        LOG_ERR("oom new session object");
        goto out;
    }

    s->output.session_handle = handle;
    s->internal.path = dup_path;
    s->internal.ectx = ctx;
    dup_path = NULL;

    TPM2_HANDLE sapi_handle = 0;
    TPMA_SESSION attrs = 0;

    if (ctx) {
        tpm2_util_esys_handle_to_sys_handle(ctx, handle,
                    &sapi_handle);

        /* hack this in here, should be done when starting the session */
        TSS2_RC rc =
            Esys_TRSess_GetAttributes(
                ctx,
                handle,
                &attrs);
        if (rc != TSS2_RC_SUCCESS) {
            LOG_WARN("Esys_TRSess_GetAttributes: 0x%x", rc);
        }
    }

    s->internal.is_final = is_final;

    LOG_INFO("Restored session: ESYS_TR(0x%x) SAPI(0x%x) attrs(0x%x)", handle, sapi_handle, attrs);

out:
    free(dup_path);
    if (f) {
        fclose(f);
    }
    return s;
}

tool_rc tpm2_session_close(tpm2_session **s) {

    tpm2_session *session = *s;

    FILE *session_file = NULL;

    tool_rc rc = tool_rc_general_error;

    if (!session) {
        return tool_rc_success;
    }

    /*
     * Do not back up:
     *   - password sessions are implicit
     *   - hmac sessions live the life of the tool
     */
    if (session->output.session_handle == ESYS_TR_PASSWORD) {
        rc = tool_rc_success;
        goto out;
    }

    bool flush = session->internal.is_final;
    const char *path = session->internal.path;
    if (path) {
        session_file = fopen(path, "w+b");
        if (!session_file) {
            LOG_ERR("Could not open path \"%s\", due to error: \"%s\"",
                    path, strerror(errno));
            goto out;
        }
    } else {
        flush = true;
    }

    if (flush) {

        rc = tpm2_flush_context(session->internal.ectx, session->output.session_handle);
        /* done use rc to inidcate status */
        goto out;
    }

    /*
     * Now write the session_type, handle and auth hash data to disk
     */
    bool result = files_write_header(session_file, SESSION_VERSION);
    if (!result) {
         LOG_ERR("Could not write context file header");
         goto out;
    }

    // UINT8 session type:
    TPM2_SE session_type = session->input->session_type;
    result = files_write_bytes(session_file, &session_type,
                sizeof(session_type));
    if (!result) {
        LOG_ERR("Could not write session type");
        goto out;
    }

    // UINT16 - auth hash digest
    TPMI_ALG_HASH hash = tpm2_session_get_authhash(session);
    result = files_write_16(session_file, hash);
    if (!result) {
        LOG_ERR("Could not write auth hash");
        goto out;
    }

    /*
     * Save session context at end of tpm2_session. With tabrmd support it
     * can be reloaded under certain circumstances.
     */

    ESYS_TR handle = tpm2_session_get_handle(session);

    TPM2_HANDLE sapi_handle = 0;
    tpm2_util_esys_handle_to_sys_handle(session->internal.ectx, handle,
                &sapi_handle);

    LOG_INFO("Saved session: ESYS_TR(0x%x) SAPI(0x%x)", handle, sapi_handle);

    rc = files_save_tpm_context_to_file(session->internal.ectx,
                tpm2_session_get_handle(session), session_file);
    if (rc != tool_rc_success) {
        LOG_ERR("Could not write session context");
    }

out:
    if (session_file) {
        fclose(session_file);
    }

    tpm2_session_free(s);

    return rc;
}

bool tpm2_session_restart(ESYS_CONTEXT *context, tpm2_session *s) {

    ESYS_TR handle = tpm2_session_get_handle(s);

    TSS2_RC rval = Esys_PolicyRestart(context, handle,
                        ESYS_TR_NONE, ESYS_TR_NONE, ESYS_TR_NONE);
    if (rval != TPM2_RC_SUCCESS) {
        LOG_PERR(Esys_PolicyRestart, rval);
    }

    return rval == TPM2_RC_SUCCESS;
}
