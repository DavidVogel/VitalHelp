/**
 * @file full_interface_test.h
 * @brief Declares the FullInterfaceTest class, which tests the FullInterface UI component of the synth.
 */

#pragma once

#include "interface_test.h"

/**
 * @class FullInterfaceTest
 * @brief A test class for verifying the functionality and stability of the FullInterface UI component.
 *
 * This test initializes a full synth interface, including oscilloscopes and audio memory visualization,
 * and then runs stress tests to ensure the entire interface responds correctly and is stable under
 * various random conditions.
 */
class FullInterfaceTest : public InterfaceTest {
public:
    /**
     * @brief Constructs a new FullInterfaceTest with the specified test name.
     */
    FullInterfaceTest() : InterfaceTest("Full Interface") { }

    /**
     * @brief Runs the test by setting up the full interface, configuring all necessary data,
     *        and executing stress tests against it.
     */
    void runTest() override;
};
