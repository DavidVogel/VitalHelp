/**
 * @file ladder_filter_test.cpp
 * @brief Implements the LadderFilterTest class, performing input bounds tests on the LadderFilter processor.
 */

#include "ladder_filter_test.h"
#include "ladder_filter.h"

void LadderFilterTest::runTest() {
    // Create a LadderFilter processor instance.
    vital::LadderFilter ladder_filter;

    // Run standardized input bounds tests to ensure the ladder filter handles extreme inputs gracefully.
    runInputBoundsTest(&ladder_filter);
}

// Registers the test instance so it will be automatically discovered and run.
static LadderFilterTest ladder_filter_test;
