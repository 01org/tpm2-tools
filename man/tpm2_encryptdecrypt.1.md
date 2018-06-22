% tpm2_encryptdecrypt(1) tpm2-tools | General Commands Manual
%
% SEPTEMBER 2017

# NAME

**tpm2_encryptdecrypt**(1) - performs symmetric encryption or decryption.

# SYNOPSIS

**tpm2_encryptdecrypt** [*OPTIONS*]

# DESCRIPTION

tpm2_encryptdecrypt(1) - performs symmetric encryption or decryption with a
specified symmetric key.

# OPTIONS

  * **-c**, **--key-context**=_KEY\_CONTEXT\_OBJECT_:

    Name of the key context object to be used for the  operation. Either a file
    or a handle number. See section "Context Object Format".

  * **-P**, **--auth-key**=_KEY\_AUTH_:

    Optional authorization value to use the key specified by **-k**.
    Authorization values should follow the "authorization formatting standards",
    see section "Authorization Formatting".

  * **-D**, **--decrypt**:

    Perform a decrypt operation. Default is encryption.

  * **-I**, **--in-file**=_INPUT\_FILE_:

    Optional. Specifies the input file path for either the encrypted or decrypted
    data, depending on option **-D**. If not specified, defaults to **stdin**.

  * **-S**, **--session**=_SESSION\_FILE_:

    Optional, a session file from **tpm2_startauthsession**(1)'s **-S** option. This session
    is used in lieu of starting a session and using the PCR policy options.

  * **-o**, **--out-file**=_OUT\_FILE_:

    Optional. Specifies the output file path for either the encrypted or decrypted
    data, depending on option **-D**. If not specified, defaults to **stdout**.

  * **-G**, **--mode**=_CIPHER\_MODE_ALGORITHM_:

    The key algorithm associated with this object. It defaults to the objects
    mode or CFB if the objects mode is not configured.

    It accepts friendly names just like -g option.
    See section "Supported Public Object Algorithms" for a list
    of supported object algorithms.

  * **-i**, **--iv**=_IV\_INPUT\_FILE_ : [ _IV\_OUTPUT\_FILE_ ]:

  Optional. The initialization vector to use. Defaults to 0's. The specification
  syntax allows for an input file and output file source to be specified. The input file
  path is first, optionally followed by a colon ":" and the output iv path. This the output
  iv can be saved for subsequent calls when chaining.

[common options](common/options.md)

[common tcti options](common/tcti.md)

[context object format](commmon/ctxobj.md)

[authorization formatting](common/password.md)

# EXAMPLES

```
tpm2_encryptdecrypt -C 0x81010001 -P abc123 -I <filePath> -o <filePath>
tpm2_encryptdecrypt -C key.dat -P abc123 -I <filePath> -o <filePath>
tpm2_encryptdecrypt -C 0x81010001 -P 123abca -X -I <filePath> -o <filePath>
```

# RETURNS

0 on success or 1 on failure.

[footer](common/footer.md)
