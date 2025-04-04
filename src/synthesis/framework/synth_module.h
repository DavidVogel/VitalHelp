/**
 * @file synth_module.h
 * @brief Defines the SynthModule class which extends ProcessorRouter to form a building block
 *        of the Vital synthesizer, encapsulating controls, modulation sources, and sub-modules.
 *
 * A SynthModule is a higher-level construct that groups together multiple Processors,
 * Inputs, Outputs, and controls, handling both mono and polyphonic modulation chains.
 * It provides interfaces for creating and retrieving modulation sources and destinations,
 * managing parameter controls, and nesting submodules.
 */

#pragma once

#include "synth_types.h"
#include "processor_router.h"

#include <climits>
#include <vector>
#include <map>
#include <memory>
#include <string>

namespace vital {
    class ValueSwitch;
    class SynthModule;

    /**
     * @class StatusOutput
     * @brief A helper class to track the "status" of a particular Output as a poly_float value.
     *
     * The StatusOutput class allows reading and clearing of a status value from
     * a given Output. It can accumulate values for different voices (via update calls),
     * and also recognizes a special "clear" value.
     */
    class StatusOutput {
    public:
        static constexpr float kClearValue = INT_MIN; ///< Special "clear" value indicating no status.

        /**
         * @brief Constructor.
         * @param source The Output from which status values are read.
         */
        StatusOutput(Output* source) : source_(source), value_(0.0f) { }

        /**
         * @brief Returns the current status value.
         * @return A poly_float containing the current status.
         */
        force_inline poly_float value() const { return value_; }

        /**
         * @brief Updates the status value using the provided mask.
         *
         * Copies masked values from the source Output into the internal value.
         * @param voice_mask A mask indicating which voices are active.
         */
        force_inline void update(poly_mask voice_mask) {
            poly_float masked_value = source_->buffer[0] & voice_mask;
            value_ = masked_value + utils::swapVoices(masked_value);
        }

        /**
         * @brief Updates the status value without masking.
         *
         * Copies the value from the source Output directly.
         */
        force_inline void update() {
            value_ = source_->buffer[0];
        }

        /**
         * @brief Clears the stored status value.
         */
        force_inline void clear() { value_ = kClearValue; }

        /**
         * @brief Checks if a given poly_float is the special "clear" value.
         * @param value The value to check.
         * @return True if the value matches the clear value.
         */
        force_inline bool isClearValue(poly_float value) const { return poly_float::equal(value, kClearValue).anyMask(); }

        /**
         * @brief Checks if a given float is the special "clear" value.
         * @param value The value to check.
         * @return True if the value matches the clear value.
         */
        force_inline bool isClearValue(float value) const { return value == kClearValue; }

    private:
        Output* source_;      ///< The source Output to read from.
        poly_float value_;    ///< The current stored status value.

        JUCE_LEAK_DETECTOR(StatusOutput)
    };

    /**
     * @struct ModuleData
     * @brief Holds various data structures that define the internal state of a SynthModule.
     *
     * This includes owned mono processors, submodules, and maps of controls, modulation sources,
     * destinations, and status outputs. It is shared among various parts of the SynthModule.
     */
    struct ModuleData {
        std::vector<Processor*> owned_mono_processors; ///< Processors owned by this module (mono).
        std::vector<SynthModule*> sub_modules;         ///< Nested submodules.

        control_map controls;                          ///< Map of control parameter names to Value Processors.
        output_map mod_sources;                        ///< Map of modulation source names to Outputs.
        std::map<std::string, std::unique_ptr<StatusOutput>> status_outputs; ///< Map of status outputs.
        input_map mono_mod_destinations;               ///< Map of mono modulation destinations.
        input_map poly_mod_destinations;               ///< Map of poly modulation destinations.
        output_map mono_modulation_readout;            ///< Outputs used to read mono modulation totals.
        output_map poly_modulation_readout;            ///< Outputs used to read poly modulation totals.
        std::map<std::string, ValueSwitch*> mono_modulation_switches; ///< Mono modulation switches.
        std::map<std::string, ValueSwitch*> poly_modulation_switches; ///< Poly modulation switches.

