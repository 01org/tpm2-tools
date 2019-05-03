/* SPDX-License-Identifier: BSD-2-Clause */
//**********************************************************************;
// Copyright (c) 2015, Intel Corporation
// All rights reserved.
//
//**********************************************************************;

#ifndef SRC_TPM_KDFA_H_
#define SRC_TPM_KDFA_H_

#include <tss2/tss2_sys.h>

/* TODO DOCUMENT ME */
/**
 *
 * @param hashAlg
 * @param key
 * @param label
 * @param contextU
 * @param contextV
 * @param bits
 * @param resultKey
 * @return
 */
TSS2_RC tpm2_kdfa(TPMI_ALG_HASH hashAlg,
        TPM2B *key, char *label, TPM2B *contextU, TPM2B *contextV,
        UINT16 bits, TPM2B_MAX_BUFFER *resultKey );

#endif /* SRC_TPM_KDFA_H_ */
