/**
 * @file value_switch_test.h
 * @brief Declares the ValueSwitchTest class, which tests the ValueSwitch processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class ValueSwitchTest
 * @brief A test class that verifies the stability and correctness of the ValueSwitch processor.
 *
 * This test ensures that the ValueSwitch processor handles extreme input conditions without producing
 * invalid (non-finite) outputs. By leveraging the ProcessorTest framework, it applies standardized
 * input bounds tests to ensure the value switching mechanism behaves predictably.
 */
class ValueSwitchTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new ValueSwitchTest with the specified test name.
     */
    ValueSwitchTest() : ProcessorTest("Value Switch") { }

    /**
     * @brief Runs the ValueSwitch test by performing input bounds checks, verifying that the
     *        switching output remains stable and finite under extreme input conditions.
     */
    void runTest() override;
};
