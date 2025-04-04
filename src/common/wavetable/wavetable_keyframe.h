/*
Summary:
WavetableKeyframe defines a single point in a wavetable where the waveform configuration is known. By interpolating between keyframes, a component can produce evolving waveforms. This class provides generic interpolation (linear and cubic) and serialization methods, while subclasses specify how waveform data is stored and rendered.
 */

#pragma once

#include "JuceHeader.h"
#include "json/json.h"
#include "synth_constants.h"

using json = nlohmann::json;

class WavetableComponent;

namespace vital {
    class WaveFrame;
} // namespace vital

/**
 * @brief Represents a single state of a waveform at a specific position in a wavetable.
 *
 * WavetableKeyframe acts as a snapshot of a waveform configuration at a particular frame index.
 * Wavetable components typically have multiple keyframes spaced across the wavetable dimension.
 * Interpolating between these keyframes produces smooth changes in the waveform over the course
 * of the table.
 *
 * Each keyframe stores:
 * - A position within the wavetable (0 to kNumOscillatorWaveFrames-1).
 * - A reference to its owning component.
 *
 * Subclasses define how the keyframe’s waveform data is stored and how interpolation and rendering
 * are performed.
 */
class WavetableKeyframe {
public:
    /**
     * @brief Performs linear interpolation between two points.
     *
     * @param point_from The starting value.
     * @param point_to The ending value.
     * @param t The interpolation factor [0,1].
     * @return The interpolated value.
     */
    static float linearTween(float point_from, float point_to, float t);

    /**
     * @brief Performs cubic interpolation taking into account a previous and next point for smoother curves.
     *
     * @param point_prev The value before the start.
     * @param point_from The starting value.
     * @param point_to The ending value.
     * @param point_next The value after the end.
     * @param range_prev The horizontal distance to point_prev.
     * @param range The horizontal distance between point_from and point_to.
     * @param range_next The horizontal distance to point_next.
     * @param t The interpolation factor [0,1].
     * @return The smoothly interpolated value.
     */
    static float cubicTween(float point_prev, float point_from, float point_to, float point_next,
                            float range_prev, float range, float range_next, float t);

    /**
     * @brief Constructs a WavetableKeyframe with a default position of 0 and no owner.
     */
    WavetableKeyframe() : position_(0), owner_(nullptr) { }
    virtual ~WavetableKeyframe() { }

    /**
     * @brief Gets the index of this keyframe within its owner component.
     *
     * @return The index of the keyframe or -1 if not found.
     */
    int index();

    /**
     * @brief Gets the wavetable frame position of this keyframe.
     *
     * @return The frame position (0 to kNumOscillatorWaveFrames-1).
     */
    int position() const { return position_; }

    /**
     * @brief Sets the frame position of this keyframe.
     *
     * @param position The new frame position.
     */
    void setPosition(int position) {
        VITAL_ASSERT(position >= 0 && position < vital::kNumOscillatorWaveFrames);
        position_ = position;
    }

    /**
     * @brief Copies the state from another keyframe of the same type.
     *
     * @param keyframe The source keyframe to copy from.
     */
    virtual void copy(const WavetableKeyframe* keyframe) = 0;

    /**
     * @brief Linearly interpolates between two keyframes.
     *
     * @param from_keyframe The starting keyframe.
     * @param to_keyframe The ending keyframe.
     * @param t The interpolation factor [0,1].
     */
    virtual void interpolate(const WavetableKeyframe* from_keyframe,
                             const WavetableKeyframe* to_keyframe, float t) = 0;

    /**
     * @brief Performs a smooth (cubic) interpolation using four keyframes for even smoother transitions.
     *
     * By default does nothing. Subclasses can implement cubic interpolation if desired.
     *
     * @param prev_keyframe The keyframe before from_keyframe.
     * @param from_keyframe The starting keyframe.
     * @param to_keyframe The ending keyframe.
     * @param next_keyframe The keyframe after to_keyframe.
     * @param t The interpolation factor [0,1].
     */
    virtual void smoothInterpolate(const WavetableKeyframe* prev_keyframe,
                                   const WavetableKeyframe* from_keyframe,
                                   const WavetableKeyframe* to_keyframe,
                                   const WavetableKeyframe* next_keyframe, float t) { }

    /**
     * @brief Renders the waveform of this keyframe into a WaveFrame.
     *
     * @param wave_frame The WaveFrame to fill with this keyframe's waveform data.
     */
    virtual void render(vital::WaveFrame* wave_frame) = 0;

    /**
     * @brief Serializes the state of this keyframe to a JSON object.
     *
     * @return A JSON object representing the keyframe.
     */
    virtual json stateToJson();

    /**
     * @brief Restores the keyframe's state from a JSON object.
     *
     * @param data The JSON object containing the keyframe's state.
     */
    virtual void jsonToState(json data);

    /**
     * @brief Gets the WavetableComponent that owns this keyframe.
     *
     * @return A pointer to the owner component.
     */
    WavetableComponent* owner() { return owner_; }

    /**
     * @brief Sets the owner of this keyframe.
     *
     * @param owner The WavetableComponent that owns this keyframe.
     */
    void setOwner(WavetableComponent* owner) { owner_ = owner; }

protected:
    int position_;             ///< The position of this keyframe along the wavetable dimension.
    WavetableComponent* owner_;///< The component that owns this keyframe.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavetableKeyframe)
};
