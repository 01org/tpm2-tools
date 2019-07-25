% tpm2_policysecret(1) tpm2-tools | General Commands Manual

# NAME

**tpm2_policysecret**(1) - Enables secret(password/hmac) based authorization to
a policy.

# SYNOPSIS

**tpm2_policysecret** [*OPTIONS*] _AUTH\_VALUE_

# DESCRIPTION

**tpm2_policysecret**(1) - Enables secret (password/hmac) based authorization to a
 policy. The secret is the auth value of any TPM object (NV/Hierarchy/Loaded/
 Persistent).

# OPTIONS

  * **-c**, **\--object-context**=_OBJECT_CONTEXT_:

    A context object specifier of a transient/permanent/persistent object. Either
    a file path of a object context blob or a loaded/persistent/permanent handle
    id. See section "Context Object Format". As an argument, it takes the auth
    value of the associated TPM object, a single dash - can be used to read the
    auth value from stdin. The argument follows the "authorization formatting
    standards", see section "Authorization Formatting".

  * **-S**, **\--session**=_SESSION_FILE_:

    The policy session file generated via the **-S** option to
    **tpm2_startauthsession**(1).

  * **-L**, **\--policy**=_POLICY\_FILE_:

    File to save the policy digest.

[common options](common/options.md)

[common tcti options](common/tcti.md)

[authorization formatting](common/authorizations.md)

# EXAMPLES

Associate auth value of a sealing object to the owner hierarchy password.
* Start a trial auth session and run **tpm2_policysecret**(1) to create policy that
can only be satisfied if owner hierarchy auth value is supplied.
* Start a real policy session and provide the owner hierarchy auth value.
* Provide the session input where in the policysecret for owner hierarchy auth
was satisfied to the unseal tool.
* If the policy was satisfied unsealing should succeed.

## Generate a policy that binds to the secret of the auth object
```
TPM_RH_OWNER=0x40000001

tpm2_startauthsession -S session.ctx

tpm2_policysecret -S session.ctx -c $TPM_RH_OWNER -L secret.policy

tpm2_flushcontext session.ctx
```

## Create a TPM object like a sealing object with the policy
```
tpm2_createprimary -Q -C o -g sha256 -G rsa -c prim.ctx

tpm2_create -Q -g sha256 -u sealing_key.pub -r sealing_key.priv -i- -C prim.ctx -L secret.policy <<< "SEALED-SECRET"

tpm2_load -C prim.ctx -u sealing_key.pub -r sealing_key.priv -n sealing_key.name -c sealing_key.ctx
```

## Satisfy the policy and unseal the secret
```
tpm2_startauthsession \--policy-session -S session.ctx

tpm2_policysecret -S session.ctx -c $TPM_RH_OWNER -L secret.policy

unsealed=`tpm2_unseal -p "session:session.ctx" -c sealing_key.ctx`
echo $unsealed

tpm2_flushcontext session.ctx
```

[returns](common/returns.md)

[footer](common/footer.md)
