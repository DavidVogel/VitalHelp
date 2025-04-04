/**
 * @file dirty_filter_test.cpp
 * @brief Implements the DirtyFilterTest class, performing input bounds tests on the DirtyFilter processor.
 */

#include "dirty_filter_test.h"
#include "dirty_filter.h"

void DirtyFilterTest::runTest() {
    // Create a DirtyFilter processor instance.
    vital::DirtyFilter dirty_filter;

    // Run standardized input bounds tests to ensure the dirty filter handles extreme inputs gracefully.
    runInputBoundsTest(&dirty_filter);
}

// Registers the test instance so it will be automatically discovered and run.
static DirtyFilterTest dirty_filter_test;
