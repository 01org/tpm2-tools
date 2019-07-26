/* SPDX-License-Identifier: BSD-3-Clause */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "files.h"
#include "log.h"
#include "tpm2.h"
#include "tpm2_alg_util.h"
#include "tpm2_convert.h"
#include "tpm2_openssl.h"
#include "tpm2_tool.h"

typedef struct tpm_quote_ctx tpm_quote_ctx;
struct tpm_quote_ctx {
    struct {
        const char *ctx_path;
        const char *auth_str;
        tpm2_loaded_object object;
    } ak;

    char *signature_path;
    char *message_path;
    char *pcr_path;
    FILE *pcr_output;
    tpm2_convert_sig_fmt sig_format;
    TPMI_ALG_HASH sig_hash_algorithm;
    tpm2_algorithm algs;
    TPM2B_DATA qualifyingData;
    TPML_PCR_SELECTION pcrSelections;
    TPMS_CAPABILITY_DATA cap_data;

    struct {
        UINT8  p : 1;
        UINT8 l : 1;
        UINT8 L : 1;
        UINT8 o : 1;
        UINT8 G : 1;
    } flags;

    tpm2_pcrs pcrs;
};

static tpm_quote_ctx ctx = {
    .algs = {
        .count = 3,
        .alg = {
            TPM2_ALG_SHA1,
            TPM2_ALG_SHA256,
            TPM2_ALG_SHA384
        }
    },
    .qualifyingData = TPM2B_EMPTY_INIT,
};


// write all PCR banks according to g_pcrSelection & g_pcrs->
static bool write_pcr_values() {

    // PCR output to file wasn't requested
    if (ctx.pcr_output == NULL) {
        return true;
    }

    // Export TPML_PCR_SELECTION structure to pcr outfile
    if (fwrite(&ctx.pcrSelections,
            sizeof(TPML_PCR_SELECTION), 1,
            ctx.pcr_output) != 1) {
        LOG_ERR("write to output file failed: %s", strerror(errno));
        return false;
    }

    // Export PCR digests to pcr outfile
    if (fwrite(&ctx.pcrs.count, sizeof(UINT32), 1, ctx.pcr_output) != 1) {
        LOG_ERR("write to output file failed: %s", strerror(errno));
        return false;
    }

    UINT32 j;
    for (j = 0; j < ctx.pcrs.count; j++) {
        if (fwrite(&ctx.pcrs.pcr_values[j], sizeof(TPML_DIGEST), 1, ctx.pcr_output) != 1) {
            LOG_ERR("write to output file failed: %s", strerror(errno));
            return false;
        }
    }

    return true;
}

static bool write_output_files(TPM2B_ATTEST *quoted, TPMT_SIGNATURE *signature) {

    bool res = true;
    if (ctx.signature_path) {
        res &= tpm2_convert_sig_save(signature, ctx.sig_format, ctx.signature_path);
    }

    if (ctx.message_path) {
        res &= files_save_bytes_to_file(ctx.message_path,
                (UINT8*)quoted->attestationData,
                quoted->size);
    }

    res &= write_pcr_values();

    return res;
}

static tool_rc quote(ESYS_CONTEXT *ectx, TPML_PCR_SELECTION *pcrSelection) {

    TPM2B_ATTEST *quoted = NULL;
    TPMT_SIGNATURE *signature = NULL;
    TPMT_SIG_SCHEME inScheme = {
        .scheme = TPM2_ALG_NULL
    };

    if(ctx.flags.G) {
        tool_rc rc = tpm2_alg_util_get_signature_scheme(ectx, ctx.ak.object.tr_handle,
                                    ctx.sig_hash_algorithm, TPM2_ALG_NULL, &inScheme);
        if (rc != tool_rc_success) {
            return rc;
        }
    }

    tool_rc rc = tpm2_quote(ectx, &ctx.ak.object, &inScheme, &ctx.qualifyingData,
        pcrSelection, &quoted, &signature);
    if (rc != tool_rc_success) {
        return rc;
    }

    tpm2_tool_output( "quoted: " );
    tpm2_util_print_tpm2b(quoted);
    tpm2_tool_output("\nsignature:\n" );
    tpm2_tool_output("  alg: %s\n", tpm2_alg_util_algtostr(signature->sigAlg, tpm2_alg_util_flags_sig));

    UINT16 size;
    BYTE *sig = tpm2_convert_sig(&size, signature);
    if (!sig) {
        return tool_rc_general_error;
    }
    tpm2_tool_output("  sig: ");
    tpm2_util_hexdump(sig, size);
    tpm2_tool_output("\n");
    free(sig);

    if (ctx.pcr_output) {
        // Filter out invalid/unavailable PCR selections
        if (!pcr_check_pcr_selection(&ctx.cap_data, &ctx.pcrSelections)) {
            LOG_ERR("Failed to filter unavailable PCR values for quote!");
            return tool_rc_general_error;
        }

        // Gather PCR values from the TPM (the quote doesn't have them!)
        rc = pcr_read_pcr_values(ectx, &ctx.pcrSelections, &ctx.pcrs);
        if (rc != tool_rc_success) {
            LOG_ERR("Failed to retrieve PCR values related to quote!");
            return rc;
        }

        // Grab the digest from the quote
        TPM2B_DIGEST quoteDigest = TPM2B_TYPE_INIT(TPM2B_DIGEST, buffer);
        TPM2B_DATA extraData = TPM2B_TYPE_INIT(TPM2B_DATA, buffer);
        if (!tpm2_util_get_digest_from_quote(quoted, &quoteDigest, &extraData)) {
            LOG_ERR("Failed to get digest from quote!");
            return tool_rc_general_error;
        }

        // Print out PCR values as output
        if (!pcr_print_pcr_struct(&ctx.pcrSelections, &ctx.pcrs)) {
            LOG_ERR("Failed to print PCR values related to quote!");
            return tool_rc_general_error;
        }

        // Calculate the digest from our selected PCR values (to ensure correctness)
        TPM2B_DIGEST pcr_digest = TPM2B_TYPE_INIT(TPM2B_DIGEST, buffer);
        if (!tpm2_openssl_hash_pcr_banks(ctx.sig_hash_algorithm, &ctx.pcrSelections, &ctx.pcrs, &pcr_digest)) {
            LOG_ERR("Failed to hash PCR values related to quote!");
            return tool_rc_general_error;
        }
        tpm2_tool_output("calcDigest: ");
        tpm2_util_hexdump(pcr_digest.buffer, pcr_digest.size);
        tpm2_tool_output("\n");

        // Make sure digest from quote matches calculated PCR digest
        if (!tpm2_util_verify_digests(&quoteDigest, &pcr_digest)) {
            LOG_ERR("Error validating calculated PCR composite with quote");
            return tool_rc_general_error;
        }
    }

    // Write everything out
    bool res = write_output_files(quoted, signature);

    free(quoted);
    free(signature);

    return res ? tool_rc_success : tool_rc_general_error;
}

