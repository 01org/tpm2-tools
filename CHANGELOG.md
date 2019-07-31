## Changelog
### next
  * tpm2_quote: -f becomes -o.
  * tpm2_quote: -C and -P becomes -c and -p respectively.
  * tpm2_quote: -g becomes optional.
  * tpm2_print: -i becomes an argument.
  * tpm2_pcrread: renamed from tpm2_pcrlist.
  * tpm2_pcrlist: -gls go away, -g and -l become a single argument.
  * tpm2_pcrevent: -x becomes an argument.
  * tpm2_nvwrite: short option changed to argument
    -x used to specify nv index is now an argument
  * tpm2_nvwrite: input file changed from argument to option
    -i or --input is now the method to specify file data to write
  * tpm2_nvlist: Renaming the tool
    - tpm2_nvlist is now tpm2_nvreadpublic
  * tpm2_nvundefine: short option changed to argument
    -x used to specify nv index is now an argument
  * tpm2_nvrelease: Renaming the tool
    - tpm2_nvrelease is now tpm2_nvundefine
  * tpm2_nvreadlock: short option changed to argument
    -x used to specify nv index is now an argument
  * tpm2_nvread: short option changed to argument
    -x used to specify nv index is now an argument
  * tpm2_nvincrement: short option changed to argument
    -x used to specify nv index is now an argument
  * tpm2_nvdefine: short option changed to argument
    -x used to specify nv index is now an argument
  * tpm2_hierarchycontrol: new tool added for enabling or disabling the use
    of a hierarchy and its associated NV storage.
  * tpm2_quote: long option changed
    qualification-data is now qualification
  * tpm2_policyauthorize: short and long options changed.
    policy-output is now policy.
    input is now the option for specifying the policy to authorize.
    qualification-data is now qualification.
    -o is now -L.
    -L is now -i.
  * tpm2_import: short option changed
    object-attributes is now attributes
  * tpm2_load: short option changed
    -o is now -c
  * tpm2_loadexternal: short and long options changed
    object-attributes is now attributes
    -o is now -c
  * tpm2_create: short and long options changed
    object-attributes is now attributes
    -o is now -c
  * tpm2_createprimary: short and long options changed
    object-attributes is now attributes
    -o is now -c
  * tpm2_print: long option changed
    in-file is now input
  * tpm2_checkquote: short and long options changed
    halg is now hash-algorithm
    pcr-input-file is now pcr
    pubfile is now public
    qualify-data is now qualification
    -f is now -F
    -F is now -f
  * tpm2_hash:
    - add --hex for specifying hex output.
    - default output of hash to stdout.
    - default output of hash to binary.
    - remove output of ticket to stdout.
  * tpm2_listpersistent: deleted as tpm2_getcap and tpm2_readpublic can be used instead.
  * tpm2_hmac:
      - add -g option for specifying hash algorithm.
      - add -t option for specifying ticket result.
  * tpm2_getrandom:
      - change default output to binary.
      - add --hex option for output to hex format.
  * tpm2_verifysignature: short and long options changed
    halg is now hash-algorithm
    sig is now signature
    -D is now -d
  * tpm2_unseal: long options changed
    auth-key is now auth
    out-file is now output
    context-object is now object-context
  * tpm2_startauthsession: short and long options changed
    halg is now hash-algorithm
    key is now key-context
    -k is now -c
  * tpm2_sign: short and long options changed
    auth-key is now auth
    halg is now hash-algorithm
    sig-scheme is now scheme
    out-sig is now signature
    -D is now -d
  * tpm2_send: long option changed
    out-file is now output
  * tpm2_rsaencrypt: long option changed
    out-file is now output
  * tpm2_rsadecrypt: long options changed
    auth-key is now auth
    in-file is now input
    out-file is now output
  * tpm2_readpublic: long options changed
    out-file is now output
    context is now object-context
  * tpm2_quote: short and long options changed
    auth-ak is now ak-auth
    id-list is now pcr-index
    sel-list is now pcr-list
    qualify-data is now qualification-data
    pcrs is now pcr
    halg is now hash-signature
    -l is now -i
    -L is now -l
    -p is now -f
    -f is now -F
  * tpm2_policysecret: short and long options changed
    context is now object-context
    out-policy-file is now policy
    -o is now -L
  * tpm2_policypcr: short and long options changed
    out-policy-file is now policy
    pcr-input-file is now pcr
    set-list is now pcr-list
    -o is now -L
    -F is now -f
    -L is now -l
  * tpm2_policypassword: short and long options changed
    out-policy-file is now policy
    -o is now -L
  * tpm2_policyor: short and long options changed
    out-policy-file is now policy
    -o is now -L
    -L is now -l
  * tpm2_policylocality: short and long options changed
    out-policy-file is now policy
    -o is now -L
  * tpm2_policyduplicationselect: short and long options changed
    new-parent-name is now parent-name
    out-policy-file is now policy
    -o is now -L
  * tpm2_policycommandcode: short and long options changed
    out-policy-file is now policy
    -o is now -L
  * tpm2_policyauthorize: short and long options changed
    out-policy-file is now policy-output
    in-policy-file is now policy
    qualify-data is now qualification-data
    -i is now -L
  * tpm2_pcrlist: short and long options changed
    halg is now hash-algorithm
    out-file is now output
    algs is now pcr-algorithms
    sel-list is now pcr-list
    -L is now -l
  * tpm2_pcrevent: long options changed
    auth-pcr is now auth
  * tpm2_pcrallocate: long option changed
    auth-platform is now auth
  * tpm2_nvwrite: short and long options changed
    auth-hierarchy is now auth
    -a is now -C
  * tpm2_nvrelease: short and long options changed
    auth-hierarchy is now auth
    -a is now -C
  * tpm2_nvreadlock: short and long options changed
    auth-hierarchy is now auth
    -a is now -C
  * tpm2_nvread: short and long options changed
    out-file is now output
    auth-hierarchy is now auth
    -a is now -C
  * tpm2_nvincrement: long option changed
    auth-hierarchy is now auth
  * tpm2_nvdefine: long options changed
    auth-hierarchy is now hierarchy-auth
    auth-index is now index-auth
    policy-file is now policy
  * tpm2_makecredential: long options changed
    enckey is now encryption-key
    out-file is now output
  * tpm2_loadexternal: long options changed
    key-alg is now key-algorithm
    pubfile is now public
    privfile is now private
    auth-key is now auth
    policy-file is now policy
    halg is now hash-algorithm
    out-context is now key-context
  * tpm2_load: long options changed
    context-parent is now parent-context
    auth-parent is now auth
    pubfile is now public
    privfile is now private
    out-context is now key-context
  * tpm2_import: long options changed
    auth-parent is now parent-auth
    auth-key is now key-auth
    algorithm is now key-algorithm
    in-file is now input
    parent-key is now parent-context
    privfile is now private
    pubfile is now public
    halg is now hash-algorithm
    policy-file is now policy
    sym-alg-file is now encryption-key
  * tpm2_hmac: short and long options changed
    out-file is now output
    auth-key is now auth
    -C is now -c
    -P is now -p
  * tpm2_hash: short and long options changed
    halg is now hash-algorithm
    out-file is now output
    -a is now -C
  * tpm2_getrandom: long option changed
    out-file is now output
  * tpm2_getmanufec: short and long options changed
    auth-endorse is now eh-auth
    auth-owner is now owner-auth
    auth-ek is now ek-auth
    handle is now persistent-handle
    algorithm is now ek-algorithm
    -e is now -P
    -P is now -p
  * tpm2_evictcontrol: long options changed
    auth-hierarchy is now auth
    context is now key-context
  * tpm2_encryptdecrypt: long options changed
    auth-key is now auth
    in-file is now input
    out-file is now output
  * tpm2_duplicate: long options changed
    auth-key is now auth
    inner-wrapper-alg is now wrapper-algorithm
    input-key-file is now encryptionkey-in
    output-key-file is now encryptionkey-out
    parent-key is now parent-context
    context is now key-context
  * tpm2_dictionarylockout: long option changed
    auth-lockout is now auth
  * tpm2_createprimary: long options changed
    auth-hierarchy is now hierarchy-auth
    auth-object is now key-auth
    halg is now hash-algorithm
    kalg is now key-algorithm
    context-object is now key-context
    policy-file is now policy
  * tpm2_certify: long option changed
    halg is now hash-algorithm
  * tpm2_createpolicy: long options changed
    out-policy-file is now policy
    policy-digest-alg is now policy-algorithm
    pcr-input-file is now pcr
    -o is now -L
    -L is now -l
    -F is now -f
  * tpm2_createek: short and long options changed
    auth-endorse is now eh-auth
    auth-owner is now owner-auth
    auth-ek is now ek-auth
    file is now public
    context is now ek-context
    algorithm is now key-algorithm
    -e is now -P
    -P is now -p
    -p is now -u
  * tpm2_createak:
    auth-endorse is now eh-auth
    auth-ak is now ak-auth
    halg is now hash-algorithm
    kalg is now key-algorithm
  * tpm2_clearcontrol: Short and long options changed
    auth-handle is now hierarchy
    -c is now -C
  * tpm2_create:
    auth-parent is now parent-auth
    auth-key is now key-auth
    in-file is now sealing-input
    policy-file is now policy
    pubfile is now public
    privfile is now private
    out-context is now key-context
    halg is now hash-algorithm
    kalg is now key-algorithm
  * tpm2_changeauth: --privfile becomes private
  * tpm2_evictcontrol: -p becomes an argument.
  * tpm2_getcap: -c becomes an argument.
  * tpm2_evictcontrol: -a becomes -C.
  * tpm2_encryptdecrypt: -D becomes -d.
  * tpm2_duplicate: short and long options changes to make it consistent with
    similar tools.
      - -g becomes -G
      - \--duplicate-key-private becomes \--private
  * tpm2_nvincrement: short option changed
    -a becomes -C
  * tpm2_createak: short and long options changed to make it consistent with
    similar options in other tools.
    -e becomes -P,
    -P becomes -p,
    -D becomes -g,
    -p becomes -u,
    context becomes ak-context,
    algorithm becomes kalg,
    digest-alg becomes halg,
    privfile becomes private.
  * cmac algorithm support.
  * tpm2_activatecredential: long options have been changed to align with spec language.
    context is now credentialedkey-context.
    key-context is now credentialkey-context.
    auth-key is now credentialedkey-auth.
    auth-endorse is now credentialkey-auth.
    in-file is now credential-secret.
    out-file is now certinfo-data.
  * tpm2_certify: long options have been changed to align with spec language.
    obj-context is now certifiedkey-context.
    key-context is now signingkey-context.
    auth-object is now certifiedkey-auth.
    auth-key is now signingkey-auth.
  * tpm2_changeauth: tool options change to support only one object at a time. Most command line options changed.
  * tpm2_getcap: most instances of value replaced with raw in YAML output.
  * tpm2_getcap: TPM2_PT_MANUFACTURER displays string value and raw value.
  * tpm2_createak: remove -k persistant option. Use tpm2_evictcontrol.
  * tpm2_nvincrement: -L and -F pcr policy options go away, replaced with pcr password minilanguage.
  * tpm2_nvwrite: -L and -F pcr policy options go away, replaced with pcr password minilanguage.
  * tpm2_nvread: -L and -F pcr policy options go away, replaced with pcr password minilanguage.
  * tpm2_unseal: -L and -F pcr policy options go away, replaced with pcr password minilanguage.
  * tpm2_evictcontrol: support serializing ESYS_TR handle to disk.
  * tpm2_readpublic: support serializing ESYS_TR handle to disk.
  * tpm2_startauthsession: support encrypted and bound sessions.
  * tpm2_duplicate: add new tool for duplicating TPM objects.
  * tpm2_createpolicy: Remove small option -a and rename long option to "policy-session"
  * tpm2_startauthsession: Remove small option -a and rename long option to "policy-session"
  * tpm2_startauthsession: Fix long version of policy-digest-alg to halg
  * tpm2_policysecret: Fix long version of policy-file to out-policy-file
  * tpm2_policypcr: Fix long version of policy-file to out-policy-file
  * tpm2_policypcr: Fix -f option to -o
  * tpm2_policypassword: Fix long version of policy-file to out-policy-file
  * tpm2_policyor: Fix long version of policy-file to out-policy-file
  * tpm2_policylocality: Fix long version of policy-file to out-policy-file
  * tpm2_policyduplicationselect: Fix long version of policy-file to out-policy-file
  * tpm2_policyduplicationselect: Fix long version of obj-name to object-name
  * tpm2_policyduplicationselect: Remove small option -i and rename long option to "include-if-exists"
  * tpm2_policycommandcode: Fix long version of policy-file to out-policy-file
  * tpm2_policyauthorize Fix -f option to -i
  * tpm2_pcrlist: Remove unused -f option
  * tpm2_nvwrite: Remove small option for --offset
  * tpm2_nvread: Fix -f option to -o
  * tpm2_makecredential: Fix -enc-key option to -enckey
  * tpm2_createpolicy: Fix -f option to -o
  * tpm2_loadexternal: Remove unused -P option
  * tpm2_encryptdecrypt: Enable back '-t'/'--iv' support
  * tpm2_create : Fix "-A" option to "-b" for attributes
  * tpm2_createprimary : Fix "-A" option to "-b" for attributes
  * tpm2_import : Fix "-A" option to "-b" for attributes
  * tpm2_loadexternal : Fix "-A" option to "-b" for attributes
  * tpm2_nvdefine : Fix "-A" option to "-b" for attributes
  * tpm2_certify: Fix "-a" option to "-o"
  * tpm2_createpolicy: Fix "-P" option to "--policy-pcr"
  * tpm2_pcrevent: Fix "-i" option to "-x"
  * tpm2_createek: Fix "-o" option to "-w"
  * tpm2_createak: Fix "-o" option to "-w"
  * tpm2_nvdefine: Fix "-t" option to "-b"
  * tpm2_import: Fix "-k" option to "-i"
  * tpm2_getmanufec: Fix "-o" option to "-w" and "-f" option to "-o"
  * tpm2_changeauth: Fix "-o/-O" option to "-w/-W"
  * tpm2_checkquote: Fix -G option to -g
  * tpm2_getrandom: bound input request on max hash size per spec, allow -f to override this.
  * tpm_gettestresult: new tool for getting test results.
  * tpm2_pcrallocate: new tool for changing the allocated PCRs of a TPM.
  * tpm2_incrementalselftest: Add tool to test support of specific algorithms.
  * tpm2_verifysignature: Fix -G option to -g
  * tpm2_sign: Fix -G option to -g
  * tpm2_hash: Fix -G option to -g
  * tpm2_quote: Fix -G option to -g
  * tpm2_testparms: new tool for querying tpm for supported algorithms.
  * tpm2_getcap: supports "pcr" option for listing hash algorithms and bank numbers.
  * tpm2_createprimary: add -u for supporting unique field when creating objects.
  * tpm2_policylocality: add tool for creating a policy restricted to a locality.
  * tpm2_policyduplicationselect: add tool for creating a policy to restrict duplication to a new parent.
  * tpm2_selftest: tool for invoking tpm selftest.
  * tpm2_stirrandom: new command for injecting entropy into the TPM.
  * tpm2_readpublic: supports saving the binary name via -n.
  * tpm2_nvincrement: tool for incrememnting NVs configured as counters.
  * tpm2_sign: supports rsapss.
  * tpm2_sign: -s becomes -o and -s is for signing scheme.
  * tpm2_activatecredential: -f becomes -i
  * tpm2_create: -I becomes -i
  * tpm2_encryptdecrypt: -I becomes -i
  * tpm2_rsadecrypt: -I becomes -i
  * tpm2_create: supports TPM command CreateLoaded.
  * Add support for reading authorisation passwords from a file
  * tpm2_createek: now saves a context file for the generated primary's
    handle to disk.
  * tpm2_createak: now saves a context file for the generated primary's
    handle to disk.
  * tpm2_loadexternal: now saves a context file for the generated primary's
    handle to disk.
  * tpm2_load: now saves a context file for the generated primary's
    handle to disk.
  * tpm2_startauthsession: now saves a context file for the generated primary's
    handle to disk.
  * tpm2_createprimary: now saves a context file for the generated primary's
    handle to disk.
  * Ported all tools from SAPI to ESAPI
  * tpm2_loadexternal: support OSSL style -passin argument as --passin for PEM file passwords.
  * tpm2_import: support OSSL style -passin argument as --passin for PEM file passwords.
  * tpm2_readpublic: supports ECC pem and der file generation.
  * tpm2_activatecredential: Option `--endorse-passwd` changes to `--auth-endorse`.
  * tpm2_loadexternal: name output to file and stdout. Changes YAML stdout output.
  * tpm2_loadexternal: ECC Public and Private PEM support.
  * tpm2_loadexternal: AES Public and Private "raw file" support.
  * tpm2_loadexternal: RSA Public and Private PEM support.
  * tpm2_loadexternal: Object Attribute support.
  * tpm2_loadexternal: Object authorization support.
  * tpm2_loadexternal: Default hierarchy changes to the *null* hierarchy.
  * tpm2_verifysignature: stop outputting message hash.
  * tpm2_verifysignature: issues a warning when ticket is specified for a NULL hierarchy.
  * tpm2_verifysignature: make -t optional.
  * tpm2_import: support additional import key types:
    * RSA1024/2048
    * AES128/192/256
  * tpm2_import: -q changes to -u to align with tpm2_loads public/private output arguments.
  * tpm2_import: Supports setting object name algorithm via -g.
  * tpm2_unseal: -P becomes -p
  * tpm2_sign: -P becomes -p
  * tpm2_nvreadlock: long form for -P is now --auth-hierarchy
  * tpm2_rsadecrypt: -P becomes -p
  * tpm2_nvrelease: long-form of -P becomes --auth-hierarchy
  * tpm2_nvdefine: -I becomes -p
  * tpm2_encryptdecrypt: -P becomes -p
  * tpm2_dictionarylockout: -P becomes -p
  * tpm2_createprimary: -K becomes -p
  * tpm2_createak: -E becomes -e
  * tpm2_certify: -k becomes -p
  * tpm2_hash: -g changes to -G
  * tpm2_encryptdecrypt: Support IVs via -t and algorithm modes via -G.
  * tpm2_hmac: drop -g, just use the algorithm associated with the object.
  * tpm2_getmanufec: -g changes to -G
  * tpm2_createek: -g changes to -G
  * tpm2_createak: -g changes to -G
  * tpm2_verifysignature: -g becomes -G
  * tpm2_sign: -g becomes -G
  * tpm2_import: support specifying parent key with a context file,
    --parent-key-handle/-H becomes --parent-key/-C
  * tpm2_nvwrite and tpm2_nvread: when -P is "index" -a is optional and defaults to
    the NV_INDEX value passed to -x.
  * Load TCTI's by SONAME, not raw .so file
  * tpm2_activatecredential: -e becomes -E
  * tpm2_activatecredential: -e becomes -E
  * tpm2_certify: -c and -C are swapped, -k becomes -K
  * tpm2_createprimary: -K becomes -k
  * tpm2_encryptdecrypt: supports input and output to stdin and stdout respectively.
  * tpm2_create: -g/-G become optional options.
  * tpm2_createprimary: -g/-G become optional options.
  * tpm2_verifysignature - Option `-r` changes to `-f` and supports signature format "rsa".
  * tpm2_import - Parent public data option is optional and changes from `-K` to `-U`.
  * tpm2_import - Supports importing external RSA 2048 keys via pem files.
  * tpm2_pcrlist: Option `--algorithm` changes to `--halg`, which is in line with other tools.
  * tpm2_verifysignature: Option `-r` and `--raw` have been removed. This were unused within the tool.
  * tpm2_hmac: Option `--algorithm` changes to `--halg`, which is in line with the manpage.
  * tpm2_makecredential: Option `--sec` changes to `--secret`.
  * tpm2_activatecredential: Option `--Password` changes to `--auth-key`.
  * system tests are now run with make check when --enable-unit is used in configure.
  * tpm2_unseal: Option `--pwdk` changes to `--auth-key`.
  * tpm2_sign: Option `--pwdk` changes to `--auth-key`.
  * tpm2_rsadecrypt: Option `--pwdk` changes to `--auth-key`.
  * tpm2_quote: Option `--ak-passwd` changes to `--auth-ak`
  * tpm2_pcrevent: Option `--passwd` changes to `--auth-pcr`
  * tpm2_nvwrite: Options `--authhandle` and `--handle-passwd`
    changes to `--hierarchy` and `--auth-hierarchy` respectively.
  * tpm2_nvread: Options `--authhandle` and `--handle-passwd`
    changes to `--hierarchy` and `--auth-hierarchy` respectively.
  * tpm2_nvdefine: Options `--authhandle`, `--handle-passwd` and `--index-passwd`
    changes to `--hierarchy`, `--auth-hierarchy` and `--auth-index`
    respectively.
  * tpm2_loadexternal: `-H` changes to `-a` for specifying hierarchy.
  * tpm2_load: Option `--pwdp` changes to `--auth-parent`.
  * tpm2_hmac: Option `--pwdk` changes to `--auth-key`.
  * tpm2_hash: `-H` changes to `-a` for specifying hierarchy.
  * tpm2_getmanufec: Options `--owner-passwd`, `--endorse-passwd`
  * and `--ek-passwd`change to `--auth-owner`, `--auth-endorse`
    and `--auth-ek` respectively.
  * tpm2_evictcontrol: Option group `-A` and `--auth` changes to `-a` and `--hierarchy`
    Option `--pwda` changes to `--auth-hierarchy`
  * tpm2_encryptdecrypt: Option `--pwdk` changes to `--auth-key`.
  * tpm2_dictionarylockout: Option `--lockout-passwd` changes to `--auth-lockout`
  * tpm2_createprimary: Options `--pwdp` and `--pwdk` change to
    `--auth-hierarchy` and `--auth-object` respectively.
  * tpm2_createek: Options `--owner-passwd`, `--endorse-passwd`
  * and `--ek-passwd`change to `--auth-owner`, `--auth-endorse`
    and `--auth-ek` respectively.
  * tpm2_createak: Options `--owner-passwd`, `--endorse-passwd`
  * and `--ak-passwd`change to `--auth-owner`, `--auth-endorse`
    and `--auth-ak` respectively.
  * tpm2_create: Options `--pwdo` and `--pwdk` change to `--auth-object` and
    `--auth-key` respectively.
  * tpm2_clearlock: Option `--lockout-passwd` changes to `--auth-lockout`
  * tpm2_clear: Option `--lockout-passwd` changes to `--auth-lockout`
  * tpm2_changeauth: Options, `--old-owner-passwd`, `--old-endorse-passwd`,
    and `--old-lockout-passwd` go to `--old-auth-owner`, `--old-auth-endorse`,
    and `--old-auth-lockout` respectively.
  * tpm2_certify: Options `--pwdo` and `--pwdk` change to `--auth-object` and
    `--auth-key` respectively.
  * tpm2_createprimary: `-H` changes to `-a` for specifying hierarchy.
  * tpm2_createak: support for non-persistent AK generation.
  * tpm2_createek: support for non-persistent EK generation.
  * tpm2_getpubak renamed to tpm2_createak, -f becomes -p and -f is used for format of public key
    output.
  * tpm2_getpubek renamed to tpm2_createek, -f becomes -p and -f is used for format of public key
    output.
  * Libre SSL builds fixed.
  * Dynamic TCTIS. Support for pluggable TCTI modules via the -T or --tcti options.
  * tpm2_sign: supports signing a pre-computed hash via -D
  * tpm2_clearlock: tool added
  * test: system testing scripts moved into subordinate test directory.
  * fix a buffer overflow in nvread/write tools.
  * configure: enable code coverage option.
  * tpm2_takeownership: split into tpm2_clear and tpm2_changeauth
  * env: add TPM2TOOLS_ENABLE_ERRATA to control the -Z or errata option.

