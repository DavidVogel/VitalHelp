/**
 * @file random_lfo_test.cpp
 * @brief Implements the RandomLfoTest class, performing input bounds tests on the RandomLfo processor.
 */

#include "random_lfo_test.h"
#include "random_lfo.h"

void RandomLfoTest::runTest() {
    // Create a RandomLfo processor instance.
    vital::RandomLfo random_lfo;

    // At this point, we aren't ignoring any specific inputs or outputs for the test,
    // but the sets are provided for future flexibility.
    std::set<int> ignored_inputs;
    std::set<int> ignored_outputs;

    // Run standardized input bounds tests to ensure the random LFO handles extreme inputs gracefully.
    runInputBoundsTest(&random_lfo, ignored_inputs, ignored_outputs);
}

// Registers the test instance so it will be automatically discovered and run.
static RandomLfoTest random_lfo_test;
