/**
 * @file phaser_test.cpp
 * @brief Implements the PhaserTest class, performing input bounds tests on the Phaser processor.
 */

#include "phaser_test.h"
#include "phaser.h"

void PhaserTest::runTest() {
    // Create a Phaser processor instance.
    vital::Phaser phaser;

    // Set of outputs to ignore during the test. The cutoff output is not relevant for the main test.
    std::set<int> ignored_inputs;
    std::set<int> ignored_outputs;
    ignored_outputs.insert(vital::Phaser::kCutoffOutput);

    // Perform input bounds tests on the phaser processor, ignoring the specified output.
    runInputBoundsTest(&phaser, ignored_inputs, ignored_outputs);
}

// Registers the test instance so it will be automatically discovered and run.
static PhaserTest phaser_test;