### 3.0.2 - 2017-12-18
  * configure: enable code coverage option.
  * build: enable silent rules options.
  * Add system tests to dist tarball.
  * tpm2_nv(read|write): fix buffer overflows.

### 3.0.1 - 2017-12-11
  * Makefile: add missing LICENSE and markdown files.
### 3.0 - 2017-12-08
  * tpm2_getmanufec: -O as a flag for -f has changed. -O is for existing EK public structure
      and -f is only for generated EK public output.
  * tpm2_nvlist: output in yaml format.
  * tpm2_makecredential format changes to the -o output file.
  * tpm2-quote: -o option removed.
  * tpm2_rsaencrypt: -I is now an argument and input defaults to stdin. -o is optional and
    defaults to stdout.
  * tpm2_listpersistent: output friendly object attributes.
  * tpm2_createprimary: support friendly object attributes via -A. -H becomes auth
    hierarchy.
  * tpm2_create: support friendly object attributes via -A.
  * tpm2_nvwrite and tpm2_nvread have support for satisfying PCR policies.
  * tpm2_encryptdecrypt: has support for EncryptDecrypt2 command.
  * tpm2_nvwrite: -f option removed, support for stdin data supported. Support for starting
      index to write to.
  * errata framework added for dealing with spec errata.
  * tpm2_quote: -G option for signature hash algorithm specification.
  * tpm2_dump_capability: renamed to tpm2_getcap.
  * tpm2_send_command: renamed to tpm2_send and the input file is now an
    argument vs using -i.
  * tpm2_dump_capability: outputs human readable command codes.
  * camelCase options are now all lower case. For example, --camelCase becomes --camel-case.
  * tpm2_quote,readpublic, and sign now have support for pem/der output/inputs. See the
    respective man pages for more details.
  * tpm2_nvread: Has an output file option, -f.
  * manpages: Are now in Markdown and converted to roff using pandoc.
  * tpm2_create - options 'o' and 'O' changed to 'u' and 'r' respectively.
  * tpm2_pcrlist: support yaml output for parsing.
  * tpm2_pcrevent: new tool for hashing and extending pcrs.
  * Make tpm2_{createprimary,create,load,pcrlist,hmac} tools to support the --quiet option.
  * Support for a --quiet option to suppress messages printed by tools to standard output.
  * tpm2_hmac: support for files greater than 1024 bytes, changes in options and arguments.
  * tpm2_hash: support for files greater than 1024 bytes, changes in options and arguments.
  * Install is now to bin vs sbin. Ensure that sbin tools get removed!
  * make dist and distcheck are now working.
  * installation into customized locations are now working, see issue #402 for details.
  * tpm2_pcrlist: renamed from tpm2_listpcrs.
  * tpm2_pcrextend: new tool for extending PCRs.
  * tpm2_getmanufec: -E option no longer required, defaults to stdout.
  * tpm2_nvlist: Support for friendly nv attributes in output.
  * Support for friendly algorithm names for algorithm identifiers.
  * tpm2_nvread: The option, -s, or size option is no longer required.
  * tpm2_nvwrite: fixed to write files larger than 1024 in size.
  * tpm2_nvread: fixed to read files larger than 1024 in size.
  * tpm2_nvdefine supports "nice-names" for nv space attributes.
  * Support using PCR Policy directly with tpm2_unseal tool.
  * Support PCR policy creation in tpm2_createpolicy
  * Support using a policy session as input to tools that may need to satisfy complex policies
    other than password.
  * tpm2_unseal: supports output to stdoud.
  * tpm2_create: enforce policy based authorization.
  * tpm2_createprimary: add ability to create objects with policy based authorization.
  * tpm2_nvdefine: add ability to create nv indexes with policy based authorization.
  * Support Clang Build.
  * tpm2_unseal test uses endorsement hierarchy as platform hierarchy is unavailable on a
    real tpm.
  * Numerous cleanups and minor bug fixes.

