#include <stdbool.h>

#include <sapi/tpm20.h>

#include "log.h"
#include "tpm2_password_util.h"
#include "tpm2_util.h"

#define PASSWORD_MAX BUFFER_SIZE(TPM2B_AUTH, buffer)

bool tpm2_password_util_fromhex(TPM2B_AUTH *password, bool is_hex, const char *description,
        TPM2B_AUTH *auth) {

    if (is_hex) {
        auth->t.size = BUFFER_SIZE(TPM2B_AUTH, buffer);
        /* this routine is safe on overlapping memory areas */
        if (tpm2_util_hex_to_byte_structure((char *)password->t.buffer, &auth->t.size, auth->t.buffer)
                != 0) {
            LOG_ERR("Failed to convert hex format password for %s.",
                    description);
            return false;
        }
        /*
         * we only claim sanity on same memory, not overlapping, but well use
         * memove anyways at the expense of speed.
         */
    } else if (password != auth) {
        memmove(auth, password, sizeof(*auth));
    }
    return true;
}

bool tpm2_password_util_copy_password(const char *password, const char *description, TPM2B_AUTH *dest) {

    if (!password) {
        LOG_ERR("Please input the %s password!", description);
        return false;
    }

    if (!dest || !description) {
        return false;
    }

    size_t len = strlen(password);
    if (len >= PASSWORD_MAX) {
        LOG_ERR("Over-length password for %s. Got %zu expected less than %zu!", description, len, PASSWORD_MAX);
        return false;
    }

    dest->t.size = len;
    snprintf((char *)dest->t.buffer, PASSWORD_MAX, "%s", password);
    return true;
}
