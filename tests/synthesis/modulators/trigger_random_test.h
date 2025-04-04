/**
 * @file trigger_random_test.h
 * @brief Declares the TriggerRandomTest class, which tests the TriggerRandom processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class TriggerRandomTest
 * @brief A test class that verifies the stability and correctness of the TriggerRandom processor.
 *
 * This test ensures that the TriggerRandom processor, which generates random values on triggers,
 * handles extreme input values without producing invalid (non-finite) outputs. It uses the
 * ProcessorTest framework to run standardized input bounds tests.
 */
class TriggerRandomTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new TriggerRandomTest with the specified test name.
     */
    TriggerRandomTest() : ProcessorTest("Trigger Random") { }

    /**
     * @brief Runs the TriggerRandom test by performing input bounds checks, verifying that the
     *        random generator remains stable and produces finite outputs under extreme input conditions.
     */
    void runTest() override;
};
