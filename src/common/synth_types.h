/*
Summary:
In these files, we define fundamental types and structures that are central to the internal state and logic of the Vital synthesizer engine. The ModulationConnection and ModulationConnectionBank manage the links between modulation sources (like LFOs) and destinations (like filter cutoff). The StringLayout helps map a computer keyboard to musical notes. The modulation_change, control_map, input_map, and output_map typedefs provide convenient, strongly-typed ways to manage changes to controls, handle various inputs, and track outputs throughout the synthesizer’s audio graph.
*/

#pragma once

#include "circular_queue.h"
#include "synth_parameters.h"
#include "operators.h"
#include "value.h"

#include <map>
#include <string>

namespace vital {

    class ValueSwitch;
    class ModulationConnectionProcessor;

    /**
     * @brief A structure representing a single modulation connection between a modulation source and a destination parameter.
     *
     * Each ModulationConnection links a `source_name` (e.g., an LFO or envelope) to a `destination_name` (a synth parameter).
     * It holds a ModulationConnectionProcessor to apply transformations like scaling or mapping curves.
     *
     * The `isModulationSourceDefaultBipolar` method checks if the source typically produces bipolar values (e.g., LFOs).
     * Connections can be reset, and by default start disconnected until `resetConnection` is called.
     */
    struct ModulationConnection {
        /**
         * @brief Constructs a ModulationConnection with an index and empty source/destination.
         *
         * @param index The connection index, often used to identify this modulation slot.
         */
        ModulationConnection(int index) : ModulationConnection(index, "", "") { }

        /**
         * @brief Constructs a ModulationConnection with given index and names.
         *
         * @param index The connection index.
         * @param from The modulation source name.
         * @param to The destination parameter name.
         */
        ModulationConnection(int index, std::string from, std::string to);

        /**
         * @brief Destroys the ModulationConnection, cleaning up its processor.
         */
        ~ModulationConnection();

        /**
         * @brief Checks if a given modulation source is bipolar by default.
         *
         * Some modulation sources (e.g., LFOs) naturally produce values in a bipolar range.
         *
         * @param source The modulation source name.
         * @return True if default is bipolar, false otherwise.
         */
        static bool isModulationSourceDefaultBipolar(const std::string& source);

        /**
         * @brief Resets this modulation connection to a new source and destination.
         *
         * @param from The new modulation source name.
         * @param to The new destination parameter name.
         */
        void resetConnection(const std::string& from, const std::string& to) {
            source_name = from;
            destination_name = to;
        }

        std::string source_name;  ///< The name of the modulation source.
        std::string destination_name; ///< The name of the destination parameter.
        std::unique_ptr<ModulationConnectionProcessor> modulation_processor; ///< Processor applying scaling/mapping.

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationConnection)
    };

    /**
     * @brief A container managing a fixed number of ModulationConnections.
     *
     * ModulationConnectionBank pre-allocates a fixed number of connections (kMaxModulationConnections).
     * It provides methods to create new connections by assigning a source and destination to an available slot.
     */
    class ModulationConnectionBank {
    public:
        /**
         * @brief Constructs the bank and pre-allocates all modulation connection slots.
         */
        ModulationConnectionBank();

        /**
         * @brief Destroys the ModulationConnectionBank and its connections.
         */
        ~ModulationConnectionBank();

        /**
         * @brief Creates a new modulation connection by finding an empty slot and assigning source/destination.
         *
         * @param from The modulation source name.
         * @param to The destination parameter name.
         * @return A pointer to the newly created ModulationConnection, or nullptr if no slot is available.
         */
        ModulationConnection* createConnection(const std::string& from, const std::string& to);

        /**
         * @brief Retrieves a ModulationConnection by index.
         *
         * @param index The connection index (0-based).
         * @return A pointer to the ModulationConnection.
         */
        ModulationConnection* atIndex(int index) { return all_connections_[index].get(); }

        /**
         * @brief Returns the total number of connections allocated (including unused ones).
         *
         * @return The number of connections.
         */
        size_t numConnections() { return all_connections_.size(); }

    private:
        std::vector<std::unique_ptr<ModulationConnection>> all_connections_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationConnectionBank)
    };

    /**
     * @brief Manages a keyboard layout mapping for a computer keyboard used as a MIDI input device.
     *
     * StringLayout stores a layout of characters that map to notes, and keys to shift the octave up or down.
     */
    class StringLayout {
    public:
        /**
         * @brief Constructs a StringLayout with no keys and an empty layout.
         */
        StringLayout() : up_key_(0), down_key_(0) { }

        /**
         * @brief Retrieves the current keyboard layout (a wstring of character-to-note mappings).
         *
         * @return The layout as a wstring.
         */
        std::wstring getLayout() { return layout_; }

        /**
         * @brief Sets the current keyboard layout.
         *
         * @param layout The new keyboard layout.
         */
        void setLayout(const std::wstring& layout) { layout_ = layout; }

        /**
         * @brief Gets the character assigned to increase the octave.
         *
         * @return The up key character.
         */
        wchar_t getUpKey() { return up_key_; }

        /**
         * @brief Sets the character assigned to increase the octave.
         *
         * @param up_key The character for octave up.
         */
        void setUpKey(wchar_t up_key) { up_key_ = up_key; }

        /**
         * @brief Gets the character assigned to decrease the octave.
         *
         * @return The down key character.
         */
        wchar_t getDownKey() { return down_key_; }

        /**
         * @brief Sets the character assigned to decrease the octave.
         *
         * @param down_key The character for octave down.
         */
        void setDownKey(wchar_t down_key) { down_key_ = down_key; }

    protected:
        std::wstring layout_; ///< The mapping of keys to notes.
        int up_key_;          ///< The key code (wchar_t) for octave up.
        int down_key_;        ///< The key code (wchar_t) for octave down.

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StringLayout)
    };

    /**
     * @brief A structure describing changes to the modulation routing in the engine.
     *
     * A modulation_change is used to communicate updates in modulation connections:
     * connecting, disconnecting, or changing modulation parameters. It includes pointers to
     * sources, destinations, and switches controlling mono/poly modulation. The engine uses
     * this to apply changes atomically to the modulation network.
     */
    typedef struct {
        Output* source;                     ///< The modulation source output.
        Processor* mono_destination;        ///< The mono modulation destination (if any).
        Processor* poly_destination;        ///< The poly modulation destination (if any).
        mono_float destination_scale;       ///< Scaling factor for the destination parameter.
        ValueSwitch* mono_modulation_switch;///< Switch to enable/disable mono modulation.
        ValueSwitch* poly_modulation_switch;///< Switch to enable/disable poly modulation.
        ModulationConnectionProcessor* modulation_processor; ///< The processor for applying modulation shape/curve.
        bool disconnecting;                 ///< True if this change represents a disconnection.
        int num_audio_rate;                 ///< Count of audio-rate modulations connected from the same source.
    } modulation_change;

    /**
     * @brief Maps parameter names to Value pointers representing synth control parameters.
     */
    typedef std::map<std::string, Value*> control_map;

    /**
     * @brief Represents a single control change: pairs a parameter Value with the new requested value.
     */
    typedef std::pair<Value*, mono_float> control_change;

    /**
     * @brief Maps parameter names to Processor pointers, representing input processors for signals.
     */
    typedef std::map<std::string, Processor*> input_map;

    /**
     * @brief Maps parameter names to Output pointers, representing output signals from various modules.
     */
    typedef std::map<std::string, Output*> output_map;
} // namespace vital
