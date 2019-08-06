% tpm2_createprimary(1) tpm2-tools | General Commands Manual

# NAME

**tpm2_createprimary**(1) - Create a primary key.

# SYNOPSIS

**tpm2_createprimary** [*OPTIONS*]

# DESCRIPTION

**tpm2_createprimary**(1) - This command is used to create a primary object under
one of the hierarchies: Owner, Platform, Endorsement, NULL. The command will create
and load a Primary Object. The sensitive and public portions are not returned.
A context file for the created object's handle is saved as a file for future interactions
with the created primary.

# OPTIONS

  * **-C**, **\--hierarchy**=_HIERARCHY_:

    Specify the hierarchy under which the object is created. This will also
    dictate which authorization secret (if any) must be supplied. Defaults to
    **o**, **TPM_RH_OWNER**, when no value specified.
    Supported options are:
      * **o** for **TPM_RH_OWNER**
      * **p** for **TPM_RH_PLATFORM**
      * **e** for **TPM_RH_ENDORSEMENT**
      * **n** for **TPM_RH_NULL**
      * **`<num>`** where a raw number can be used.

  * **-P**, **\--hierarchy-auth**=_HIERARCHY\_\_AUTH\_VALUE_:

    Optional authorization value when authorization is required to create object
    under the specified hierarchy given via the **-a** option. Authorization
    values should follow the "authorization formatting standards", see section
    "Authorization Formatting".

  * **-p**, **\--key-auth**=_OBJECT\_AUTH_:

    Optional authorization password for the newly created object. Password
    values should follow the "authorization formatting standards", see section
    "Authorization Formatting".

  * **-g**, **\--hash-algorithm**=_ALGORITHM_:

    The hash algorithm to use for generating the objects name.
    If not specified, the default name algorithm is SHA256.
    Algorithms should follow the "formatting standards", see section
    "Algorithm Specifiers". Also, see section
    "Supported Hash Algorithms" for a list of supported hash algorithms.

  * **-G**, **\--key-algorithm**=_KEY\_ALGORITHM_:

    Algorithm type for generated key. If not specified, the default key
    algorithm is rsa2048:null:aes128cfb. See section "Supported Public Object Algorithms"
    for a list of supported object algorithms.

  * **-c**, **\--key-context**=_CONTEXT\_FILE\_NAME_:

    File name to use for the returned object context, required.

  * **-L**, **\--policy**=_POLICY\_FILE_:

    An optional file input that contains the policy digest for policy based authorization of the object.

  * **-a**, **\--attributes**=_ATTRIBUTES_:

    The object attributes, optional. Object attributes follow the specifications
    as outlined in "object attribute specifiers". The default for created objects is:

    `TPMA_OBJECT_RESTRICTED|TPMA_OBJECT_DECRYPT|TPMA_OBJECT_FIXEDTPM|TPMA_OBJECT_FIXEDPARENT|TPMA_OBJECT_SENSITIVEDATAORIGIN|TPMA_OBJECT_USERWITHAUTH`

  * **-u**, **\--unique-data**=_UNIQUE\_FILE_:

    An optional file input that contains the binary bits of a **TPMU_PUBLIC_ID** union where
    numbers (such as length words) are in little-endian format. This is passed in the
    unique field of **TPMT_PUBLIC**.

[common options](common/options.md)

[common tcti options](common/tcti.md)

[authorization formatting](common/authorizations.md)

[supported hash algorithms](common/hash.md)

[supported public object algorithms](common/object-alg.md)

[algorithm specifiers](common/alg.md)

# EXAMPLES

## Create an ECC primary object
```bash
tpm2_createprimary -C o -g sha256 -G ecc -c context.out
```

## Create a primary object that follows the guidance of TCG Provisioning guide

See : https://trustedcomputinggroup.org/wp-content/uploads/TCG-TPM-v2.0-Provisioning-Guidance-Published-v1r1.pdf

Where unique.dat contains the binary-formatted data: 0x00 0x01 (0x00 * 256)

```bash
tpm2_createprimary -C o -G rsa2048:aes128cfb -g sha256 -c prim.ctx \
  -a 'restricted|decrypt|fixedtpm|fixedparent|sensitivedataorigin|userwithauth|noda' \
  -u unique.dat
```


[returns](common/returns.md)

[footer](common/footer.md)