        JUCE_LEAK_DETECTOR(ModuleData)
    };

    /**
     * @class SynthModule
     * @brief A ProcessorRouter that encapsulates a cohesive unit of functionality in the synthesizer.
     *
     * A SynthModule groups multiple processors and controls together. It can be nested within other
     * SynthModules, forming a hierarchical structure. It manages mono and poly modulation sources
     * and destinations, provides access to controls, and allows enabling/disabling sets of processors.
     */
    class SynthModule : public ProcessorRouter {
    public:
        /**
         * @brief Constructs a SynthModule with specified I/O and control rate.
         * @param num_inputs Number of inputs to the module.
         * @param num_outputs Number of outputs from the module.
         * @param control_rate True if the module operates at control rate, false for audio rate.
         */
        SynthModule(int num_inputs, int num_outputs, bool control_rate = false) :
                ProcessorRouter(num_inputs, num_outputs, control_rate) {
            data_ = std::make_shared<ModuleData>();
        }

        /**
         * @brief Destructor.
         */
        virtual ~SynthModule() { }

        /**
         * @brief Returns a map of all controls from this module and its submodules.
         * @return A control_map containing all controls.
         */
        control_map getControls();

        /**
         * @brief Retrieves a modulation source output by name.
         * @param name The name of the modulation source.
         * @return A pointer to the Output or nullptr if not found.
         */
        Output* getModulationSource(std::string name);

        /**
         * @brief Retrieves a StatusOutput by name.
         * @param name The name of the status output.
         * @return A pointer to the StatusOutput or nullptr if not found.
         */
        const StatusOutput* getStatusOutput(std::string name) const;

        /**
         * @brief Retrieves a modulation destination Processor by name and poly mode.
         * @param name The name of the modulation destination.
         * @param poly True for polyphonic, false for monophonic.
         * @return A pointer to the Processor or nullptr if not found.
         */
        Processor* getModulationDestination(std::string name, bool poly);

        /**
         * @brief Retrieves a mono modulation destination by name.
         * @param name The name of the destination.
         * @return A pointer to the Processor or nullptr if not found.
         */
        Processor* getMonoModulationDestination(std::string name);

        /**
         * @brief Retrieves a poly modulation destination by name.
         * @param name The name of the destination.
         * @return A pointer to the Processor or nullptr if not found.
         */
        Processor* getPolyModulationDestination(std::string name);

        /**
         * @brief Retrieves a modulation switch by name and poly mode.
         * @param name The name of the modulation switch.
         * @param poly True for polyphonic, false for monophonic.
         * @return A pointer to the ValueSwitch or nullptr if not found.
         */
        ValueSwitch* getModulationSwitch(std::string name, bool poly);

        /**
         * @brief Retrieves a mono modulation switch by name.
         * @param name The name of the modulation switch.
         * @return A pointer to the ValueSwitch or nullptr if not found.
         */
        ValueSwitch* getMonoModulationSwitch(std::string name);

        /**
         * @brief Retrieves a poly modulation switch by name.
         * @param name The name of the modulation switch.
         * @return A pointer to the ValueSwitch or nullptr if not found.
         */
        ValueSwitch* getPolyModulationSwitch(std::string name);

        /**
         * @brief Updates all modulation switches based on whether their destinations have inputs.
         */
        void updateAllModulationSwitches();

        /**
         * @brief Returns a reference to the map of modulation sources.
         * @return A reference to the output_map of sources.
         */
        output_map& getModulationSources();

        /**
         * @brief Returns a reference to the map of mono modulation destinations.
         * @return A reference to the input_map of mono destinations.
         */
        input_map& getMonoModulationDestinations();

        /**
         * @brief Returns a reference to the map of poly modulation destinations.
         * @return A reference to the input_map of poly destinations.
         */
        input_map& getPolyModulationDestinations();

        /**
         * @brief Returns a reference to the map of mono modulation readouts.
         * @return A reference to the output_map of mono modulation readouts.
         */
        virtual output_map& getMonoModulations();

        /**
         * @brief Returns a reference to the map of poly modulation readouts.
         * @return A reference to the output_map of poly modulation readouts.
         */
        virtual output_map& getPolyModulations();

