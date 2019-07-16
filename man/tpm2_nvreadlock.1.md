% tpm2_nvreadlock(1) tpm2-tools | General Commands Manual

# NAME

**tpm2_nvreadlock**(1) - Lock the Non-Volatile (NV) index for further reads.

# SYNOPSIS

**tpm2_nvreadlock** [*OPTIONS*]

# DESCRIPTION

**tpm2_nvreadlock**(1) - Lock the Non-Volatile (NV) index for further reads. The lock on the NV
index is unlocked when the TPM is restarted and the NV index becomes readable again.

# OPTIONS

  * **-x**, **\--index**=_NV\_INDEX_:

    Specifies the index to define the space at.

  * **-a**, **\--hierarchy**=_AUTH_:

    Specifies the hierarchy used to authorize:
    * **o** for **TPM_RH_OWNER**
    * **p** for **TPM_RH_PLATFORM**
    Defaults to **o**, **TPM_RH_OWNER**, when no value has been
    specified.
    * **`<num>`** where a hierarchy handle may be used.

  * **-P**, **\--auth-hierarchy**=_AUTH\_HIERARCHY\_VALUE_:

    Specifies the authorization value for the hierarchy. Authorization values
    should follow the "authorization formatting standards", see section
    "Authorization Formatting".

[common options](common/options.md)

[common tcti options](common/tcti.md)

[authorization formatting](common/authorizations.md)

# EXAMPLES

## Lock an index protected by a password
```
tpm2_nvreadlock -x 0x1500016 -a 0x40000001 -P passwd
```

[returns](common/returns.md)

[footer](common/footer.md)
