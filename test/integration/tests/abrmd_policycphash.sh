# SPDX-License-Identifier: BSD-3-Clause

source helpers.sh

cleanup() {
    rm -f helpers.sh signing_key_private.pem signing_key_public.pem \
    signing_key.ctx signing_key.name authorized.policy policy.dat \
    primary.ctx key.prv key.pub key.ctx new_parent.prv new_parent.pub \
    new_parent.ctx new_parent.name key.name name.hash policy.namehash \
    policynamehash.signature policy.namehash verification.tkt dupprv.bin \
    dupseed.dat

    tpm2_flushcontext session.ctx 2>/dev/null || true
    if [ "${1}" != "no-shutdown" ]; then
        shut_down
    fi
}
trap cleanup EXIT

start_up

cleanup "no-shutdown"

generate_policycphash() {
    tpm2_startauthsession -S session.ctx -g sha256
    tpm2_policycphash -S session.ctx -L policy.cphash --cphash cp.hash
    tpm2_flushcontext session.ctx
    rm session.ctx
}

sign_and_verify_policycphash() {
    openssl dgst -sha256 -sign signing_key_private.pem \
    -out policycphash.signature policy.cphash

    tpm2_verifysignature -c signing_key.ctx -g sha256 -m policy.cphash \
    -s policycphash.signature -t verification.tkt -f rsassa
}

setup_authorized_policycphash() {
    tpm2_startauthsession -S session.ctx --policy-session -g sha256
    tpm2_policycphash -S session.ctx --cphash cp.hash
    tpm2_policyauthorize -S session.ctx -i policy.cphash -n signing_key.name \
    -t verification.tkt
}

setup_owner_policy() {
    tpm2_setprimarypolicy -C o -L policy.cphash -g sha256
    tpm2_startauthsession -S session.ctx --policy-session -g sha256
    tpm2_policycphash -S session.ctx --cphash cp.hash
}

start_policy_cphash() {
    tpm2_startauthsession -S session.ctx --policy-session -g sha256
    tpm2_policycphash -S session.ctx --cphash cp.hash
}

create_authorized_policy() {
  tpm2_clear
  # Define an authorized policy for an object
  openssl genrsa -out signing_key_private.pem 2048
  openssl rsa -in signing_key_private.pem -out signing_key_public.pem -pubout
  tpm2_loadexternal -G rsa -C o -u signing_key_public.pem -c signing_key.ctx \
  -n signing_key.name
  tpm2_startauthsession -S session.ctx -g sha256
  tpm2_policyauthorize -S session.ctx -L authorized.policy -n signing_key.name
  tpm2_flushcontext session.ctx
}

# Restrict the value that can be set through tpm2_nvsetbits.
create_authorized_policy
tpm2_nvdefine 1 -a "policywrite|authwrite|ownerread|nt=bits" -L authorized.policy
## Create policycphash
tpm2_nvsetbits 1 -i 1 --cphash cp.hash
generate_policycphash
## Sign and verify policycphash
sign_and_verify_policycphash
## Satisfy policycphash and execute nvsetbits
setup_authorized_policycphash
tpm2_nvsetbits 1 -i 1 -P "session:session.ctx"
tpm2_flushcontext session.ctx
## Attempt setting another bit which was not recorded in policycphash
setup_authorized_policycphash
trap - ERR
tpm2_nvsetbits 1 -i 2 -P "session:session.ctx"
if [ $? == 0 ];then
  echo "ERROR: nvsetbits must fail!"
  exit 1
fi
trap onerror ERR
tpm2_flushcontext session.ctx
tpm2_nvundefine 1

# Test tpm2_nvextend
create_authorized_policy
tpm2_nvdefine 1 -a "nt=extend|ownerread|policywrite" -L authorized.policy
echo "foo" | tpm2_nvextend -i- 1 --cphash cp.hash
generate_policycphash
sign_and_verify_policycphash
setup_authorized_policycphash
echo "foo" | tpm2_nvextend -i- 1 -P "session:session.ctx"
tpm2_flushcontext session.ctx
## test the failing scenario
setup_authorized_policycphash
trap - ERR
echo "food" | tpm2_nvextend -i- 1 -P "session:session.ctx"
if [ $? == 0 ];then
  echo "ERROR: nvextend must fail!"
  exit 1
