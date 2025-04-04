/**
 * @file phaser_test.h
 * @brief Declares the PhaserTest class, which tests the Phaser processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class PhaserTest
 * @brief A test class that verifies the stability and correctness of the Phaser processor.
 *
 * This test checks whether the Phaser processor handles extreme input conditions without
 * producing non-finite outputs. It uses the ProcessorTest framework to run standardized
 * input bounds tests and ignores certain non-primary outputs as necessary.
 */
class PhaserTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new PhaserTest with a specified test name.
     */
    PhaserTest() : ProcessorTest("Phaser") { }

    /**
     * @brief Runs the phaser test by performing input bounds checks, ensuring that the phaser
     *        output remains stable and finite under extreme input conditions.
     */
    void runTest() override;
};
