/**
 * @file poly_utils_test.h
 * @brief Declares the PolyUtilsTest class, which tests utility functions related to poly_float and poly_int operations.
 */

#pragma once

#include "JuceHeader.h"

/**
 * @class PolyUtilsTest
 * @brief A test class for verifying the functionality of various poly_* utility functions.
 *
 * This test class checks operations such as swapping stereo channels, swapping voice allocations,
 * reversing arrays, mid-side encoding/decoding, and mask-based conditional loading of values.
 * It ensures that these utilities work correctly and produce expected results.
 */
class PolyUtilsTest : public UnitTest {
public:
    /**
     * @brief Constructs a new PolyUtilsTest with a specified test name and category.
     */
    PolyUtilsTest() : UnitTest("Poly Utils", "Utils") { }

    /**
     * @brief Runs all poly utilities tests, verifying that operations on poly_float and poly_int values
     *        behave as expected.
     */
    void runTest() override;
};
