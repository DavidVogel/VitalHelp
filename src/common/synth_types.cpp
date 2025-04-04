/*
Summary:
In these files, we define fundamental types and structures that are central to the internal state and logic of the Vital synthesizer engine. The ModulationConnection and ModulationConnectionBank manage the links between modulation sources (like LFOs) and destinations (like filter cutoff). The StringLayout helps map a computer keyboard to musical notes. The modulation_change, control_map, input_map, and output_map typedefs provide convenient, strongly-typed ways to manage changes to controls, handle various inputs, and track outputs throughout the synthesizer’s audio graph.
*/

#include "synth_types.h"

#include "synth_constants.h"
#include "modulation_connection_processor.h"

namespace vital {
  namespace {
    // Delimiter used in modulation source names to separate prefixes.
    const std::string kModulationSourceDelimiter = "_";

    // Set of known prefixes that produce bipolar values by default (e.g. "lfo", "random").
    const std::set<std::string> kBipolarModulationSourcePrefixes = {
      "lfo",
      "stereo",
      "random",
      "pitch"
    };

    /**
     * @brief Checks if a given connection is available (has empty source and destination).
     *
     * @param connection A pointer to a ModulationConnection.
     * @return True if both source and destination are empty, meaning unused.
     */
    force_inline bool isConnectionAvailable(ModulationConnection* connection) {
      return connection->source_name.empty() && connection->destination_name.empty();
    }
  }

  ModulationConnection::ModulationConnection(int index, std::string from, std::string to) :
      source_name(std::move(from)), destination_name(std::move(to)) {
    // Create a ModulationConnectionProcessor for this connection index.
    modulation_processor = std::make_unique<ModulationConnectionProcessor>(index);
  }

  ModulationConnection::~ModulationConnection() { }

  bool ModulationConnection::isModulationSourceDefaultBipolar(const std::string& source) {
    // Determine if the source prefix indicates a bipolar output.
    std::size_t pos = source.find(kModulationSourceDelimiter);
    std::string prefix = source.substr(0, pos);
    return kBipolarModulationSourcePrefixes.count(prefix) > 0;
  }

  ModulationConnectionBank::ModulationConnectionBank() {
    // Pre-allocate kMaxModulationConnections with empty source/destination.
    for (int i = 0; i < kMaxModulationConnections; ++i) {
      std::unique_ptr<ModulationConnection> connection = std::make_unique<ModulationConnection>(i);
      all_connections_.push_back(std::move(connection));
    }
  }

  ModulationConnectionBank::~ModulationConnectionBank() { }

  ModulationConnection* ModulationConnectionBank::createConnection(const std::string& from, const std::string& to) {
    // Find an available connection slot and set up the connection.
    int index = 1;
    for (auto& connection : all_connections_) {
      std::string invalid_connection = "modulation_" + std::to_string(index++) + "_amount";
      if (to != invalid_connection && isConnectionAvailable(connection.get())) {
        connection->resetConnection(from, to);
        connection->modulation_processor->setBipolar(ModulationConnection::isModulationSourceDefaultBipolar(from));
        return connection.get();
      }
    }

    // No available slots.
    return nullptr;
  }
} // namespace vital
