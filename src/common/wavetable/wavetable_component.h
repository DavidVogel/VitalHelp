/*
Summary:
WavetableComponent is the base class for elements that produce or modify wavetables. It manages keyframes representing waveform states at certain positions, and supports interpolation between these states using none, linear, or cubic methods. By serializing and deserializing keyframes, it integrates smoothly with preset systems. Interpolation ensures smooth transitions across the wavetable dimension, enabling dynamic and evolving sounds.
 */

#pragma once

#include "JuceHeader.h"
#include "wavetable_keyframe.h"
#include "wavetable_component_factory.h"
#include "json/json.h"

using json = nlohmann::json;

namespace vital {
    class WaveFrame;
}

/**
 * @brief A base class representing a component in a wavetable synthesis chain.
 *
 * WavetableComponent manages a collection of WavetableKeyframes, each representing a waveform configuration at a certain
 * position (time/frame index). By interpolating between these keyframes, the component produces a smooth transition
 * of waveform shapes across the wavetable dimension. WavetableComponents can represent sources of waveforms, or
 * modifiers that apply transformations to existing waveforms.
 *
 * Key functionalities include:
 * - Creating and managing keyframes.
 * - Interpolating between keyframes using linear or cubic styles.
 * - Serialization to/from JSON for preset saving.
 */
class WavetableComponent {
public:
    /**
     * @enum InterpolationStyle
     * @brief Defines how interpolation is performed between keyframes.
     */
    enum InterpolationStyle {
        kNone,    ///< No interpolation, just jumps between keyframes.
        kLinear,  ///< Linear interpolation between adjacent keyframes.
        kCubic,   ///< Cubic interpolation for smoother transitions.
        kNumInterpolationStyles
    };

    /**
     * @brief Constructs a WavetableComponent with a default linear interpolation style.
     */
    WavetableComponent() : interpolation_style_(kLinear) { }
    virtual ~WavetableComponent() { }

    /**
     * @brief Creates a new keyframe at a given position.
     *
     * Implemented by subclasses to produce a keyframe type suited to their functionality.
     *
     * @param position The wavetable frame index (0 to kNumOscillatorWaveFrames-1).
     * @return A pointer to the newly created WavetableKeyframe.
     */
    virtual WavetableKeyframe* createKeyframe(int position) = 0;

    /**
     * @brief Renders the waveform at a given position into a WaveFrame.
     *
     * Uses interpolation between keyframes based on the current interpolation style to produce a waveform for the given position.
     *
     * @param wave_frame The WaveFrame to fill with the resulting waveform.
     * @param position The position along the wavetable dimension [0, kNumOscillatorWaveFrames-1].
     */
    virtual void render(vital::WaveFrame* wave_frame, float position) = 0;

    /**
     * @brief Returns the type of this WavetableComponent.
     *
     * Used to identify the specific component for serialization and UI mapping.
     *
     * @return The component type enumerated in WavetableComponentFactory.
     */
    virtual WavetableComponentFactory::ComponentType getType() = 0;

    /**
     * @brief Serializes the component’s state and all keyframes to a JSON object.
     *
     * @return A JSON object representing the entire state.
     */
    virtual json stateToJson();

    /**
     * @brief Restores the component’s state from a JSON object.
     *
     * Clears existing keyframes and reconstructs them from the provided JSON data.
     *
     * @param data The JSON object containing saved state data.
     */
    virtual void jsonToState(json data);

    /**
     * @brief Called before rendering to perform any needed precomputation.
     *
     * By default does nothing, can be overridden by subclasses.
     */
    virtual void prerender() { }

    /**
     * @brief Indicates whether this component relies on multiple keyframes.
     *
     * @return True by default, can be overridden by subclasses that do not use keyframes.
     */
    virtual bool hasKeyframes() { return true; }

    /**
     * @brief Clears all keyframes and inserts a default one at position 0.
     */
    void reset();

    /**
     * @brief Interpolates a destination keyframe at a given position.
     *
     * Uses the current interpolation style (none, linear, cubic) to populate `dest` with data
     * interpolated from existing keyframes.
     *
     * @param dest The destination keyframe to fill.
     * @param position The position along the wavetable to interpolate at.
     */
    void interpolate(WavetableKeyframe* dest, float position);

    /**
     * @brief Inserts a new keyframe at the given position, creating and sorting it into the array.
     *
     * @param position The position along the wavetable dimension.
     * @return The newly created WavetableKeyframe.
     */
    WavetableKeyframe* insertNewKeyframe(int position);

    /**
     * @brief Repositions a keyframe in the keyframe list after its position changed.
     *
     * Removes and re-inserts it in sorted order by position.
     *
     * @param keyframe The keyframe to reposition.
     */
    void reposition(WavetableKeyframe* keyframe);

    /**
     * @brief Removes a specific keyframe from the component.
     *
     * @param keyframe The keyframe to remove.
     */
    void remove(WavetableKeyframe* keyframe);

    /**
     * @brief Gets the number of keyframes.
     *
     * @return The number of keyframes.
     */
    inline int numFrames() const { return static_cast<int>(keyframes_.size()); }

    /**
     * @brief Finds the index of a given keyframe.
     *
     * @param keyframe The keyframe pointer to find.
     * @return The index of the keyframe or -1 if not found.
     */
    inline int indexOf(WavetableKeyframe* keyframe) const {
        for (int i = 0; i < keyframes_.size(); ++i) {
            if (keyframes_[i].get() == keyframe)
                return i;
        }
        return -1;
    }

    /**
     * @brief Gets a keyframe by index.
     *
     * @param index The keyframe index.
     * @return The WavetableKeyframe at that index.
     */
    inline WavetableKeyframe* getFrameAt(int index) const { return keyframes_[index].get(); }

    /**
     * @brief Finds the insertion index for a given position to keep keyframes sorted.
     *
     * @param position The position along the wavetable.
     * @return The index where a keyframe with this position should be inserted.
     */
    int getIndexFromPosition(int position) const;

    /**
     * @brief Gets a keyframe by position if one matches exactly, or nullptr otherwise.
     *
     * @param position The position to search for.
     * @return The keyframe at that position or nullptr if none matches.
     */
    WavetableKeyframe* getFrameAtPosition(int position);

    /**
     * @brief Gets the highest position among all keyframes.
     *
     * If no keyframes exist, returns 0. If the component doesn't use keyframes, returns the last possible frame index.
     *
     * @return The position of the last keyframe.
     */
    int getLastKeyframePosition();

    /**
     * @brief Sets the global interpolation style.
     *
     * @param type The InterpolationStyle (none, linear, cubic).
     */
    void setInterpolationStyle(InterpolationStyle type) { interpolation_style_ = type; }

    /**
     * @brief Gets the current global interpolation style.
     *
     * @return The InterpolationStyle currently used.
     */
    InterpolationStyle getInterpolationStyle() const { return interpolation_style_; }

protected:
    std::vector<std::unique_ptr<WavetableKeyframe>> keyframes_; ///< The list of keyframes sorted by position.
    InterpolationStyle interpolation_style_; ///< Current interpolation style (none, linear, cubic).

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavetableComponent)
};
