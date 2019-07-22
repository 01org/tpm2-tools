# SPDX-License-Identifier: BSD-3-Clause

source helpers.sh

TPM_RH_OWNER=0x40000001
SEALED_SECRET="SEALED-SECRET"
session_ctx=session.ctx
o_policy_digest=policy.digest
primary_ctx=prim.ctx
seal_key_pub=sealing_key.pub
seal_key_priv=sealing_key.priv
seal_key_ctx=sealing_key.ctx

cleanup() {
    rm -f  $session_ctx $o_policy_digest $primary_ctx $seal_key_pub $seal_key_priv\
    $seal_key_ctx

    tpm2_flushcontext -S $session_ctx 2>/dev/null || true

    tpm2_clear

    if [ "${1}" != "no-shutdown" ]; then
        shut_down
    fi
}
trap cleanup EXIT

start_up

cleanup "no-shutdown"

tpm2_clear

tpm2_changeauth -c o ownerauth

# Create Policy
tpm2_startauthsession -S $session_ctx
tpm2_policysecret -S $session_ctx -c $TPM_RH_OWNER -L $o_policy_digest ownerauth
tpm2_flushcontext -S $session_ctx
rm $session_ctx

# Create and Load Object
tpm2_createprimary -Q -C o  -o $primary_ctx -P ownerauth
tpm2_create -Q -g sha256 -u $seal_key_pub -r $seal_key_priv -C $primary_ctx\
  -L $o_policy_digest -i- <<< $SEALED_SECRET
tpm2_load -C $primary_ctx -u $seal_key_pub -r $seal_key_priv -o $seal_key_ctx

# Satisfy policy and unseal data
tpm2_startauthsession --policy-session -S $session_ctx
echo -n "ownerauth" | tpm2_policysecret -S $session_ctx -c $TPM_RH_OWNER -L $o_policy_digest file:-
unsealed=`tpm2_unseal -p"session:$session_ctx" -c $seal_key_ctx`
tpm2_flushcontext -S $session_ctx
rm $session_ctx

test "$unsealed" == "$SEALED_SECRET"

if [ $? != 0 ]; then
  echo "Failed policysecret integration test where in ref object password was set."
fi

#Test the policy with auth reference object password not set
unsealed=""
tpm2_changeauth -c o -p ownerauth

tpm2_startauthsession --policy-session -S $session_ctx
tpm2_policysecret -S $session_ctx -c $TPM_RH_OWNER -L $o_policy_digest
unsealed=`tpm2_unseal -p"session:$session_ctx" -c $seal_key_ctx`
tpm2_flushcontext -S $session_ctx
rm $session_ctx

test "$unsealed" == "$SEALED_SECRET"

if [ $? != 0 ]; then
  echo "Failed policysecret integration test where in ref object is passwordless."
fi

exit 0
