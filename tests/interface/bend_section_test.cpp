/**
 * @file bend_section_test.cpp
 * @brief Implements the BendSectionTest class, conducting stress tests on the BendSection UI component.
 */

#include "bend_section_test.h"
#include "bend_section.h"

void BendSectionTest::runTest() {
    createSynthEngine();

    // Ensures that we hold a lock for message manager operations (UI updates)
    MessageManager::getInstance();
    std::unique_ptr<MessageManagerLock> lock = std::make_unique<MessageManagerLock>();

    // Create a BendSection component for testing
    BendSection bend_section("Bend");

    // Release the lock before running tests to allow UI thread operations
    lock.reset();

    // Perform stress tests with random input on the bend_section
    runStressRandomTest(&bend_section);

    // Clean up the synth engine after tests are complete
    deleteSynthEngine();
}

// Instantiate a static test instance that will be run automatically.
static BendSectionTest bend_section_test;
