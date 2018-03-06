#include "constants.h"
coding_signature_t get_signature()
{
    coding_signature_t signature;
    signature.symbol = SYMBOL_DIRECT;
    signature.header = HEADER_BLOCK;
    signature.ans = ANS_RANGE;
    return signature;
}