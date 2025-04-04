/*
Summary:
The Parameters and ValueDetailsLookup classes centralize all parameter definitions for Vital. They define the behavior, ranges, and displayed names/units of every parameter (e.g., oscillators, filters, LFOs, envelopes). The code organizes parameters into groups (for envelopes, LFOs, oscillators, filters, and more) and ensures compatibility across versions. This system allows for easy retrieval of parameter metadata throughout the synthesizer’s code and UI layers.
 */

#pragma once

#include "common.h"

#include <map>
#include <string>

namespace vital {

    /**
     * @brief Holds metadata about a single parameter (control) in the Vital synthesizer.
     *
     * The ValueDetails structure describes how a parameter should be scaled, displayed,
     * and interpreted. This includes minimum/maximum values, default values, display names,
     * units, and the scaling function to apply (linear, exponential, etc.). It also supports
     * indexed parameters for stepping through discrete values (e.g., enumeration fields).
     */
    struct ValueDetails {
        /**
         * @enum ValueScale
         * @brief Describes the scaling mode used to interpret and display parameter values.
         */
        enum ValueScale {
            kIndexed,    ///< Parameter steps through discrete indexed values.
            kLinear,     ///< Parameter scales linearly between min and max.
            kQuadratic,  ///< Parameter value transformed by a quadratic curve.
            kCubic,      ///< Parameter value transformed by a cubic curve.
            kQuartic,    ///< Parameter value transformed by a quartic curve.
            kSquareRoot, ///< Parameter value transformed by a square root function.
            kExponential ///< Parameter value transformed by an exponential function.
        };

        std::string name;               ///< Unique parameter name/identifier.
        int version_added = 0;          ///< Version code when the parameter was introduced or changed.
        mono_float min = 0.0f;          ///< Minimum parameter value.
        mono_float max = 1.0f;          ///< Maximum parameter value.
        mono_float default_value = 0.0f;///< Default value for the parameter.
        mono_float post_offset = 0.0f;  ///< Offset applied after scaling (for certain scale types).
        mono_float display_multiply = 1.0f; ///< Multiplier for converting internal values to display units.
        ValueScale value_scale = kLinear;   ///< The scaling mode of the parameter value.
        bool display_invert = false;        ///< If true, invert the displayed value range.
        std::string display_units;          ///< Units to display next to the parameter (e.g., "Hz", "dB").
        std::string display_name;           ///< Human-readable name for display in UI.
        const std::string* string_lookup = nullptr; ///< Optional lookup table for indexed parameter names.
        std::string local_description;      ///< Local description or additional metadata.
    } typedef ValueDetails;

    /**
     * @brief Maintains a lookup table for all parameters defined in Vital.
     *
     * ValueDetailsLookup stores and indexes all parameter metadata (ValueDetails) used
     * by the synthesizer. It supports:
     * - Retrieving parameter details by name or index.
     * - Checking if a given name is a parameter.
     * - Adding parameter groups (like envelopes, LFOs, etc.) with prefixed names and IDs.
     * - Accessing parameter ranges and display names.
     */
    class ValueDetailsLookup {
    public:
        /**
         * @brief Constructs a ValueDetailsLookup and initializes its parameter tables.
         *
         * Loads predefined parameters and organizes them into lookup tables.
         */
        ValueDetailsLookup();

        /**
         * @brief Checks if a given name corresponds to a known parameter.
         *
         * @param name The parameter name to check.
         * @return True if the parameter exists, false otherwise.
         */
        const bool isParameter(const std::string& name) const {
            return details_lookup_.count(name);
        }

        /**
         * @brief Retrieves ValueDetails for a given parameter name.
         *
         * @param name The parameter name.
         * @return A const reference to the associated ValueDetails.
         */
        const ValueDetails& getDetails(const std::string& name) const {
            auto details = details_lookup_.find(name);
            VITAL_ASSERT(details != details_lookup_.end());
            return details->second;
        }

        /**
         * @brief Retrieves ValueDetails by parameter index.
         *
         * @param index The parameter index (based on loading order).
         * @return A pointer to the ValueDetails, or nullptr if out of range.
         */
        const ValueDetails* getDetails(int index) const {
            return details_list_[index];
        }

