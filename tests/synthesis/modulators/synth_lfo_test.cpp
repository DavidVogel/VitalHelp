/**
 * @file synth_lfo_test.cpp
 * @brief Implements the SynthLfoTest class, performing input bounds tests on the SynthLfo processor.
 */

#include "synth_lfo_test.h"
#include "synth_lfo.h"
#include "line_generator.h"

void SynthLfoTest::runTest() {
    // Create a LineGenerator to serve as the LFO source.
    LineGenerator line_source;

    // Create a SynthLfo processor instance using the line_source.
    vital::SynthLfo synth_lfo(&line_source);

    // Prepare sets for ignoring certain inputs/outputs if necessary.
    std::set<int> ignored_inputs;
    std::set<int> ignored_outputs;

    // Ignore the OscPhase output as it may not be relevant to the primary stability test.
    ignored_outputs.insert(vital::SynthLfo::kOscPhase);

    // Run standardized input bounds tests to ensure the synth LFO handles extreme inputs gracefully.
    runInputBoundsTest(&synth_lfo, ignored_inputs, ignored_outputs);
}

// Registers the test instance so it will be automatically discovered and run.
static SynthLfoTest synth_lfo_test;
