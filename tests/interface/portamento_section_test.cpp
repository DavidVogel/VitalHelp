/**
 * @file portamento_section_test.cpp
 * @brief Implements the PortamentoSectionTest class, performing stress tests on the PortamentoSection UI component.
 */

#include "portamento_section_test.h"
#include "portamento_section.h"

void PortamentoSectionTest::runTest() {
    // Create and configure the synth engine required by the PortamentoSection.
    createSynthEngine();

    // Acquire a MessageManagerLock for safe UI operations.
    MessageManager::getInstance();
    std::unique_ptr<MessageManagerLock> lock = std::make_unique<MessageManagerLock>();

    // Create the PortamentoSection component for testing.
    PortamentoSection portamento_section("Portamento");

    // Release the lock before running stress tests to allow UI processing.
    lock.reset();

    // Perform stress tests with random input on the portamento_section
    // to ensure it behaves correctly and robustly.
    runStressRandomTest(&portamento_section);

    // Clean up the synth engine once tests are complete.
    deleteSynthEngine();
}

// Instantiate a static test instance that registers and runs this test.
static PortamentoSectionTest portamento_section_test;