fi
trap onerror ERR
tpm2_flushcontext session.ctx
tpm2_nvundefine 1

# Test tpm2_nvincrement
create_authorized_policy
tpm2_nvdefine 1 -s 8 -a "nt=counter|ownerread|policywrite" -L authorized.policy
tpm2_nvincrement 1 --cphash cp.hash
generate_policycphash
sign_and_verify_policycphash
setup_authorized_policycphash
tpm2_nvincrement 1 -P "session:session.ctx"
tpm2_flushcontext session.ctx
tpm2_nvundefine 1

# Test tpm2_nvread
create_authorized_policy
tpm2_nvdefine 1 -s 8 -a "ownerwrite|policyread" -L authorized.policy
echo "foo" | tpm2_nvwrite 1 -i- -C o
tpm2_nvread 1 -s 8 --cphash cp.hash
generate_policycphash
sign_and_verify_policycphash
setup_authorized_policycphash
tpm2_nvread 1 -s 8 -P "session:session.ctx" | xxd -p
## test the failing scenario
setup_authorized_policycphash
trap - ERR
tpm2_nvread 1 -s 7 --offset 1 -P "session:session.ctx"
if [ $? == 0 ];then
  echo "ERROR: nvread must fail!"
  exit 1
fi
trap onerror ERR
tpm2_flushcontext session.ctx
tpm2_nvundefine 1

# Test tpm2_nvreadlock
create_authorized_policy
tpm2_nvdefine 1 -C o -s 32 -a "policyread|policywrite|read_stclear" \
-L authorized.policy
tpm2_nvreadlock 1 -C 0x01000001 --cphash cp.hash
generate_policycphash
sign_and_verify_policycphash
setup_authorized_policycphash
tpm2_nvreadlock 1 -C 0x01000001 -P "session:session.ctx"
tpm2_flushcontext session.ctx
tpm2_nvundefine 1

# Test tpm2_nvwritelock
create_authorized_policy
tpm2_nvdefine 1 -C o -s 32 -a "policyread|policywrite|writedefine" \
-L authorized.policy
tpm2_nvwritelock 1 -C 0x01000001 --cphash cp.hash
generate_policycphash
sign_and_verify_policycphash
setup_authorized_policycphash
tpm2_nvwritelock 1 -C 0x01000001 -P "session:session.ctx"
tpm2_flushcontext session.ctx
tpm2_nvundefine 1
## attempt with globallock attribute set
tpm2_nvdefine 1 -C o -s 32 -a "ownerread|ownerwrite|globallock"
tpm2_nvwritelock --global -C o --cphash cp.hash
generate_policycphash
setup_owner_policy
tpm2_nvwritelock --global -C o -P "session:session.ctx"
tpm2_flushcontext session.ctx
tpm2_nvundefine 1

# Test tpm2_nvdefine
tpm2_nvdefine 1 -C o -s 32 -a "ownerread|ownerwrite" --cphash cp.hash
generate_policycphash
setup_owner_policy
tpm2_nvdefine 1 -C o -s 32 -a "ownerread|ownerwrite" -P "session:session.ctx"
tpm2_flushcontext session.ctx
## attempt failing scenario
start_policy_cphash
trap - ERR
tpm2_nvdefine 2 -C o -s 32 -a "ownerread|ownerwrite" -P "session:session.ctx"
if [ $? == 0 ];then
  echo "ERROR: nvdefine must fail!"
  exit 1
fi
trap onerror ERR
tpm2_flushcontext session.ctx
tpm2_nvundefine 1

