/**
 * @file decimator_test.cpp
 * @brief Implements the DecimatorTest class, performing input bounds tests on the Decimator processor.
 */

#include "decimator_test.h"
#include "decimator.h"

void DecimatorTest::runTest() {
    // Create a Decimator processor instance.
    vital::Decimator decimator;

    // Run a standardized input bounds test to ensure the decimator handles extreme input values
    // without producing invalid (non-finite) outputs.
    runInputBoundsTest(&decimator);
}

// Registers the test instance so it will be automatically discovered and run.
static DecimatorTest decimator_test;
