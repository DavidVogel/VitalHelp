/**
 * @file linkwitz_riley_filter_test.h
 * @brief Declares the LinkwitzRileyFilterTest class, which tests the LinkwitzRileyFilter processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class LinkwitzRileyFilterTest
 * @brief A test class that verifies the stability and correctness of the LinkwitzRileyFilter processor.
 *
 * This test ensures that the LinkwitzRileyFilter, which implements a Linkwitz-Riley crossover filter,
 * handles extreme input values without producing non-finite outputs. It leverages the ProcessorTest framework
 * to run standardized input bounds tests.
 */
class LinkwitzRileyFilterTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new LinkwitzRileyFilterTest with the specified test name.
     */
    LinkwitzRileyFilterTest() : ProcessorTest("Linkwitz Riley Filter") { }

    /**
     * @brief Runs the LinkwitzRileyFilter test by performing input bounds checks, verifying that the
     *        filter remains stable and produces finite output under extreme input conditions.
     */
    void runTest() override;
};