# Test tpm2_nvundefine
tpm2_nvdefine 1
tpm2_nvundefine 1 --cphash cp.hash
generate_policycphash
setup_owner_policy
tpm2_nvundefine 1 -P "session:session.ctx"
tpm2_flushcontext session.ctx
## attempt failing scenario
tpm2_nvdefine 2
start_policy_cphash
trap - ERR
tpm2_nvundefine 2 -P "session:session.ctx"
if [ $? == 0 ];then
  echo "ERROR: nvundefine must fail!"
  exit 1
fi
trap onerror ERR
tpm2_flushcontext session.ctx
tpm2_nvundefine -C p 2

#Test tpm2_nvcertify
tpm2_createprimary -C o -c primary.ctx -Q
tpm2_create -G rsa -u signing_key.pub -r signing_key.priv -C primary.ctx \
-c signing_key.ctx -Q
tpm2_readpublic -c signing_key.ctx -f pem -o sslpub.pem -Q
tpm2_nvdefine -s 32 -C o -a "ownerread|ownerwrite|authread|authwrite" 1
dd if=/dev/urandom bs=1 count=32 status=none| tpm2_nvwrite 1 -i-
tpm2_nvcertify -C signing_key.ctx -g sha256 -f plain -s rsassa \
-o signature.bin --attestation attestation.bin --size 32 1 -c o --cphash cp.hash
generate_policycphash
setup_owner_policy
tpm2_nvcertify -C signing_key.ctx -g sha256 -f plain -s rsassa \
-o signature.bin --attestation attestation.bin --size 32 1 -c o -p "session:session.ctx"
tpm2_flushcontext session.ctx
tpm2_nvundefine 1

#Test tpm2_policynv
tpm2_nvdefine -C o -a "ownerwrite|ownerread" -s 2 1
operandA=0x81
operandB=0x80
echo $operandA | xxd -r -p | tpm2_nvwrite -C o -i- 1
tpm2_startauthsession -S policy_session.ctx --policy-session -g sha256
echo $operandB | xxd -r -p | tpm2_policynv -i- -C o --cphash cp.hash 1 neq -S policy_session.ctx
generate_policycphash
setup_owner_policy
echo $operandB | xxd -r -p | tpm2_policynv -S policy_session.ctx -i- -C o -P "session:session.ctx" 1 neq
tpm2_flushcontext policy_session.ctx
tpm2_flushcontext session.ctx
tpm2_nvundefine 1

# Test tpm2_policyauthorizenv
tpm2_nvdefine -C o 1 -a "ownerread|ownerwrite" -s 34
tpm2_startauthsession -S session.ctx
tpm2_policypassword -S session.ctx -L policy.pass
tpm2_flushcontext session.ctx
POLICYDIGESTALGORITHM=000b
echo $POLICYDIGESTALGORITHM | xxd -p -r | cat - policy.pass | \
tpm2_nvwrite -C o 1 -i-
tpm2_startauthsession -S policy_session.ctx --policy-session
tpm2_policyauthorizenv -S policy_session.ctx -C o 1 --cphash cp.hash
generate_policycphash
setup_owner_policy
tpm2_policypassword -S policy_session.ctx
tpm2_policyauthorizenv -S policy_session.ctx -C o 1 -P "session:session.ctx"
tpm2_flushcontext policy_session.ctx
tpm2_flushcontext session.ctx
tpm2_nvundefine 1

# Test tpm2_policysecret
tpm2_startauthsession -S policy_session.ctx --policy-session -g sha256
tpm2_policysecret -S policy_session.ctx -c o --cphash cp.hash
tpm2_startauthsession -S session.ctx -g sha256
tpm2_policyauthvalue -S session.ctx -L policy.authval
tpm2_policycphash -S session.ctx --cphash cp.hash -L policy.authval_cphash
tpm2_flushcontext session.ctx
tpm2_setprimarypolicy -C o -L policy.authval_cphash -g sha256
tpm2_startauthsession -S session.ctx --policy-session -g sha256
tpm2_policyauthvalue -S session.ctx
tpm2_policycphash -S session.ctx --cphash cp.hash
## Changing the policysecret authhandle parameter fro "o" to "p" should fail
tpm2_policysecret -S policy_session.ctx -c o session:session.ctx
tpm2_flushcontext session.ctx
tpm2_flushcontext policy_session.ctx

