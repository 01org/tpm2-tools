% tpm2_rsaencrypt(1) tpm2-tools | General Commands Manual

# NAME

**tpm2_rsaencrypt**(1) - Performs an RSA encryption operation using the TPM.

# SYNOPSIS

**tpm2_rsaencrypt** [*OPTIONS*] _FILE_

# DESCRIPTION

**tpm2_rsaencrypt**(1) - Performs RSA encryption on the contents of _FILE_
(defaulting to stdin) using the indicated padding scheme according to
IETF RFC 3447 (PKCS#1). The scheme of keyHandle should not be **TPM_ALG_NULL**.

The key referenced by key-context is **required** to be:

1. An RSA key
2. Have the attribute *decrypt* **SET** in it's attributes.

# OPTIONS

  * **-c**, **\--key-context**=_KEY\_CONTEXT\_OBJECT_:

    Context object pointing to the the public portion of RSA key to use for
    encryption. Either a file or a handle number.
    See section "Context Object Format".

  * **-o**, **\--output**=_OUTPUT\_FILE_:

    Optional output file path to record the decrypted data to. The default is to print
    the binary encrypted data to stdout.

  * **-g**, **\--scheme**=_PADDING\_SCHEME_:

    Optional, set the padding scheme (defaults to rsaes).

    * null  - TPM_ALG_NULL
    * rsaes - TPM_ALG_RSAES
    * oaep  - TPM_ALG_OAEP

[common options](common/options.md)

[common tcti options](common/tcti.md)

[context object format](common/ctxobj.md)

# EXAMPLES

```
tpm2_rsaencrypt -C 0x81010001 -o encrypted.out
```

[returns](common/returns.md)

[footer](common/footer.md)
