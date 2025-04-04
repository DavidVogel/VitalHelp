/**
 * @file compressor_section_test.h
 * @brief Declares the CompressorSectionTest class, which tests the CompressorSection interface component.
 */

#pragma once

#include "interface_test.h"

/**
 * @class CompressorSectionTest
 * @brief A test class for verifying the functionality and stability of the CompressorSection interface.
 *
 * This test class sets up a synth engine environment, creates a CompressorSection component,
 * and performs stress tests to ensure the compressor's UI elements and controls function correctly.
 */
class CompressorSectionTest : public InterfaceTest {
public:
    /**
     * @brief Constructs a new CompressorSectionTest with a specific test name.
     */
    CompressorSectionTest() : InterfaceTest("Compressor Section") { }

    /**
     * @brief Runs the compressor section test by creating the component and subjecting it to stress tests.
     */
    void runTest() override;
};
