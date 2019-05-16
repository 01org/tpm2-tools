#!/bin/bash
# SPDX-License-Identifier: BSD-3-Clause

source helpers.sh

cleanup() {
    tpm2_clearcontrol -c -p

    shut_down
}
trap cleanup EXIT

start_up

tpm2_clearcontrol

tpm2_clearcontrol -c -p

tpm2_clear

exit 0
