/*
 * Copyright (c) 2016, Intel Corporation
 * Copyright (c) 2018, Fraunhofer SIT
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Intel Corporation nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdbool.h>
#include <stdlib.h>

#include <unistd.h>

#include "log.h"
#include "tpm2_tcti_ldr.h"
#include "tpm2_options.h"
#include "tpm2_util.h"
#include "tpm2_errata.h"
#include "main.h"

bool output_enabled = true;

static void tcti_teardown (TSS2_TCTI_CONTEXT *tcti_context) {
    if (tcti_context == NULL)
        return;
    Tss2_Tcti_Finalize (tcti_context);
    free (tcti_context);
}

static void esys_teardown (ESYS_CONTEXT **esys_context) {

    if (esys_context == NULL)
        return;
    if (*esys_context == NULL)
        return;
    Esys_Finalize (esys_context);
}

static void esys_teardown_full (ESYS_CONTEXT **esys_context) {

    TSS2_TCTI_CONTEXT *tcti_context = NULL;
    TSS2_RC rc;

    rc = Esys_GetTcti(*esys_context, &tcti_context);
    if (rc != TPM2_RC_SUCCESS)
        return;
    esys_teardown (esys_context);
    tcti_teardown (tcti_context);
}

#define SUPPORTED_ABI_VERSION \
{ \
    .tssCreator = 1, \
    .tssFamily = 2, \
    .tssLevel = 1, \
    .tssVersion = 108, \
}

static ESYS_CONTEXT* esys_ctx_init(TSS2_TCTI_CONTEXT *tcti_ctx) {

    TSS2_ABI_VERSION abi_version = SUPPORTED_ABI_VERSION;
    ESYS_CONTEXT *esys_ctx;

    TSS2_RC rval = Esys_Initialize(&esys_ctx, tcti_ctx, &abi_version);
    if (rval != TPM2_RC_SUCCESS) {
        LOG_PERR(Esys_Initialize, rval);
        return NULL;
    }

    return esys_ctx;
}

/*
 * This program is a template for TPM2 tools that use the ESAPI. It does
 * nothing more than parsing command line options that allow the caller to
 * specify which TCTI to use for the test.
 */
int main(int argc, char *argv[]) {

    int ret = 1;

    tpm2_options *tool_opts = NULL;
    if (tpm2_tool_onstart) {
        bool res = tpm2_tool_onstart(&tool_opts);
        if (!res) {
            LOG_ERR("retrieving tool options");
            return 1;
        }
    }

    if (argc == 1 && tool_opts && (tool_opts->flags & TPM2_OPTIONS_SHOW_USAGE)) {
        tpm2_print_usage(argv[0], tool_opts);
        return ret;
    }

    tpm2_option_flags flags = { .all = 0 };
    TSS2_TCTI_CONTEXT *tcti = NULL;
    tpm2_option_code rc = tpm2_handle_options(argc, argv, tool_opts, &flags, &tcti);
    if (rc != tpm2_option_code_continue) {
        ret = rc == tpm2_option_code_err ? 1 : 0;
        goto free_opts;
    }

    if (flags.verbose) {
        log_set_level(log_level_verbose);
    }

    /*
     * We don't want a cyclic dependency between tools/options. Resolving those
     * works well on linux/elf based systems, but darwin and windows tend to
     * fall flat on there face. This is why we set quiet mode outside of
     * option and argument life-cycle. Thus TOOL_OUTPUT is only guaranteed
     * to respect quiet from here on out (onrun and onexit).
     */
    if (flags.quiet) {
        output_enabled = false;
    }

    /* figure out the tcti */

    /* TODO ESYS INIT */
    ESYS_CONTEXT *esys_context = NULL;
    if (tcti) {
        esys_context = esys_ctx_init(tcti);
        if (!esys_context) {
            goto free_opts;
        }
    }

    if (flags.enable_errata) {
        LOG_ERR("errata flag currently not supported");
        //TODO: tpm2_errata_init(esys_context);
    }

    /*
     * Call the specific tool, all tools implement this function instead of
     * 'main'.
     */
    ret = tpm2_tool_onrun(esys_context, flags) ? 1 : 0;
    if (ret != 0) {
        LOG_ERR("Unable to run %s", argv[0]);
    }

    /*
     * Cleanup contexts & memory allocated for the modified argument vector
     * passed to execute_tool.
     */
    esys_teardown_full(&esys_context);

free_opts:
    if (tool_opts) {
        tpm2_options_free(tool_opts);
    }

    if (tpm2_tool_onexit) {
        tpm2_tool_onexit();
    }

    tpm2_tcti_ldr_unload();

    exit(ret);
}