# Test tpm2_create
create_authorized_policy
tpm2_createprimary -C o -c prim.ctx -G rsa -L authorized.policy
tpm2_create -C prim.ctx -G rsa --cphash cp.hash
generate_policycphash
sign_and_verify_policycphash
setup_authorized_policycphash
tpm2_create -C prim.ctx -G rsa -P "session:session.ctx"
tpm2_flushcontext session.ctx
## Attempt creating a key type that was not recorded in policycphash
setup_authorized_policycphash
trap - ERR
tpm2_create -C prim.ctx -G aes -P "session:session.ctx"
if [ $? == 0 ];then
  echo "ERROR: tpm2_create must fail!"
  exit 1
fi
trap onerror ERR
tpm2_flushcontext session.ctx

# Test tpm2_load
create_authorized_policy
tpm2_createprimary -C o -c prim.ctx -G rsa -L authorized.policy
tpm2_create -C prim.ctx -G rsa -u key.pub -r key.priv
tpm2_create -C prim.ctx -G rsa -u key_2.pub -r key_2.priv
tpm2_load -C prim.ctx -u key.pub -r key.priv --cphash cp.hash
generate_policycphash
sign_and_verify_policycphash
setup_authorized_policycphash
tpm2_load -C prim.ctx -u key.pub -r key.priv -c key.ctx -P "session:session.ctx"
tpm2_flushcontext session.ctx
## Attempt loading another key that was not recorded in policycphash
setup_authorized_policycphash
trap - ERR
tpm2_load -C prim.ctx -u key_2.pub -r key_2.priv -c key_2.ctx \
-P "session:session.ctx"
if [ $? == 0 ];then
  echo "ERROR: tpm2_load must fail!"
  exit 1
fi
trap onerror ERR
tpm2_flushcontext session.ctx

# Test tpm2_activatecredential
create_authorized_policy
tpm2_createprimary -C o -c prim.ctx -G rsa
tpm2_readpublic -c prim.ctx -o prim.pub
tpm2_create -C prim.ctx -u key.pub -r key.priv -c key.ctx -L authorized.policy
tpm2_readpublic -c key.ctx -n key.name
echo "plaintext" > plain.txt
tpm2_makecredential -e prim.pub  -s plain.txt -n `xxd -p -c 34 key.name` \
-o cred.secret
tpm2_activatecredential -c key.ctx -C prim.ctx -i cred.secret -o act_cred.secret \
--cphash cp.hash
tpm2_startauthsession -S session.ctx -g sha256
tpm2_policycphash -S session.ctx -L policy.cphash --cphash cp.hash
tpm2_policycommandcode -S session.ctx TPM2_CC_ActivateCredential -L policy.cphash
tpm2_flushcontext session.ctx
sign_and_verify_policycphash
tpm2_startauthsession -S session.ctx --policy-session -g sha256
tpm2_policycphash -S session.ctx --cphash cp.hash
tpm2_policycommandcode -S session.ctx TPM2_CC_ActivateCredential
tpm2_policyauthorize -S session.ctx -i policy.cphash -n signing_key.name \
-t verification.tkt
tpm2_activatecredential -c key.ctx -C prim.ctx -i cred.secret -o act_cred.secret \
-p "session:session.ctx"
tpm2_flushcontext session.ctx

# Test tpm2_unseal
create_authorized_policy
tpm2_createprimary -C o -c prim.ctx
echo "plaintext" | \
tpm2_create -C prim.ctx -c key.ctx -u key.pub -r key.priv -L authorized.policy -i-
tpm2_unseal -c key.ctx --cphash cp.hash
generate_policycphash
sign_and_verify_policycphash
setup_authorized_policycphash
tpm2_unseal -c key.ctx -p "session:session.ctx"
tpm2_flushcontext session.ctx

exit 0
