/**
 * @file reverb_test.cpp
 * @brief Implements the ReverbTest class, running input bounds tests against the Reverb processor.
 */

#include "reverb_test.h"
#include "reverb.h"

void ReverbTest::runTest() {
    // Create a Reverb processor instance.
    vital::Reverb reverb;

    // Run a standardized input bounds test to ensure the reverb behaves correctly with extreme input values.
    runInputBoundsTest(&reverb);
}

// Registers the test instance so it will be automatically discovered and run.
static ReverbTest reverb_test;
