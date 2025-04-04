/*
Summary:
WavetableComponentFactory provides a centralized means to instantiate various WavetableComponents, both sources and modifiers, and to map between enumerated types, string names, and constructed objects. This makes it easier to integrate new components, handle component-specific UI or preset logic, and maintain a clean separation of concerns for creation and naming of WavetableComponents.
 */

#pragma once

#include "JuceHeader.h"

class WavetableComponent;

/**
 * @brief Factory class for creating and identifying different types of WavetableComponents.
 *
 * WavetableComponentFactory allows creation of source and modifier components by type enumerations or by name strings.
 * It provides:
 * - A set of enumerated ComponentTypes representing both wave sources and waveform modifiers.
 * - Functions to create a component instance from a ComponentType or a string name.
 * - Methods to retrieve human-readable names of components.
 * - Separations of components into source and modifier categories.
 */
class WavetableComponentFactory {
public:
    /**
     * @enum ComponentType
     * @brief Enumerates all known WavetableComponents, including sources and modifiers.
     */
    enum ComponentType {
        kWaveSource,        ///< A basic wave source.
        kLineSource,        ///< A line-based wave source.
        kFileSource,        ///< A file-based audio source.
        kNumSourceTypes,    ///< Marks the end of source types.
        kShepardToneSource = kNumSourceTypes, // Deprecated Shepard tone source.

        kBeginModifierTypes = kNumSourceTypes + 1,
        kPhaseModifier = kBeginModifierTypes, ///< Modifier that shifts phase.
        kWaveWindow,                          ///< Modifier that applies window functions to the wave.
        kFrequencyFilter,                     ///< Modifier that filters frequency components.
        kSlewLimiter,                         ///< Modifier that limits slew rate.
        kWaveFolder,                          ///< Modifier that applies wave folding.
        kWaveWarp,                            ///< Modifier that warps the waveform.
        kNumComponentTypes                    ///< Total count of all component types.
    };

    /**
     * @brief Returns the total number of component types defined.
     *
     * @return The total count of component types.
     */
    static int numComponentTypes() { return kNumComponentTypes; }

    /**
     * @brief Returns the number of source types defined.
     *
     * @return The number of source component types.
     */
    static int numSourceTypes() { return kNumSourceTypes; }

    /**
     * @brief Returns the number of modifier types defined.
     *
     * @return The number of modifier component types.
     */
    static int numModifierTypes() { return kNumComponentTypes - kBeginModifierTypes; }

    /**
     * @brief Creates a new WavetableComponent instance of a given enumerated type.
     *
     * @param type The ComponentType enumerated value.
     * @return A pointer to the newly created WavetableComponent.
     */
    static WavetableComponent* createComponent(ComponentType type);

    /**
     * @brief Creates a new WavetableComponent instance from a name string.
     *
     * @param type The component name as a string.
     * @return A pointer to the newly created WavetableComponent.
     */
    static WavetableComponent* createComponent(const std::string& type);

    /**
     * @brief Gets the human-readable name of a component from its enumerated type.
     *
     * @param type The ComponentType enumerated value.
     * @return The corresponding component name as a std::string.
     */
    static std::string getComponentName(ComponentType type);

    /**
     * @brief Converts an integer index to a source ComponentType.
     *
     * @param type The integer index representing a source type.
     * @return The corresponding ComponentType.
     */
    static ComponentType getSourceType(int type) { return static_cast<ComponentType>(type); }

    /**
     * @brief Converts an integer index to a modifier ComponentType.
     *
     * @param type The integer index for modifier types.
     * @return The corresponding ComponentType for a modifier.
     */
    static ComponentType getModifierType(int type) {
        return (ComponentType)(type + kBeginModifierTypes);
    }

private:
    WavetableComponentFactory() { }
};