        /**
         * @brief Allows correction of module state to a given time (if needed).
         * @param seconds The time to correct to, in seconds.
         *
         * Override this method in subclasses if required.
         */
        virtual void correctToTime(double seconds) { }

        /**
         * @brief Enables or disables all owned processors.
         * @param enable True to enable, false to disable.
         */
        void enableOwnedProcessors(bool enable);

        /**
         * @brief Enables or disables this SynthModule and its owned processors.
         * @param enable True to enable, false to disable.
         */
        virtual void enable(bool enable) override;

        /**
         * @brief Adds a mono processor to this module.
         * @param processor The Processor to add.
         * @param own True if this module should take ownership of the processor.
         */
        void addMonoProcessor(Processor* processor, bool own = true);

        /**
         * @brief Adds a mono processor that is considered idle (not part of main processing chain).
         * @param processor The Processor to add.
         */
        void addIdleMonoProcessor(Processor* processor);

        /**
         * @brief Clones this SynthModule.
         * @return A new SynthModule instance that is a copy of this one.
         */
        virtual Processor* clone() const override { return new SynthModule(*this); }

        /**
         * @brief Adds a submodule to this SynthModule.
         * @param module The submodule to add.
         */
        void addSubmodule(SynthModule* module) { data_->sub_modules.push_back(module); }

    protected:
        /**
         * @brief Creates a simple control processor for a given parameter name.
         * @param name The name of the parameter.
         * @param audio_rate True if this control should run at audio rate, false otherwise.
         * @param smooth_value True if the control should be smoothed over time.
         * @return A pointer to the created Value processor.
         */
        Value* createBaseControl(std::string name, bool audio_rate = false, bool smooth_value = false);

        /**
         * @brief Creates a base mod control, which is a control combined with a modulation input.
         * @param name The parameter name.
         * @param audio_rate True if audio-rate control.
         * @param smooth_value True if smoothing is desired.
         * @param internal_modulation Optional internal modulation source.
         * @return The resulting Output that represents the modulated control.
         */
        Output* createBaseModControl(std::string name, bool audio_rate = false,
                                     bool smooth_value = false, Output* internal_modulation = nullptr);

        /**
         * @brief Creates a monophonic mod control, including applying parameter scaling.
         * @param name The parameter name.
         * @param audio_rate True if audio-rate control.
         * @param smooth_value True if smoothing is desired.
         * @param internal_modulation Optional internal modulation source.
         * @return The resulting Output that represents the modulated and scaled control.
         */
        Output* createMonoModControl(std::string name, bool audio_rate = false,
                                     bool smooth_value = false, Output* internal_modulation = nullptr);

        /**
         * @brief Creates a polyphonic mod control, including applying parameter scaling.
         * @param name The parameter name.
         * @param audio_rate True if audio-rate control.
         * @param smooth_value True if smoothing is desired.
         * @param internal_modulation Optional internal modulation source.
         * @param reset An optional Input reset trigger for polyphonic modulation.
         * @return The resulting Output that represents the modulated and scaled control.
         */
        Output* createPolyModControl(std::string name, bool audio_rate = false,
                                     bool smooth_value = false, Output* internal_modulation = nullptr, Input* reset = nullptr);

        /**
         * @brief Creates a tempo sync switch that toggles between tempo-based frequency and free-running frequency.
         * @param name The parameter name prefix.
         * @param frequency A Processor representing the base frequency.
         * @param beats_per_second Output representing beats per second (tempo).
         * @param poly True if polyphonic.
         * @param midi Optional Input for MIDI data, used for keytracking.
         * @return The resulting Output of the tempo sync switch.
         */
        Output* createTempoSyncSwitch(std::string name, Processor* frequency,
                                      const Output* beats_per_second, bool poly, Input* midi = nullptr);

        /**
         * @brief Creates a status output associated with a given Output.
         * @param name The status output name.
         * @param source The source Output to track.
         */
        void createStatusOutput(std::string name, Output* source);

        std::shared_ptr<ModuleData> data_; ///< Shared data storage for this SynthModule.

        JUCE_LEAK_DETECTOR(SynthModule)
    };
} // namespace vital
