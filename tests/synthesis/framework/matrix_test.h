/**
 * @file matrix_test.h
 * @brief Declares the MatrixTest class, which tests the vital::matrix data structure and its operations.
 */

#pragma once

#include "JuceHeader.h"

/**
 * @class MatrixTest
 * @brief A test class for verifying the functionality and correctness of the matrix operations.
 *
 * The MatrixTest class checks operations on matrices, such as transposing a matrix, and ensures that
 * they produce the expected results. It may be extended to test additional operations like multiplication
 * and inversions if needed.
 */
class MatrixTest : public UnitTest {
public:
    /**
     * @brief Constructs a MatrixTest with a given test name and category.
     */
    MatrixTest() : UnitTest("Matrix", "Utils") { }

    /**
     * @brief Runs the matrix tests. It currently verifies the transpose operation on a matrix.
     */
    void runTest() override;

    /**
     * @brief Runs tests related to floating-point matrix operations.
     *
     * @note Currently unused. Can be extended for additional float-related matrix tests.
     */
    void runFloatTests();

    /**
     * @brief Runs tests related to integer matrix operations.
     *
     * @note Currently unused. Can be extended for additional integer-related matrix tests.
     */
    void runIntTests();
};
