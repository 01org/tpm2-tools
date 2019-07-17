% tpm2_createek(1) tpm2-tools | General Commands Manual

# NAME

**tpm2_createek**(1) - Generate TCG profile compliant endorsement key.

# SYNOPSIS

**tpm2_createek** [*OPTIONS*]

# DESCRIPTION

**tpm2_createek**(1) - Generate TCG profile compliant endorsement key (EK), which is the primary object
of the endorsement hierarchy.

If a transient object is generated the tool outputs a context file specified via **-c**.

Refer to:
<http://www.trustedcomputinggroup.org/files/static_page_files/7CAA5687-1A4B-B294-D04080D058E86C5F>

# OPTIONS

  * **-e**, **\--auth-endorse**=_ENDORSE\_AUTH_:

    Specifies current endorsement hierarchy authorization.
    authorizations should follow the "authorization formatting standards", see section
    "Authorization Formatting".

  * **-P**, **\--auth-ek**=_EK\_AUTH_

    Specifies the endorsement key authorization when created.
    Same formatting as the endorse authorization value or **-e** option.

  * **-w**, **\--auth-owner**=_OWNER\_AUTH_

    Specifies the current owner authorization.
    Same formatting as the endorse password value or **-e** option.

  * **-c**, **\--context**=_PERSISTENT\_HANDLE_:

    Specifies the name of a context object used to store the EK, either a path
    to save the context of the EK or a handle used to persist EK in the TPM.

    If a value of **-** is passed the tool will find a vacant persistent handle
    to use and print out the automatically selected handle.

    If one saves the context file via this option and the public key via the
    **-p** option, the EK can be restored via a call to **tpm2_loadexternal**(1).

  * **-G**, **\--algorithm**=_ALGORITHM_:

    Specifies the algorithm type of EK. Supports:
    * **ecc** - An P256 key.
    * **rsa** - An RSA2048 key.
    * **keyedhash** - hmac key.

  * **-p**, **\--file**=_FILE_:

    Optional: specifies the file used to save the public portion of EK. This defaults
    to a binary data structure corresponding to the **TPM2B_PUBLIC** structure in the
    specification. Using the **\--format** option allows one to change this
    behavior.

  * **-t**, **\--template**:

    Optional: Uses the manufacturer defined EK Template and EK Nonce from fixed NV Indices
    to populate the **TPM2B_PUBLIC** public area. See the TCG EK Credential Profile specification
    for more information:
      - https://trustedcomputinggroup.org/wp-content/uploads/TCG_IWG_Credential_Profile_EK_V2.1_R13.pdf


[pubkey options](common/pubkey.md)

[common options](common/options.md)

[common tcti options](common/tcti.md)

[context object format](common/ctxobj.md)

[supported public object algorithms](common/object-alg.md)

[algorithm specifiers](common/alg.md)

# EXAMPLES

### Create an Endorsement Key and make it persistent
```
tpm2_createek -e abc123 -w abc123 -P passwd -c 0x81010001 -G rsa -p ek.pub
```

### Create a transient Endorsement Key, flush it, and reload it.
```
tpm2_createek -G rsa -p ek.pub

# Check that it is loaded in transient memory
tpm2_getcap handles-transient
- 0x80000000

# Flush the handle
tpm2_flushcontext -c 0x80000000

# Note that it is flushed
tpm2_getcap handles-transient
<null output>

# Reload it via loadexternal
tpm2_loadexternal -C o -u ek.pub -o ek.ctx

# Check that it is re-loaded in transient memory
tpm2_getcap handles-transient
- 0x80000000

```

[returns](common/returns.md)

[footer](common/footer.md)
