/**
 * @file modulation_stress_test.cpp
 * @brief Implements the ModulationStressTest class, performing large-scale and random modulation stress tests on the SoundEngine.
 */

#include "modulation_stress_test.h"
#include "synth_constants.h"
#include "synth_types.h"
#include "synth_parameters.h"
#include "modulation_connection_processor.h"

namespace {
    /// Number of times to process the audio block during checks.
    constexpr int kProcessAmount = 35;
    /// Number of samples per process call.
    constexpr int kNumSamples = vital::kMaxBufferSize;
    /// A large modulation amount to stress the engine.
    constexpr float kLargeModulationAmount = 1000.0f;
    /// Number of modulation hook-ups (connection cycles) to perform.
    constexpr int kModulationHookupNumber = 35;
    /// A default destination used if creating a connection to the chosen destination fails.
    const std::string kDefaultConnection = "osc_1_level";

    /**
     * @brief Helper function to create a modulation_change struct from a ModulationConnection and SoundEngine.
     * @param connection The ModulationConnection from which to derive changes.
     * @param engine The SoundEngine providing modulation sources and destinations.
     * @return A fully populated modulation_change struct.
     */
    vital::modulation_change createModulationChange(vital::ModulationConnection* connection, vital::SoundEngine* engine) {
        vital::modulation_change change;
        change.source = engine->getModulationSource(connection->source_name);
        change.mono_destination = engine->getMonoModulationDestination(connection->destination_name);
        change.mono_modulation_switch = engine->getMonoModulationSwitch(connection->destination_name);
        change.destination_scale = 1.0f;

        VITAL_ASSERT(change.source != nullptr);
        VITAL_ASSERT(change.mono_destination != nullptr);
        VITAL_ASSERT(change.mono_modulation_switch != nullptr);

        change.poly_modulation_switch = engine->getPolyModulationSwitch(connection->destination_name);
        change.poly_destination = engine->getPolyModulationDestination(connection->destination_name);
        change.modulation_processor = connection->modulation_processor.get();
        return change;
    }

    /**
     * @brief Enables all parameters ending in "_on" in the given SoundEngine's controls, ensuring all features are active.
     * @param engine A pointer to the SoundEngine whose parameters should be enabled.
     */
    void turnEverythingOn(vital::SoundEngine* engine) {
        std::map<std::string, vital::ValueDetails> parameters = vital::Parameters::lookup_.getAllDetails();
        vital::control_map controls = engine->getControls();
        for (auto& parameter : parameters) {
            String name = parameter.second.name;
            if (name.endsWith("_on") && controls.count(parameter.second.name))
                controls[parameter.second.name]->set(1.0f);
        }
    }
} // namespace

void ModulationStressTest::processAndCheckFinite(vital::Processor* processor) {
    // Reassert the sample rate to ensure internal processing aligns with the correct sample rate.
    processor->setSampleRate(processor->getSampleRate());

    int num_outputs = processor->numOutputs();

    // Process multiple times to stress the processor.
    for (int i = 0; i < kProcessAmount; ++i)
        processor->process(kNumSamples);

    // Verify the output is finite for each output channel.
    for (int i = 0; i < num_outputs; ++i) {
        vital::Output* output = processor->output(i);
        expect(vital::utils::isFinite(output->buffer, output->buffer_size),
               "Output buffer contains non-finite values.");
    }
}

void ModulationStressTest::allModulations() {
    beginTest("All Modulations");

    vital::SoundEngine engine;
    engine.noteOn(60, 1.0f, 0, 0);
    processAndCheckFinite(&engine);
    engine.noteOn(62, 1.0f, 0, 0);
    processAndCheckFinite(&engine);
    engine.noteOn(64, 1.0f, 0, 0);
    processAndCheckFinite(&engine);

    vital::output_map& source_map = engine.getModulationSources();
    vital::input_map& mono_destination_map = engine.getMonoModulationDestinations();
    vital::input_map& poly_destination_map = engine.getMonoModulationDestinations();

    // Collect all source and destination names.
    std::vector<std::string> sources;
    std::vector<std::string> destinations;
    for (auto& source_iter : source_map)
        sources.push_back(source_iter.first);

    for (auto& destination_iter : mono_destination_map)
        destinations.push_back(destination_iter.first);

    for (auto& destination_iter : poly_destination_map) {
        if (!mono_destination_map.count(destination_iter.first))
            destinations.push_back(destination_iter.first);
    }

    std::vector<vital::ModulationConnection*> connections;
    turnEverythingOn(&engine);
    vital::ModulationConnectionBank& modulation_bank = engine.getModulationBank();

    int sources_size = static_cast<int>(sources.size());
    int destinations_size = static_cast<int>(destinations.size());

    // Connect and disconnect a large number of modulations in cycles.
    for (int i = 0; i < kModulationHookupNumber; ++i) {
        int source_start = (i * (sources_size - vital::kMaxModulationConnections)) / kModulationHookupNumber;
        int dest_start = (i * (destinations_size - vital::kMaxModulationConnections)) / kModulationHookupNumber;
        for (int m = 0; m < vital::kMaxModulationConnections; ++m) {
            int source_index = vital::utils::iclamp(source_start + m, 0, sources_size - 1);
            int destination_index = vital::utils::iclamp(dest_start + m, 0, destinations_size - 1);
            std::string source = sources[source_index];
            std::string destination = destinations[destination_index];

            vital::ModulationConnection* connection = modulation_bank.createConnection(source, destination);
            if (connection == nullptr)
                connection = modulation_bank.createConnection(source, kDefaultConnection);

            expect(connection != nullptr, "Failed to create modulation connection.");

            connection->modulation_processor->setBaseValue(kLargeModulationAmount * (rand() % 2 ? 1.0f : -1.0f));
            connections.push_back(connection);

            vital::modulation_change change = createModulationChange(connection, &engine);
            change.disconnecting = false;
            engine.connectModulation(change);
        }

        processAndCheckFinite(&engine);

        // Disconnect all the created modulations this cycle.
        for (vital::ModulationConnection* connection : connections) {
            vital::modulation_change change = createModulationChange(connection, &engine);
            change.disconnecting = true;
            engine.disconnectModulation(change);
            connection->source_name = "";
            connection->destination_name = "";
        }
        connections.clear();
        processAndCheckFinite(&engine);
    }
}

