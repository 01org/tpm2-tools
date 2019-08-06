% tpm2_sign(1) tpm2-tools | General Commands Manual

# NAME

**tpm2_sign**(1) - Sign a hash using the TPM.

# SYNOPSIS

**tpm2_sign** [*OPTIONS*] _FILE_

# DESCRIPTION

**tpm2_sign**(1) - Signs an externally provided message or hash with the
specified symmetric or asymmetric signing key.

If the signing key is a restricted signing key, then validation can be provided
via the **-t** output. The ticket indicates that the TPM performed the hash of
the message.

# OPTIONS

  * **-c**, **\--key-context**=_KEY\_CONTEXT\_OBJECT_:

    Context object pointing to the the key used for signing. Either a file or a
    handle number. See section "Context Object Format".

  * **-p**, **\--auth**=_KEY\_AUTH_:

    Optional authorization value to use the key specified by **-c**.
    Authorization values should follow the "authorization formatting standards",
    see section "Authorization Formatting".

  * **-g**, **\--hash-algorithm**=_HASH\_ALGORITHM_:

    The hash algorithm used to digest the message.
    Algorithms should follow the "formatting standards", see section
    "Algorithm Specifiers".
    Also, see section "Supported Hash Algorithms" for a list of supported hash
    algorithms.

  * **-s**, **\--scheme**=_SIGNING\_SCHEME_:

    The signing scheme used to sign the message. Optional.
    Signing schemes should follow the "formatting standards", see section
     "Algorithm Specifiers".
    Also, see section "Supported Signing Schemes" for a list of supported
     signature schemes.
    If specified, the signature scheme must match the key type.
    If left unspecified, a default signature scheme for the key type will
     be used.

  * **-d**, **\--digest**

    Indicate that _FILE_ is a file containing the digest of the message.
    When this option and **-t** is specified, a warning is
    generated and the **validation ticket (-t) is ignored**.
    You cannot use this option to sign a digest against a restricted
    signing key.

  * **-t**, **\--ticket**=_TICKET\_FILE_:

    The ticket file, containing the validation structure, optional.

  * **-o**, **\--signature**=_SIGNATURE\_FILE_:

    The signature file, records the signature structure.

  * **-f**, **\--format**

    Format selection for the signature output file. See section "Signature Format Specifiers".

[common options](common/options.md)

[common tcti options](common/tcti.md)

[context object format](common/ctxobj.md)

[authorization formatting](common/authorizations.md)

[supported hash algorithms](common/hash.md)

[supported signing schemes](common/sign-alg.md)

[algorithm specifiers](common/alg.md)

[signature format specifiers](common/signature.md)

# EXAMPLES

## Sign and verify with the TPM using the *endorsement* hierarchy
```bash
tpm2_createprimary -C e -c primary.ctx

tpm2_create -G rsa -u rsa.pub -r rsa.priv -C primary.ctx

tpm2_load -C primary.ctx -u rsa.pub -r rsa.priv -c rsa.ctx

echo "my message" > message.dat

tpm2_sign -c rsa.ctx -g sha256 -o sig.rssa message.dat

tpm2_verifysignature -c rsa.ctx -g sha256 -s sig.rssa -m message.dat
```

## Sign with the TPM and verify with OSSL
```bash
openssl ecparam -name prime256v1 -genkey -noout -out private.ecc.pem

openssl ec -in private.ecc.pem -out public.ecc.pem -pubout

# Generate a hash to sign
echo "data to sign" > data.in.raw

sha256sum data.in.raw | awk '{ print "000000 " $1 }' | xxd -r -c 32 > data.in.digest

# Load the private key for signing
tpm2_loadexternal -Q -G ecc -r private.ecc.pem -c key.ctx

# Sign in the TPM and verify with OSSL
tpm2_sign -Q -c key.ctx -g sha256 -d -f plain -o data.out.signed data.in.digest

openssl dgst -verify public.ecc.pem -keyform pem -sha256 -signature data.out.signed data.in.raw
```

[returns](common/returns.md)

[footer](common/footer.md)
