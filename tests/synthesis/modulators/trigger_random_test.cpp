/**
 * @file trigger_random_test.cpp
 * @brief Implements the TriggerRandomTest class, performing input bounds tests on the TriggerRandom processor.
 */

#include "trigger_random_test.h"
#include "trigger_random.h"

void TriggerRandomTest::runTest() {
    // Create a TriggerRandom processor instance.
    vital::TriggerRandom trigger_random;

    // Run a standardized input bounds test to ensure the trigger random processor handles extreme inputs gracefully.
    runInputBoundsTest(&trigger_random);
}

// Registers the test instance so it will be automatically discovered and run.
static TriggerRandomTest trigger_random_test;
