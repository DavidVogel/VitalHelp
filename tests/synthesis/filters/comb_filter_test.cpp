/**
 * @file comb_filter_test.cpp
 * @brief Implements the CombFilterTest class, running input bounds tests on the CombFilter processor with different filter styles.
 */

#include "comb_filter_test.h"
#include "comb_filter.h"

void CombFilterTest::runTest() {
    // Create a CombFilter processor with a certain memory size.
    vital::CombFilter comb_filter(5000);

    // We will ignore the style input during the test because we'll manually vary it.
    std::set<int> ignored_inputs;
    ignored_inputs.insert(vital::CombFilter::kStyle);

    // Create a style Value and plug it into the comb_filter, allowing us to change filter types.
    vital::Value style;
    comb_filter.plug(&style, vital::CombFilter::kStyle);

    // Cycle through all filter types, running input bounds tests for each.
    for (int i = 0; i < vital::CombFilter::kNumFilterTypes; ++i) {
        style.set(i);
        runInputBoundsTest(&comb_filter, ignored_inputs, std::set<int>());
    }
}

// Registers the test instance so it will be automatically discovered and run.
static CombFilterTest comb_filter_test;
