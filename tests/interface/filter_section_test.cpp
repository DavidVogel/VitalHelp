/**
 * @file filter_section_test.cpp
 * @brief Implements the FilterSectionTest class, performing stress tests on the FilterSection UI component.
 */

#include "filter_section_test.h"
#include "filter_section.h"

void FilterSectionTest::runTest() {
    // Create and configure the synth engine required by the FilterSection.
    vital::SoundEngine* engine = createSynthEngine();

    // Acquire a MessageManagerLock for safe UI operations.
    MessageManager::getInstance();
    std::unique_ptr<MessageManagerLock> lock = std::make_unique<MessageManagerLock>();

    // Create the FilterSection component with given parameters and modulation sources.
    FilterSection filter_section(1, engine->getMonoModulations(), engine->getPolyModulations());

    // Release the lock before running stress tests to allow UI processing.
    lock.reset();

    // Perform stress tests with random input on the filter_section
    // to ensure it behaves correctly and robustly.
    runStressRandomTest(&filter_section);

    // Clean up the synth engine once tests are complete.
    deleteSynthEngine();
}

// Instantiate a static test instance that registers and runs this test.
static FilterSectionTest filter_section_test;
