#include "constants.h"
coding_signature_t get_signature()
{
    coding_signature_t signature;
    signature.symbol = SYMBOL_DIRECT;
    signature.header = HEADER_SINGLE;
    signature.ans = ANS_RANGE;
    signature.bit_factor = DEFAULT_BIT_FACTOR;
    return signature;
}