### v2.0 - 2017-03-29

  * Tracked on the milestone: https://github.com/tpm2-software/tpm2-tools/milestone/2
  * Reworked all the tools to support configurable TCTIs, based on build time
    configuration, one can specify the tcti via the --tcti (-T) option to all
    tools.
  * tpm2_getrandom interface made -s a positional argument.
  * Numerous bug fixes.

### v1.1 - 2016-11-04

  * travis ci support.
  * Allow for unit tests to be enabled selectively.
  * tpm2_rc_decode tool: Decode TPM_RC error codes.
  * Android Make file
  * tpm2_listpersistent: list all persistent objects
  * test scripts for tpm2-tools
  * tpm2_nvreadlock
  * tpm2_getmanufec: retrieve EC from tpm manufacturer server.
  * Copy 'common' and 'sample' code from the TPM2.0-TSS repo.

  **Modified**

  * tpm2_takeownership: update option -c to use lockout password to clear.
  * tpm2_listpcrs: add options -L and -s, rewrite to increase performance.
  * tpm2_quote: added -L option to support selection of multiple banks.
  * tpm2_quote: add -q option to get qualifying data.
  * configure: Use pkg-config to get info about libcurl and libcrypto.
  * configure: Use pkg-config to locate SAPI and TCTI headers / libraries.
  * tpm2_x: Add -X option to enable password input in Hex format.
  * tpm2_nvdefine: Change -X option to -I.
  * tpm2-nvwrite: fix for unable to write 1024B+ data.
  * tpm2_getmanufec: Fix base64 encoding.
  * tpm2_x: fixed a lot of TPM2B failures caused by wrong initialization.
  * tpm2_getmanufec: let configure handle libs.
  * tpm2_getmanufec: Convert from dos to unix format.
  * build: Check for TSS2 library @ configure time.
  * build: Detect required TSS2 and TCTI headers.
  * build: Use libtool to build the common library
  * build: Install all binaries into sbin.
  * build: Build common sources into library.
  * build: Move all source files to 'src'.
  * Makefile.am: Move all build rules into single Makefile.am.
  * everything: Use new TCTI headers and fixup API calls.
  * everything: Update source to cope with sapi header cleanup.
  * tpm2_activatecredential: Updated to support TCG compatible EK
  * tpm2_getpubak: Updated to use TCG compatible EK
  * tpm2_getpubek: fix ek creation to follow TCG EK profile spec.

  **Removed**

  * Windows related code
  * dependency on the TPM2.0-TSS repo source code

### v1.0 - 2015-10-19

  * 1.0 release
  * 29 tools included
