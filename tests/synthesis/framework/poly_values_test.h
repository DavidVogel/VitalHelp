/**
 * @file poly_values_test.h
 * @brief Declares the PolyValuesTest class, which tests the poly_float and poly_int classes and their operations.
 */

#pragma once

#include "JuceHeader.h"

/**
 * @class PolyValuesTest
 * @brief A test class for verifying the functionality and correctness of the poly_float and poly_int vector types.
 *
 * The PolyValuesTest runs through a series of arithmetic and comparison operations on poly_float and poly_int objects
 * to ensure they behave as expected. It checks basic operations such as addition, subtraction, and multiplication,
 * as well as comparison operators and special functions like sum().
 */
class PolyValuesTest : public UnitTest {
public:
    /**
     * @brief Constructs a PolyValuesTest with a given test name and category.
     */
    PolyValuesTest() : UnitTest("Poly Values", "Utils") { }

    /**
     * @brief Runs all poly value tests, including both float and int based tests.
     */
    void runTest() override;

    /**
     * @brief Runs tests related to floating-point poly values (poly_float).
     *
     * These tests verify arithmetic (addition, subtraction, multiplication), comparison operators,
     * and sum operations for poly_float.
     */
    void runFloatTests();

    /**
     * @brief Runs tests related to integer-based poly values (poly_int).
     *
     * These tests verify arithmetic (addition, subtraction, multiplication), assignment,
     * negation, comparisons, and sum operations for poly_int.
     */
    void runIntTests();
};
