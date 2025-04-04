/*
Summary:
SlewLimitModifier applies slew-rate limiting to the wavetable’s time-domain signal, controlling how quickly the waveform can rise or fall per sample. By adjusting the upward and downward slew parameters, one can achieve smoother transitions and reduce harsh dynamics in the resulting sound. Interpolating between keyframes allows these constraints to evolve across the wavetable.
 */

#pragma once

#include "JuceHeader.h"
#include "wavetable_component.h"

/**
 * @brief A WavetableComponent that applies slew-rate limiting to a wave’s time-domain signal.
 *
 * SlewLimitModifier imposes a maximum rate at which the waveform’s amplitude can change between samples.
 * By limiting the upward and downward rate of change (slew), it can soften transients and produce smoother
 * waveform transitions. Users can control separate upward and downward slew limits.
 */
class SlewLimitModifier : public WavetableComponent {
public:
    /**
     * @brief A keyframe class holding parameters for slew-rate limits at a given position.
     *
     * The SlewLimitModifierKeyframe stores two values: `slew_up_run_rise_` and `slew_down_run_rise_`,
     * which represent inverse slew limits for upward and downward amplitude changes respectively.
     * Interpolating between keyframes allows changing these limits gradually across a wavetable.
     */
    class SlewLimitModifierKeyframe : public WavetableKeyframe {
    public:
        /**
         * @brief Constructs a keyframe with default slew limit values.
         */
        SlewLimitModifierKeyframe();
        virtual ~SlewLimitModifierKeyframe() { }

        void copy(const WavetableKeyframe* keyframe) override;
        void interpolate(const WavetableKeyframe* from_keyframe,
                         const WavetableKeyframe* to_keyframe, float t) override;
        void render(vital::WaveFrame* wave_frame) override;
        json stateToJson() override;
        void jsonToState(json data) override;

        /**
         * @brief Gets the upward slew limit.
         *
         * This limit defines how quickly amplitude can rise from one sample to the next.
         *
         * @return The upward slew limit parameter.
         */
        float getSlewUpLimit() { return slew_up_run_rise_; }

        /**
         * @brief Gets the downward slew limit.
         *
         * This limit defines how quickly amplitude can fall from one sample to the next.
         *
         * @return The downward slew limit parameter.
         */
        float getSlewDownLimit() { return slew_down_run_rise_; }

        /**
         * @brief Sets the upward slew limit parameter.
         *
         * @param slew_up_limit The new upward slew limit.
         */
        void setSlewUpLimit(float slew_up_limit) { slew_up_run_rise_ = slew_up_limit; }

        /**
         * @brief Sets the downward slew limit parameter.
         *
         * @param slew_down_limit The new downward slew limit.
         */
        void setSlewDownLimit(float slew_down_limit) { slew_down_run_rise_ = slew_down_limit; }

    protected:
        float slew_up_run_rise_;   ///< Parameter controlling upward slew rate.
        float slew_down_run_rise_; ///< Parameter controlling downward slew rate.

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SlewLimitModifierKeyframe)
    };

    /**
     * @brief Constructs a SlewLimitModifier.
     *
     * By default, no slew limits are applied until a keyframe is configured.
     */
    SlewLimitModifier() { }
    virtual ~SlewLimitModifier() { }

    WavetableKeyframe* createKeyframe(int position) override;
    void render(vital::WaveFrame* wave_frame, float position) override;
    WavetableComponentFactory::ComponentType getType() override;

    /**
     * @brief Retrieves a SlewLimitModifierKeyframe by index.
     *
     * @param index The index of the keyframe to retrieve.
     * @return A pointer to the corresponding SlewLimitModifierKeyframe.
     */
    SlewLimitModifierKeyframe* getKeyframe(int index);

protected:
    SlewLimitModifierKeyframe compute_frame_; ///< A keyframe used for intermediate computations.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SlewLimitModifier)
};
