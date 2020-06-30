#!/bin/bash
set -e
source helpers.sh

start_up

setup_fapi

function cleanup {
    tss2 delete --path=/
    shut_down
}

trap cleanup EXIT

PLAIN_TEXT=$TEMP_DIR/plaintext.file
KEY_PATH="HS/SRK/myRSACrypt"
ENCRYPTED_FILE=$TEMP_DIR/encrypted.file
DECRYPTED_FILE=$TEMP_DIR/decrypted.file
PCR_POLICY_DATA=$TEMP_DIR/pol_pcr16_0.json
POLICY_PCR=policy/pcr-policy
TYPES="noDa,decrypt"

echo -n "Secret Text!" > $PLAIN_TEXT

set -x

tss2 provision

expect <<EOF
# Try interactive prompt with 2 different passwords
spawn tss2 createkey --path=$KEY_PATH --type=$TYPES
expect "Authorize object Password: "
send "1\r"
expect "Authorize object Retype password: "
send "2\r"
expect {
    "Passwords do not match." {
            } eof {
                send_user "Expected password mismatch, but got nothing, or
                rather EOF\n"
                exit 1
            }
        }
        set ret [wait]
        if {[lindex \$ret 2] || [lindex \$ret 3] != 1} {
            send_user "Using interactive prompt with different passwords
            has not failed\n"
            exit 1
        }
EOF

expect <<EOF
# Try with missing path
spawn tss2 createkey --authValue=abc --type="noDa, decrypt"
set ret [wait]
if {[lindex \$ret 2] || [lindex \$ret 3] != 1} {
    send_user "Command has not failed as expected\n"
    exit 1
}
EOF

tss2 import --path=$POLICY_PCR --importData=$PCR_POLICY_DATA

expect <<EOF
# Try interactive prompt with empty passwords
spawn tss2 createkey --path=$KEY_PATH --type=$TYPES
expect "Authorize object Password: "
send "\r"
expect "Authorize object Retype password: "
send "\r"
set ret [wait]
if {[lindex \$ret 2] || [lindex \$ret 3] != 0} {
    send_user "Using interactive prompt with null password
    has failed\n"
    exit 1
}
EOF

tss2 encrypt --keyPath=$KEY_PATH --plainText=$PLAIN_TEXT \
    --cipherText=$ENCRYPTED_FILE --force

expect <<EOF
# Try with missing keypath
spawn tss2 encrypt --plainText=$PLAIN_TEXT --cipherText=$ENCRYPTED_FILE
set ret [wait]
if {[lindex \$ret 2] || [lindex \$ret 3] != 1} {
    send_user "Command has not failed as expected\n"
    exit 1
}
EOF

expect <<EOF
# Try with missing plaintext
spawn tss2 encrypt --keyPath=$KEY_PATH --cipherText=$ENCRYPTED_FILE
set ret [wait]
if {[lindex \$ret 2] || [lindex \$ret 3] != 1} {
    send_user "Command has not failed as expected\n"
    exit 1
}
EOF

expect <<EOF
# Try with missing ciphertext
spawn tss2 encrypt --keyPath=$KEY_PATH --plainText=$PLAIN_TEXT
set ret [wait]
if {[lindex \$ret 2] || [lindex \$ret 3] != 1} {
    send_user "Command has not failed as expected\n"
    exit 1
}
EOF

expect <<EOF
# Try with wrong plaintext file
spawn tss2 encrypt --keyPath=$KEY_PATH --plainText=abc \
    --cipherText=$ENCRYPTED_FILE
set ret [wait]
if {[lindex \$ret 2] || [lindex \$ret 3] != 1} {
    send_user "Command has not failed as expected\n"
    exit 1
}
EOF

expect <<EOF
# Try with missing ciphertext
spawn tss2 decrypt --keyPath=$KEY_PATH --plainText=$DECRYPTED_FILE
set ret [wait]
if {[lindex \$ret 2] || [lindex \$ret 3] != 1} {
    send_user "Command has not failed as expected\n"
    exit 1
}
EOF

expect <<EOF
# Try with missing plaintext
spawn tss2 decrypt --keyPath=$KEY_PATH --cipherText=$ENCRYPTED_FILE
set ret [wait]
if {[lindex \$ret 2] || [lindex \$ret 3] != 1} {
    send_user "Command has not failed as expected\n"
    exit 1
}
EOF

expect <<EOF
# Try with missing keyPath
spawn tss2 decrypt --cipherText=$ENCRYPTED_FILE --plainText=$DECRYPTED_FILE
set ret [wait]
if {[lindex \$ret 2] || [lindex \$ret 3] != 1} {
    send_user "Command has not failed as expected\n"
    exit 1
}
EOF

tss2 decrypt --keyPath=$KEY_PATH --cipherText=$ENCRYPTED_FILE \
    --plainText=$DECRYPTED_FILE --force


if [ "`cat $DECRYPTED_FILE`" != "`cat $PLAIN_TEXT`" ]; then
  echo "Encryption/Decryption failed"
  exit 1
fi

tss2 delete --path=$KEY_PATH

# Encrypt/Decrypt with password
tss2 createkey --path=$KEY_PATH --type="noDa, decrypt" --authValue=abc
tss2 encrypt --keyPath=$KEY_PATH --plainText=$PLAIN_TEXT \
    --cipherText=$ENCRYPTED_FILE --force
echo -n "Fail" > $DECRYPTED_FILE
expect <<EOF
spawn tss2 decrypt --keyPath=$KEY_PATH --cipherText=$ENCRYPTED_FILE \
    --plainText=$DECRYPTED_FILE --force
expect "Authorize object : "
send "abc\r"
set ret [wait]
if {[lindex \$ret 2] || [lindex \$ret 3] != 0} {
    send_user "Authorization failed\n"
    exit 1
}
EOF

if [ "`cat $DECRYPTED_FILE`" != "`cat $PLAIN_TEXT`" ]; then
  echo "Encryption/Decryption failed"
  exit 1
fi

# Try tss2 createkey with missing type. This only works for tpm2-tss >=2.4.2.
# Therefore, make the test conditional
VERSION="$(tss2 createkey -v | grep -Po 'fapi-version=.*' | grep -Eo '([0-9]+\.{1})+[0-9]' | sed 's/[^0-9]*//g')"
if [ $VERSION -ge "242" ]; then
    tss2 delete --path=$KEY_PATH
    tss2 createkey --path=$KEY_PATH --authValue=abc
fi

exit 0
