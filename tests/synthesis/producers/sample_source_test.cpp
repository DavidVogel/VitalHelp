/**
 * @file sample_source_test.cpp
 * @brief Implements the SampleSourceTest class, performing input bounds tests on the SampleSource processor.
 */

#include "sample_source_test.h"
#include "sample_source.h"

void SampleSourceTest::runTest() {
    // Create a SampleSource processor instance.
    vital::SampleSource sample_source;

    // Run a standardized input bounds test to ensure the sample source handles extreme inputs gracefully.
    runInputBoundsTest(&sample_source);
}

// Registers the test instance so it will be automatically discovered and run.
static SampleSourceTest sample_source_test;