        /**
         * @brief Gets a human-readable display name for a parameter.
         *
         * @param name The parameter name.
         * @return A string with the display name.
         */
        std::string getDisplayName(const std::string& name) const {
            return getDetails(name).display_name;
        }

        /**
         * @brief Gets the number of parameters defined.
         *
         * @return The total number of parameters.
         */
        int getNumParameters() const {
            return static_cast<int>(details_list_.size());
        }

        /**
         * @brief Gets the full parameter range (max - min).
         *
         * @param name The parameter name.
         * @return The range of the parameter as a mono_float.
         */
        mono_float getParameterRange(const std::string& name) const {
            auto details = details_lookup_.find(name);
            VITAL_ASSERT(details != details_lookup_.end());
            return details->second.max - details->second.min;
        }

        /**
         * @brief Returns a copy of all parameter details in a map.
         *
         * @return A map of parameter names to their ValueDetails.
         */
        std::map<std::string, ValueDetails> getAllDetails() const {
            return details_lookup_;
        }

        /**
         * @brief Adds a group of parameters, each with prefixed names and display names.
         *
         * Used to create sets of parameters for envelopes, LFOs, oscillators, etc.
         *
         * @param list Array of ValueDetails to add.
         * @param num_parameters Number of parameters in the list.
         * @param index Index to append to parameter names and display names.
         * @param id_prefix Prefix for ID (e.g., "env", "lfo").
         * @param name_prefix Prefix for display name (e.g., "Envelope", "LFO").
         * @param version Optional version override if these parameters were added/modified later.
         */
        void addParameterGroup(const ValueDetails* list, int num_parameters, int index,
                               std::string id_prefix, std::string name_prefix, int version = -1);

        /**
         * @brief Adds a group of parameters using a string-based ID rather than numeric index.
         *
         * @param list Array of ValueDetails to add.
         * @param num_parameters Number of parameters in the list.
         * @param id A string ID to append.
         * @param id_prefix Prefix for ID.
         * @param name_prefix Prefix for display name.
         * @param version Optional version override.
         */
        void addParameterGroup(const ValueDetails* list, int num_parameters, std::string id,
                               std::string id_prefix, std::string name_prefix, int version = -1);

        // Static arrays of parameter definitions for different categories.
        static const ValueDetails parameter_list[];
        static const ValueDetails env_parameter_list[];
        static const ValueDetails lfo_parameter_list[];
        static const ValueDetails random_lfo_parameter_list[];
        static const ValueDetails filter_parameter_list[];
        static const ValueDetails osc_parameter_list[];
        static const ValueDetails mod_parameter_list[];

    private:
        std::map<std::string, ValueDetails> details_lookup_; ///< Mapping from parameter names to details.
        std::vector<const ValueDetails*> details_list_;      ///< Ordered list of parameter pointers.

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueDetailsLookup)
    };

    /**
     * @brief A static utility class to access parameter details globally.
     *
     * Parameters provides static methods to:
     * - Access parameter details by name or index.
     * - Check if a parameter exists.
     * - Get parameter display names and ranges.
     *
     * It uses a static ValueDetailsLookup (lookup_) internally.
     */
    class Parameters {
    public:
        static const ValueDetails& getDetails(const std::string& name) {
            return lookup_.getDetails(name);
        }

        static int getNumParameters() {
            return lookup_.getNumParameters();
        }

        static const ValueDetails* getDetails(int index) {
            return lookup_.getDetails(index);
        }

        static std::string getDisplayName(const std::string& name) {
            return lookup_.getDisplayName(name);
        }

        static const mono_float getParameterRange(const std::string& name) {
            return lookup_.getParameterRange(name);
        }

        static const bool isParameter(const std::string& name) {
            return lookup_.isParameter(name);
        }

        static std::map<std::string, ValueDetails> getAllDetails() {
            return lookup_.getAllDetails();
        }

        static ValueDetailsLookup lookup_;

    private:
        Parameters() { } ///< Private constructor prevents instantiation.
    };

} // namespace vital
