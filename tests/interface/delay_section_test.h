/**
 * @file delay_section_test.h
 * @brief Declares the DelaySectionTest class, which tests the DelaySection interface component.
 */

#pragma once

#include "interface_test.h"

/**
 * @class DelaySectionTest
 * @brief A test class for verifying the functionality and stability of the DelaySection interface.
 *
 * This test sets up a synth engine, creates a DelaySection UI component, and runs
 * stress tests to ensure that the delay controls and UI interaction behave as expected.
 */
class DelaySectionTest : public InterfaceTest {
public:
    /**
     * @brief Constructs a new DelaySectionTest with a specified test name.
     */
    DelaySectionTest() : InterfaceTest("Delay Section") { }

    /**
     * @brief Runs the DelaySection test by creating the test environment, initializing
     *        the DelaySection component, and performing stress tests.
     */
    void runTest() override;
};
