#!/bin/bash
#;**********************************************************************;
#
# Copyright (c) 2016-2018, Intel Corporation
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# 3. Neither the name of Intel Corporation nor the names of its contributors
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGE.
#;**********************************************************************;

source helpers.sh

cleanup() {
    rm -f ek.pub ek.log ek.template ek.nonce

    # Evict persistent handles, we want them to always succeed and never trip
    # the onerror trap.
    tpm2_evictcontrol -Q -a o -c 0x81010005 2>/dev/null || true

    if [ "$1" != "no-shut-down" ]; then
      shut_down
    fi
}
trap cleanup EXIT

start_up

cleanup "no-shut-down"

tpm2_createek -c 0x81010005 -G rsa -p ek.pub

cleanup "no-shut-down"

tpm2_createek -c - -G rsa -p ek.pub > ek.log
phandle=`yaml_get_kv ek.log \"persistent\-handle\"`
tpm2_evictcontrol -Q -a o -c $phandle

cleanup "no-shut-down"

tpm2_createek -G rsa -p ek.pub

cleanup "no-shut-down"

ek_nonce_index=0x01c00003
ek_template_index=0x01c00004

# Define RSA EK template
nbytes=$(wc -c $TPM2_TOOLS_TEST_FIXTURES/ek-template-default.bin | cut -f1 -d' ')
tpm2_nvdefine -Q -x $ek_template_index -a o -s $nbytes -b "ownerread|policywrite|ownerwrite"
tpm2_nvwrite -Q -x $ek_template_index -a o $TPM2_TOOLS_TEST_FIXTURES/ek-template-default.bin

# Define RSA EK nonce
echo -n -e '\0' > ek.nonce
tpm2_nvdefine -Q -x $ek_nonce_index -a o -s 1 -b "ownerread|policywrite|ownerwrite"
tpm2_nvwrite -Q -x $ek_nonce_index -a o ek.nonce

tpm2_createek -t -G rsa -p ek.pub

cleanup "no-shut-down"

exit 0
