/**
 * @file full_interface_test.cpp
 * @brief Implements the FullInterfaceTest class, performing stress tests on the FullInterface UI component.
 */

#include "full_interface_test.h"
#include "full_interface.h"
#include "synth_gui_interface.h"

void FullInterfaceTest::runTest() {
    // Create and configure the synth engine required by the full interface.
    createSynthEngine();

    // Initialize GUI data needed by the FullInterface.
    SynthGuiData data(getSynthBase());

    // Acquire a MessageManagerLock for safe UI operations.
    MessageManager::getInstance();
    std::unique_ptr<MessageManagerLock> lock = std::make_unique<MessageManagerLock>();

    // Create the FullInterface component with the provided GUI data.
    FullInterface* full_interface = new FullInterface(&data);

    // Release the lock before running stress tests to allow UI processing.
    lock.reset();

    // Configure the full interface with oscilloscope and audio memory from the synth base.
    full_interface->setOscilloscopeMemory(getSynthBase()->getOscilloscopeMemory());
    full_interface->setAudioMemory(getSynthBase()->getAudioMemory());

    // Perform stress tests with random input on the full_interface
    // to ensure correct and robust behavior across the entire UI.
    runStressRandomTest(full_interface, full_interface);

    // Clean up the synth engine once tests are complete.
    deleteSynthEngine();
}

// Instantiate a static test instance that registers and runs this test.
static FullInterfaceTest full_interface_test;
