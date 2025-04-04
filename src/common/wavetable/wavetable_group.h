/*
Summary:
WavetableGroup manages a collection of WavetableComponents, each potentially generating or modifying waveforms. It allows adding, removing, and reordering components, provides methods to render their combined output at any position in the wavetable, and handles serialization. By grouping components, it supports layering and complex combinations that form part of a larger WavetableCreator pipeline.
 */

#pragma once

#include "JuceHeader.h"
#include "wave_frame.h"
#include "wavetable_component.h"

namespace vital {
    class Wavetable;
} // namespace vital

/**
 * @brief A class representing a group of WavetableComponents combined to form part of a wavetable.
 *
 * A WavetableGroup holds multiple WavetableComponents (sources or modifiers) that work together to generate
 * or transform a waveform. By combining and averaging their outputs, the group produces a single resulting
 * waveform for any given position. WavetableGroups can be stacked by the WavetableCreator to form complex,
 * layered sounds.
 */
class WavetableGroup {
public:
    /**
     * @brief Constructs an empty WavetableGroup.
     */
    WavetableGroup() { }

    /**
     * @brief Gets the index of a particular WavetableComponent within this group.
     *
     * @param component The component to find.
     * @return The index of the component or -1 if not found.
     */
    int getComponentIndex(WavetableComponent* component);

    /**
     * @brief Adds a new WavetableComponent to this group.
     *
     * @param component The WavetableComponent to add (transfers ownership).
     */
    void addComponent(WavetableComponent* component) {
        components_.push_back(std::unique_ptr<WavetableComponent>(component));
    }

    /**
     * @brief Removes a component at a given index.
     *
     * @param index The index of the component to remove.
     */
    void removeComponent(int index);

    /**
     * @brief Moves a component one position up in the ordering.
     *
     * @param index The index of the component to move.
     */
    void moveUp(int index);

    /**
     * @brief Moves a component one position down in the ordering.
     *
     * @param index The index of the component to move.
     */
    void moveDown(int index);

    /**
     * @brief Clears all components and loads a default group configuration.
     */
    void reset();

    /**
     * @brief Allows components to precompute any necessary data before rendering.
     */
    void prerender();

    /**
     * @brief Gets the number of WavetableComponents in this group.
     *
     * @return The number of components.
     */
    int numComponents() const { return static_cast<int>(components_.size()); }

    /**
     * @brief Retrieves a component by index.
     *
     * @param index The index of the component.
     * @return A pointer to the WavetableComponent at that index.
     */
    WavetableComponent* getComponent(int index) const { return components_[index].get(); }

    /**
     * @brief Determines if all components in this group produce a Shepard tone.
     *
     * This is used to check if the entire group is producing a special "shepard tone" type table.
     *
     * @return True if all components are Shepard tone sources, false otherwise.
     */
    bool isShepardTone();

    /**
     * @brief Renders the combined waveform for a given position from all components.
     *
     * Each component is rendered and their outputs are combined to form a single WaveFrame result.
     *
     * @param wave_frame The WaveFrame to fill with the resulting waveform.
     * @param position The position along the wavetable dimension.
     */
    void render(vital::WaveFrame* wave_frame, float position) const;

    /**
     * @brief Renders the entire group directly into a Wavetable object.
     *
     * This fills all frames of the Wavetable.
     *
     * @param wavetable The Wavetable to fill.
     */
    void renderTo(vital::Wavetable* wavetable);

    /**
     * @brief Loads a default group configuration (e.g., a basic wave source).
     */
    void loadDefaultGroup();

    /**
     * @brief Gets the largest keyframe position among all components in the group.
     *
     * @return The highest keyframe position found.
     */
    int getLastKeyframePosition();

    /**
     * @brief Serializes this group's state, including all its components, to JSON.
     *
     * @return A JSON object representing the group's state.
     */
    json stateToJson();

    /**
     * @brief Restores this group's state from a JSON object.
     *
     * Clears existing components and rebuilds them from the JSON data.
     *
     * @param data The JSON object containing the group's saved state.
     */
    void jsonToState(json data);

protected:
    vital::WaveFrame compute_frame_; ///< Temporary WaveFrame for combining component outputs.
    std::vector<std::unique_ptr<WavetableComponent>> components_; ///< The list of components in this group.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavetableGroup)
};
