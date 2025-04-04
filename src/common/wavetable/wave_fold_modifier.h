/*
Summary:
WaveFoldModifier applies a nonlinear wave-folding effect to a wavetable’s time-domain waveform. By scaling and folding the waveform through sine and arcsine functions, it adds harmonic complexity and can create interesting timbral variations. Adjusting the fold boost parameter in keyframes and interpolating between them allows dynamic control over the amount of wave folding across the wavetable.
 */

#pragma once

#include "JuceHeader.h"
#include "wavetable_component.h"

/**
 * @brief A WavetableComponent that applies a wave-folding transformation to a waveform.
 *
 * WaveFoldModifier nonlinearly warps the waveform’s amplitude values, using a wave-folding approach
 * reminiscent of analog wavefolders. By boosting and folding the waveform, it can add rich harmonic
 * content and transform simple waves into more complex timbres. The amount of folding is controlled
 * by a "fold boost" parameter in the keyframes.
 */
class WaveFoldModifier : public WavetableComponent {
public:
    /**
     * @brief A keyframe class that stores the fold boost parameter for wave-folding at a given position.
     *
     * The WaveFoldModifierKeyframe contains a single parameter, `wave_fold_boost_`, which determines
     * how aggressively the wave is folded. Interpolating between keyframes allows for dynamic changes
     * in folding amount across the wavetable.
     */
    class WaveFoldModifierKeyframe : public WavetableKeyframe {
    public:
        /**
         * @brief Constructs a keyframe with a default wave fold boost value of 1.0.
         */
        WaveFoldModifierKeyframe();
        virtual ~WaveFoldModifierKeyframe() { }

        void copy(const WavetableKeyframe* keyframe) override;
        void interpolate(const WavetableKeyframe* from_keyframe,
                         const WavetableKeyframe* to_keyframe, float t) override;
        void render(vital::WaveFrame* wave_frame) override;
        json stateToJson() override;
        void jsonToState(json data) override;

        /**
         * @brief Gets the current wave fold boost factor.
         *
         * @return The wave fold boost factor.
         */
        float getWaveFoldBoost() { return wave_fold_boost_; }

        /**
         * @brief Sets a new wave fold boost factor.
         *
         * Higher values mean more intense wave folding.
         *
         * @param boost The new fold boost factor.
         */
        void setWaveFoldBoost(float boost) { wave_fold_boost_ = boost; }

    protected:
        float wave_fold_boost_; ///< The factor by which the wave is folded.

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveFoldModifierKeyframe)
    };

    /**
     * @brief Constructs a WaveFoldModifier with no additional initialization.
     */
    WaveFoldModifier() { }
    virtual ~WaveFoldModifier() { }

    WavetableKeyframe* createKeyframe(int position) override;
    void render(vital::WaveFrame* wave_frame, float position) override;
    WavetableComponentFactory::ComponentType getType() override;

    /**
     * @brief Retrieves a WaveFoldModifierKeyframe at a given index.
     *
     * @param index The index of the desired keyframe.
     * @return A pointer to the WaveFoldModifierKeyframe.
     */
    WaveFoldModifierKeyframe* getKeyframe(int index);

protected:
    WaveFoldModifierKeyframe compute_frame_; ///< A keyframe for intermediate computations.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveFoldModifier)
};
