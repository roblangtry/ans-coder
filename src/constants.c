#include "constants.h"
coding_signature_t get_signature()
{
    coding_signature_t signature;
    signature.symbol = SYMBOL_DIRECT;
    signature.header = HEADER_SINGLE;
    signature.ans = ANS_RANGE;
    signature.bit_factor = DEFAULT_BIT_FACTOR;
    signature.msb_bit_factor = DEFAULT_MSB_BIT_FACTOR;
    signature.translation = TRANSLATE_FALSE;
    signature.translate_k = 10;
    signature.hashing = HASHING_STANDARD;
    return signature;
}