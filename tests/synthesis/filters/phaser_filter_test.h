/**
 * @file phaser_filter_test.h
 * @brief Declares the PhaserFilterTest class, which tests the PhaserFilter processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class PhaserFilterTest
 * @brief A test class that verifies the stability and correctness of the PhaserFilter processor.
 *
 * This test ensures that the PhaserFilter, in both inversion states (true and false),
 * handles extreme input values without producing non-finite outputs. It leverages the
 * ProcessorTest framework to run standardized input bounds tests.
 */
class PhaserFilterTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new PhaserFilterTest with the specified test name.
     */
    PhaserFilterTest() : ProcessorTest("Phaser Filter") { }

    /**
     * @brief Runs the PhaserFilter test by performing input bounds checks on two PhaserFilter instances:
     *        one with inversion (true) and one without inversion (false), verifying stability under
     *        extreme input conditions.
     */
    void runTest() override;
};