void ModulationStressTest::randomModulations() {
    beginTest("Random Modulations");

    vital::SoundEngine engine;
    engine.noteOn(60, 1.0f, 0, 0);
    processAndCheckFinite(&engine);
    engine.noteOn(62, 1.0f, 0, 0);
    processAndCheckFinite(&engine);
    engine.noteOn(64, 1.0f, 0, 0);
    processAndCheckFinite(&engine);

    vital::output_map& source_map = engine.getModulationSources();
    vital::input_map& mono_destination_map = engine.getMonoModulationDestinations();
    vital::input_map& poly_destination_map = engine.getMonoModulationDestinations();

    // Collect source and destination names.
    std::vector<std::string> sources;
    std::vector<std::string> destinations;
    for (auto& source_iter : source_map)
        sources.push_back(source_iter.first);

    for (auto& destination_iter : mono_destination_map)
        destinations.push_back(destination_iter.first);

    for (auto& destination_iter : poly_destination_map) {
        if (!mono_destination_map.count(destination_iter.first))
            destinations.push_back(destination_iter.first);
    }

    std::vector<vital::ModulationConnection*> connections;
    turnEverythingOn(&engine);
    vital::ModulationConnectionBank& modulation_bank = engine.getModulationBank();

    int sources_size = static_cast<int>(sources.size());
    int destinations_size = static_cast<int>(destinations.size());

    // Randomly choose sources and destinations in cycles, connect and disconnect them.
    for (int i = 0; i < kModulationHookupNumber; ++i) {
        DBG("");
        int source_start = (i * (sources_size - vital::kMaxModulationConnections / 2)) / kModulationHookupNumber;
        int dest_start = (i * (destinations_size - vital::kMaxModulationConnections / 2)) / kModulationHookupNumber;

        // Half the connections are chosen systematically.
        for (int m = 0; m < vital::kMaxModulationConnections / 2; ++m) {
            int source_index = vital::utils::iclamp(source_start + m, 0, sources_size - 1);
            int destination_index = vital::utils::iclamp(dest_start + m, 0, destinations_size - 1);
            std::string source = sources[source_index];
            std::string destination = destinations[destination_index];

            vital::ModulationConnection* connection = modulation_bank.createConnection(source, destination);
            if (connection == nullptr)
                connection = modulation_bank.createConnection(source, kDefaultConnection);

            expect(connection != nullptr, "Failed to create modulation connection.");
            connection->modulation_processor->setBaseValue(kLargeModulationAmount * (rand() % 2 ? 1.0f : -1.0f));
            connections.push_back(connection);

            vital::modulation_change change = createModulationChange(connection, &engine);
            change.disconnecting = false;
            engine.connectModulation(change);
        }

        // The other half of connections are chosen randomly.
        for (int m = vital::kMaxModulationConnections / 2; m < vital::kMaxModulationConnections; ++m) {
            std::string source = sources[rand() % sources.size()];
            std::string destination = destinations[rand() % destinations.size()];

            vital::ModulationConnection* connection = modulation_bank.createConnection(source, destination);
            if (connection == nullptr)
                connection = modulation_bank.createConnection(source, kDefaultConnection);

            expect(connection != nullptr, "Failed to create modulation connection.");
            connection->modulation_processor->setBaseValue(kLargeModulationAmount * (rand() % 2 ? 1.0f : -1.0f));
            connections.push_back(connection);

            vital::modulation_change change = createModulationChange(connection, &engine);
            change.disconnecting = false;
            engine.connectModulation(change);
        }

        processAndCheckFinite(&engine);

        // Disconnect all the created modulations in this cycle.
        for (vital::ModulationConnection* connection : connections) {
            vital::modulation_change change = createModulationChange(connection, &engine);
            change.disconnecting = true;
            engine.disconnectModulation(change);
            connection->source_name = "";
            connection->destination_name = "";
        }
        connections.clear();
        processAndCheckFinite(&engine);
    }
}

void ModulationStressTest::runTest() {
    // Run tests that connect all available modulations.
    allModulations();

    // Run tests that randomly connect modulations.
    randomModulations();
}

// Register the modulation stress test so it runs automatically.
static ModulationStressTest modulation_stress_test;
