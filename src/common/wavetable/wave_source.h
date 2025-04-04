/*
Summary:
WaveSource and WaveSourceKeyframe provide a direct representation of wavetables via WaveFrames. By storing entire waveforms and supporting time or frequency domain interpolation (including cubic methods for smoothness), they enable flexible and sophisticated transitions between keyframes. This results in dynamic and evolving sounds that can be tailored by controlling interpolation modes and parameters.
 */

#pragma once

#include "JuceHeader.h"
#include "wavetable_component.h"
#include "wave_frame.h"

class WaveSourceKeyframe;

/**
 * @brief A WavetableComponent that acts as a direct source of waveforms.
 *
 * WaveSource stores a series of WaveSourceKeyframes, each holding a WaveFrame (time and frequency domain data).
 * It supports two interpolation modes:
 * - Time interpolation: Interpolates directly between time-domain waveforms.
 * - Frequency interpolation: Interpolates between frequency-domain representations, allowing smoother transitions
 *   with more control over harmonic phases and amplitudes.
 *
 * This component provides a flexible basis for representing waves as raw data and morphing between them.
 */
class WaveSource : public WavetableComponent {
  public:
    /**
     * @enum InterpolationMode
     * @brief Defines how keyframes should be interpolated.
     */
    enum InterpolationMode {
        kTime,      ///< Interpolate directly in time domain.
        kFrequency  ///< Interpolate in frequency domain.
    };

    /**
     * @brief Constructs a WaveSource with a default frequency-domain interpolation mode.
     */
    WaveSource();
    virtual ~WaveSource();

    WavetableKeyframe* createKeyframe(int position) override;
    void render(vital::WaveFrame* wave_frame, float position) override;
    WavetableComponentFactory::ComponentType getType() override;
    json stateToJson() override;
    void jsonToState(json data) override;

    /**
     * @brief Gets a WaveFrame from a specified keyframe index.
     *
     * @param index The index of the desired keyframe.
     * @return A pointer to the WaveFrame associated with the keyframe.
     */
    vital::WaveFrame* getWaveFrame(int index);

    /**
     * @brief Retrieves a WaveSourceKeyframe by index.
     *
     * @param index The index of the desired keyframe.
     * @return A pointer to the WaveSourceKeyframe at that index.
     */
    WaveSourceKeyframe* getKeyframe(int index);

    /**
     * @brief Sets the interpolation mode for morphing between keyframes.
     *
     * @param mode The InterpolationMode to use (time or frequency).
     */
    void setInterpolationMode(InterpolationMode mode) { interpolation_mode_ = mode; }

    /**
     * @brief Gets the current interpolation mode.
     *
     * @return The current InterpolationMode.
     */
    InterpolationMode getInterpolationMode() const { return interpolation_mode_; }

protected:
    std::unique_ptr<WaveSourceKeyframe> compute_frame_; ///< A keyframe for intermediate interpolation computations.
    InterpolationMode interpolation_mode_;              ///< The mode of interpolation.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveSource)
};

/**
 * @brief A keyframe that holds a single WaveFrame and supports various interpolation methods.
 *
 * A WaveSourceKeyframe stores a WaveFrame that can represent a full cycle of a waveform in both time and frequency domains.
 * It offers functions for linear and cubic interpolation in either time or frequency domains, enabling smooth transitions
 * between different waveforms stored in other keyframes.
 */
class WaveSourceKeyframe : public WavetableKeyframe {
public:
    /**
     * @brief Constructs a WaveSourceKeyframe with frequency-domain interpolation mode by default.
     */
    WaveSourceKeyframe() : interpolation_mode_(WaveSource::kFrequency) {
        wave_frame_ = std::make_unique<vital::WaveFrame>();
    }
    virtual ~WaveSourceKeyframe() { }

    /**
     * @brief Provides direct access to the stored WaveFrame.
     *
     * @return A pointer to the WaveFrame held by this keyframe.
     */
    vital::WaveFrame* wave_frame() { return wave_frame_.get(); }

    void copy(const WavetableKeyframe* keyframe) override;

    /**
     * @brief Linearly interpolate two WaveFrames in the time domain.
     *
     * @param from The starting WaveFrame.
     * @param to The ending WaveFrame.
     * @param t The interpolation factor [0,1].
     */
    void linearTimeInterpolate(const vital::WaveFrame* from, const vital::WaveFrame* to, float t);

    /**
     * @brief Cubic interpolation in time domain using four WaveFrames for smooth transitions.
     *
     * @param prev The previous WaveFrame.
     * @param from The starting WaveFrame.
     * @param to The target WaveFrame.
     * @param next The next WaveFrame after 'to'.
     * @param range_prev The distance (in keyframe positions) to prev.
     * @param range The distance to to.
     * @param range_next The distance to next.
     * @param t The interpolation factor [0,1].
     */
    void cubicTimeInterpolate(const vital::WaveFrame* prev, const vital::WaveFrame* from,
                              const vital::WaveFrame* to, const vital::WaveFrame* next,
                              float range_prev, float range, float range_next, float t);

    /**
     * @brief Linear interpolation in frequency domain.
     *
     * Interpolates amplitudes and phases of harmonics for a smoother spectral transition.
     *
     * @param from The starting WaveFrame.
     * @param to The ending WaveFrame.
     * @param t The interpolation factor [0,1].
     */
    void linearFrequencyInterpolate(const vital::WaveFrame* from, const vital::WaveFrame* to, float t);

    /**
     * @brief Cubic interpolation in frequency domain using four WaveFrames.
     *
     * Allows for very smooth harmonic transitions across multiple keyframes.
     *
     * @param prev The previous WaveFrame.
     * @param from The starting WaveFrame.
     * @param to The target WaveFrame.
     * @param next The next WaveFrame.
     * @param range_prev The distance to prev.
     * @param range The distance to to.
     * @param range_next The distance to next.
     * @param t The interpolation factor [0,1].
     */
    void cubicFrequencyInterpolate(const vital::WaveFrame* prev, const vital::WaveFrame* from,
                                   const vital::WaveFrame* to, const vital::WaveFrame* next,
                                   float range_prev, float range, float range_next, float t);

    void interpolate(const WavetableKeyframe* from_keyframe, const WavetableKeyframe* to_keyframe, float t) override;
    void smoothInterpolate(const WavetableKeyframe* prev_keyframe,
                           const WavetableKeyframe* from_keyframe,
                           const WavetableKeyframe* to_keyframe,
                           const WavetableKeyframe* next_keyframe, float t) override;

    /**
     * @brief Renders the WaveFrame into the provided wave_frame without modification.
     *
     * @param wave_frame The WaveFrame to copy into.
     */
    void render(vital::WaveFrame* wave_frame) override {
        wave_frame->copy(wave_frame_.get());
    }

    json stateToJson() override;
    void jsonToState(json data) override;

    /**
     * @brief Sets the interpolation mode for this keyframe.
     *
     * @param mode The interpolation mode (time or frequency).
     */
    void setInterpolationMode(WaveSource::InterpolationMode mode) { interpolation_mode_ = mode; }

    /**
     * @brief Gets the current interpolation mode for this keyframe.
     *
     * @return The interpolation mode currently in use.
     */
    WaveSource::InterpolationMode getInterpolationMode() const { return interpolation_mode_; }

protected:
    std::unique_ptr<vital::WaveFrame> wave_frame_;         ///< The WaveFrame representing this keyframe’s waveform.
    WaveSource::InterpolationMode interpolation_mode_;     ///< The mode (time or frequency) used for this keyframe's interpolation.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveSourceKeyframe)
};
