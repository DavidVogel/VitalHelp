/**
 * @file poly_utils_test.cpp
 * @brief Implements the PolyUtilsTest class, performing tests on poly_float and poly_int utility functions.
 */

#include "poly_utils_test.h"
#include "poly_utils.h"

#define EPSILON 0.0000001f

void PolyUtilsTest::runTest() {
    beginTest("Swap Stereo");
    vital::poly_float val;
    for (int i = 0; i < vital::poly_float::kSize; ++i)
        val.set(i, i);

    const vital::poly_float test_value = val;

    // Test swapping stereo channels: every pair (L,R) should become (R,L).
    vital::poly_float swap_stereo = vital::utils::swapStereo(test_value);
    for (int i = 0; i < vital::poly_float::kSize; i += 2) {
        expect(swap_stereo[i] == i + 1, "Left channel should be swapped with right channel.");
        expect(swap_stereo[i + 1] == i, "Right channel should be swapped with left channel.");
    }

    beginTest("Swap Voices");
    // Test swapping the first half of voices with the second half.
    vital::poly_float swap_voices = vital::utils::swapVoices(test_value);
    for (int i = 0; i < vital::poly_float::kSize / 2; ++i) {
        expect(swap_voices[i] == i + vital::poly_float::kSize / 2, "Voices in first half should swap with second half.");
        expect(swap_voices[i + vital::poly_float::kSize / 2] == i, "Voices in second half should swap with first half.");
    }

    beginTest("Reverse");
    // Test reversing the order of elements.
    vital::poly_float reverse = vital::utils::reverse(test_value);
    for (int i = 0; i < vital::poly_float::kSize; ++i)
        expect(reverse[i] == vital::poly_float::kSize - 1 - i, "Values should be reversed in order.");

    beginTest("Mid Side Encoding");
    // Test mid-side encoding and decoding round trip.
    vital::poly_float encode_mid_side = vital::utils::encodeMidSide(test_value);
    vital::poly_float decode_mid_side = vital::utils::decodeMidSide(encode_mid_side);
    for (int i = 0; i < vital::poly_float::kSize; i += 2)
        expectWithinAbsoluteError<vital::mono_float>(test_value[i], decode_mid_side[i], EPSILON,
                                                     "Mid-side round trip should preserve the signal.");

    beginTest("Mask Load");
    // Test conditional loading with mask operations.
    vital::poly_float one(-1.0f, 2.0f, 1.0f, 10.0f);
    vital::poly_float two(3.0f, 1.0f, -20.0f, 50.0f);
    vital::poly_float combine = vital::utils::maskLoad(one, two, vital::poly_float::greaterThan(two, one));
    expect(combine[0] == 3.0f);
    expect(combine[1] == 2.0f);
    expect(combine[2] == 1.0f);
    expect(combine[3] == 50.0f);

    vital::poly_int int_one(-1, 2, 1, 10);
    vital::poly_int int_two(3, 1, -20, 50);
    vital::poly_int int_combine = vital::utils::maskLoad(int_one, int_two, vital::poly_int::greaterThan(int_two, int_one));
    expect(int_combine[0] == (unsigned int)-1);
    expect(int_combine[1] == 2);
    expect(int_combine[2] == (unsigned int)-20);
    expect(int_combine[3] == 50);
}

// Registers the test instance so it will be automatically discovered and run.
static PolyUtilsTest poly_utils_test;