static bool on_option(char key, char *value) {

    switch(key)
    {
    case 'C':
        ctx.ak.ctx_path = value;
        break;
    case 'P':
        ctx.ak.auth_str = value;
        break;
    case 'i':
        if(!pcr_parse_list(value, strlen(value), &ctx.pcrSelections.pcrSelections[0]))
        {
            LOG_ERR("Could not parse pcr list, got: \"%s\"", value);
            return false;
        }
        ctx.flags.l = 1;
        break;
    case 'l':
        if(!pcr_parse_selections(value, &ctx.pcrSelections))
        {
            LOG_ERR("Could not parse pcr selections, got: \"%s\"", value);
            return false;
        }
        ctx.flags.L = 1;
        break;
    case 'q':
        ctx.qualifyingData.size = sizeof(ctx.qualifyingData) - 2;
        if(tpm2_util_hex_to_byte_structure(value, &ctx.qualifyingData.size, ctx.qualifyingData.buffer) != 0)
        {
            LOG_ERR("Could not convert \"%s\" from a hex string to byte array!", value);
            return false;
        }
        break;
    case 's':
         ctx.signature_path = value;
         break;
    case 'm':
         ctx.message_path = value;
         break;
    case 'f':
         ctx.pcr_path = value;
         ctx.flags.p = 1;
         break;
    case 'F':
         ctx.sig_format = tpm2_convert_sig_fmt_from_optarg(value);

         if (ctx.sig_format == signature_format_err) {
            return false;
         }
         break;
    case 'g':
        ctx.sig_hash_algorithm = tpm2_alg_util_from_optarg(value, tpm2_alg_util_flags_hash);
        if(ctx.sig_hash_algorithm == TPM2_ALG_ERROR) {
            LOG_ERR("Could not convert signature hash algorithm selection, got: \"%s\"", value);
            return false;
        }
        ctx.flags.G = 1;
        break;
    }

    return true;
}

bool tpm2_tool_onstart(tpm2_options **opts) {

    static const struct option topts[] = {
        { "ak-context",           required_argument, NULL, 'C' },
        { "ak-auth",              required_argument, NULL, 'P' },
        { "pcr-index",            required_argument, NULL, 'i' },
        { "pcr-list",             required_argument, NULL, 'l' },
        { "qualification",        required_argument, NULL, 'q' },
        { "signature",            required_argument, NULL, 's' },
        { "message",              required_argument, NULL, 'm' },
        { "pcr",                  required_argument, NULL, 'f' },
        { "format",               required_argument, NULL, 'F' },
        { "hash-algorithm",       required_argument, NULL, 'g' }
    };

    *opts = tpm2_options_new("C:P:i:l:q:s:m:f:F:g:", ARRAY_LEN(topts), topts,
                             on_option, NULL, 0);

    return *opts != NULL;
}

tool_rc tpm2_tool_onrun(ESYS_CONTEXT *ectx, tpm2_option_flags flags) {

    UNUSED(flags);

    /* TODO this whole file needs to be re-done, especially the option validation */
    if (!ctx.flags.l && !ctx.flags.L) {
        LOG_ERR("Expected either -l or -L to be specified.");
        return tool_rc_option_error;
    }

    tool_rc rc = tpm2_util_object_load_auth(ectx, ctx.ak.ctx_path,
        ctx.ak.auth_str, &ctx.ak.object, false, TPM2_HANDLES_ALL_W_NV);
    if (rc != tool_rc_success) {
        LOG_ERR("Invalid AK authorization");
        return rc;
    }

    if (ctx.flags.p) {
        if (!ctx.flags.G) {
            LOG_ERR("Must specify -G if -p is requested.");
            return tool_rc_option_error;
        }
        ctx.pcr_output = fopen(ctx.pcr_path, "wb+");
        if (!ctx.pcr_output) {
            LOG_ERR("Could not open PCR output file \"%s\" error: \"%s\"",
                    ctx.pcr_path, strerror(errno));
            return tool_rc_general_error;
        }
    }

    rc = pcr_get_banks(ectx, &ctx.cap_data, &ctx.algs);
    if (rc != tool_rc_success) {
        return rc;
    }

    return quote(ectx, &ctx.pcrSelections);
}

tool_rc tpm2_tool_onstop(ESYS_CONTEXT *ectx) {
    UNUSED(ectx);
    if (ctx.pcr_output) {
        fclose(ctx.pcr_output);
    }
    return tpm2_session_close(&ctx.ak.object.session);
}
