#;**********************************************************************;
#
# Copyright (c) 2017, Alibaba Group
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

populate_hash_algs() {
    declare -A local name2hex=(
        ["sha1"]=0x04
        ["sha256"]=0x0B
        ["sha384"]=0x0C
        ["sha512"]=0x0D
        ["sm3_256"]=0x12
    )
    local algs="`tpm2_getcap -c algorithms | grep 'hash:\s*set$' -B 3 | awk '{ print $6 }' | xargs`"
    local algs_supported=""
    local t_alg

    # Filter out the hash algorithms not appropriate for the test.
    for t_alg in $algs; do
        [ ! ${name2hex[$t_alg]} ] && continue

        algs_supported="$t_alg $algs_supported"
    done

    local mode=${1:-"name"}
    local ret=""
    local let i=0

    for t_alg in $algs_supported; do
        if [ "$mode" = "hex" ]; then
            ret="$ret ${name2hex[$t_alg]}"
        elif [ "$mode" = "mixed" ]; then
            [ $i -eq 0 ] && ret="$ret $t_alg" || ret="$ret ${name2hex[$t_alg]}"
            let "i=$i^1"
        else
            echo "$algs_supported"
            return
        fi
    done

    echo "$ret"
}

# Return alg argument if supported by TPM.
hash_alg_supported() {
    local orig_alg="$1"
    local alg="$orig_alg"
    local algs_supported="`populate_hash_algs name`"
    local hex2name=(
        [0x04]="sha1"
        [0x0B]="sha256"
        [0x0C]="sha384"
        [0x0D]="sha512"
        [0x12]="sm3_256"
    )

    if [ -z "$alg" ]; then
        echo "$algs_supported"
        return
    fi

    if [ "$alg" = "${alg//[^0-9a-fA-FxX]/}" ]; then
        alg=${hex2name["$alg"]}
        [ -z "$alg" ] && return
    fi

    local t_alg
    for t_alg in $algs_supported; do
        if [ "$t_alg" = "$alg" ]; then
            echo "$orig_alg"
            return
        fi
    done
}

# Certain TPM 2.0 chip, e.g, Nationz Z32H320TC, and tpm2simulator referring
# to TPM 2.0 spec rev 1.16 may have an errata, disallowing both decrypt and
# sign set for a symcipher object, and in response to RC_ATTRIBUTES. In
# order to work around it, we attempt to run tpm2_create again with sign bit
# clear.
create_object() {
    local alg_create_obj=0x20072

    if tpm2_create $@ 2>&1 | grep -q 'Create Object Failed ! ErrorCode: 0x2c2'; then
        tpm2_create -Q -A $alg_create_obj $@
    else
        true
    fi
}

import_object() {
    local alg_import_obj=0x20040

    if tpm2_import $@ 2>&1 | grep -q 'ERROR: Failed Key Import 000002C2'; then
        tpm2_import -A $alg_import_obj $@
    else
        true
    fi
}
