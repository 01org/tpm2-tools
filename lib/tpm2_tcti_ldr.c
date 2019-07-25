/* SPDX-License-Identifier: BSD-3-Clause */

#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "tpm2_tcti_ldr.h"

#define TSS2_TCTI_SO_FORMAT "libtss2-tcti-%s.so.0"

static void *handle;
static const TSS2_TCTI_INFO *info;

void tpm2_tcti_ldr_unload(void) {
    if (handle) {
#ifndef DISABLE_DLCLOSE
        tpm2_util_dlclose(handle);
#endif
        handle = NULL;
        info = NULL;
    }
}

const TSS2_TCTI_INFO *tpm2_tcti_ldr_getinfo(void) {
    return info;
}

static void* tpm2_tcti_ldr_dlopen(const char *name) {

    char path[PATH_MAX];
    size_t size = snprintf(path, sizeof(path), TSS2_TCTI_SO_FORMAT, name);
    if (size >= sizeof(path)) {
        LOG_ERR("Truncated TCTI friendly name conversion, got: \"%s\", made: \"%s\"",
                name, path);
        return NULL;
    }

    return tpm2_util_dlopen(path, RTLD_LAZY);
}

bool tpm2_tcti_ldr_is_tcti_present(const char *name) {

    void *handle = tpm2_tcti_ldr_dlopen(name);
    if (handle) {
        tpm2_util_dlclose(handle);
    }

    return handle != NULL;
}

TSS2_TCTI_CONTEXT *tpm2_tcti_ldr_load(const char *path, const char *opts) {

    TSS2_TCTI_CONTEXT *tcti_ctx = NULL;

    if (handle) {
        LOG_ERR("Attempting to load multiple tcti's simultaneously is not supported!");
        return NULL;
    }

    /*
     * Try what they gave us, if it doesn't load up, try
     * libtss2-tcti-xxx.so replacing xxx with what they gave us.
     */
    handle = tpm2_util_dlopen (path, RTLD_LAZY);
    if (!handle) {

        handle = tpm2_tcti_ldr_dlopen(path);
        if (!handle) {
            LOG_ERR("Could not dlopen library: \"%s\"", PSTR(path));
            return NULL;
        }
    }

    TSS2_TCTI_INFO_FUNC infofn = (TSS2_TCTI_INFO_FUNC)tpm2_util_dlsym(handle, TSS2_TCTI_INFO_SYMBOL);
    if (!infofn) {
        LOG_ERR("Symbol \"%s\"not found in library: \"%s\"",
                TSS2_TCTI_INFO_SYMBOL, path);
        goto err;
    }

    info = infofn();

    TSS2_TCTI_INIT_FUNC init = info->init;

    size_t size;
    TSS2_RC rc = init(NULL, &size, opts);
    if (rc != TPM2_RC_SUCCESS) {
        LOG_ERR("tcti init setup routine failed for library: \"%s\""
                " options: \"%s\"", path, PSTR(opts));
        goto err;
    }

    tcti_ctx = (TSS2_TCTI_CONTEXT*) calloc(1, size);
    if (tcti_ctx == NULL) {
        LOG_ERR("oom");
        goto err;
    }

    rc = init(tcti_ctx, &size, opts);
    if (rc != TPM2_RC_SUCCESS) {
        LOG_ERR("tcti init allocation routine failed for library: \"%s\""
                " options: \"%s\"", path, PSTR(opts));
        goto err;
    }

    return tcti_ctx;

err:
    free(tcti_ctx);
    tpm2_util_dlclose(handle);
    return NULL;
}
