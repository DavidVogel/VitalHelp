/**
 * @file distortion_test.cpp
 * @brief Implements the DistortionTest class, performing input bounds tests on the Distortion processor.
 */

#include "distortion_test.h"
#include "distortion.h"

void DistortionTest::runTest() {
    // Create a Distortion processor instance.
    vital::Distortion distortion;

    // Set of outputs to ignore in the test. For the Distortion processor, we ignore the kDriveOut output.
    std::set<int> ignored_outputs;
    ignored_outputs.insert(vital::Distortion::kDriveOut);

    // Run input bounds tests on the distortion processor, ensuring that for extreme inputs,
    // the processor outputs remain finite. We ignore certain outputs that aren't crucial to the main test.
    runInputBoundsTest(&distortion, std::set<int>(), ignored_outputs);
}

// Registers the test instance so it will be automatically discovered and run.
static DistortionTest distortion_test;
