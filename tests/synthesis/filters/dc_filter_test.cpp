/**
 * @file dc_filter_test.cpp
 * @brief Implements the DcFilterTest class, running input bounds tests against the DcFilter processor.
 */

#include "dc_filter_test.h"
#include "dc_filter.h"

void DcFilterTest::runTest() {
    // Create a DcFilter processor instance.
    vital::DcFilter dc_filter;

    // Run a standardized input bounds test to ensure the DC filter behaves correctly under extreme input conditions.
    runInputBoundsTest(&dc_filter);
}

// Registers the test instance so it will be automatically discovered and run.
static DcFilterTest dc_filter_test;
