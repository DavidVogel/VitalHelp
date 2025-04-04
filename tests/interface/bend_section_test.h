/**
 * @file bend_section_test.h
 * @brief Declares the BendSectionTest class, which tests the BendSection interface component.
 */

#pragma once

#include "interface_test.h"

/**
 * @class BendSectionTest
 * @brief A test class for verifying the functionality and behavior of the BendSection interface component.
 *
 * This test creates a synth engine, constructs a BendSection UI component, and runs stress tests
 * to ensure the BendSection behaves as expected under various conditions.
 */
class BendSectionTest : public InterfaceTest {
public:
    /**
     * @brief Constructs a BendSectionTest with a specified test name.
     */
    BendSectionTest() : InterfaceTest("Bend Section") { }

    /**
     * @brief Runs the BendSection test. This method sets up the test environment,
     * creates the BendSection component, and performs stress tests on it.
     */
    void runTest() override;
};
