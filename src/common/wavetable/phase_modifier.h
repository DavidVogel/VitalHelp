/*
Summary:
PhaseModifier manipulates the phase relationships among a wavetable’s harmonic components to achieve various timbral effects. By selecting a PhaseStyle and controlling parameters like phase offset and mix, it can produce harmonic shifts, alternate even/odd phase patterns, or clear all phase differences. Interpolating between keyframes allows for dynamic phase evolutions across the wavetable’s dimension.
 */

#pragma once

#include "JuceHeader.h"
#include "wavetable_component.h"

/**
 * @brief A WavetableComponent that modifies the phase of frequency components in a wavetable frame.
 *
 * PhaseModifier adjusts the phase relationships between harmonic components of a wave.
 * Different phase styles can produce effects such as shifting all harmonics uniformly,
 * applying alternate phase shifts to even/odd harmonics, or completely clearing phase information.
 * It allows partial blending of the phase modification using a mix parameter.
 */
class PhaseModifier : public WavetableComponent {
  public:
    /**
     * @enum PhaseStyle
     * @brief Various methods to modify harmonic phase.
     */
    enum PhaseStyle {
        kNormal,          ///< Apply a harmonic phase shift cumulatively up the harmonic series.
        kEvenOdd,         ///< Apply different phase treatments to even and odd harmonics.
        kHarmonic,        ///< Uniformly shift phase for all harmonics directly.
        kHarmonicEvenOdd, ///< Apply a harmonic-based shift with separate handling of even/odd harmonics.
        kClear,           ///< Clear phase information, making all harmonics in phase.
        kNumPhaseStyles
    };

    /**
     * @brief A keyframe class holding parameters for phase modification at a given position.
     *
     * PhaseModifierKeyframe stores a phase offset and a mix amount. It can apply various phase
     * modification styles to a WaveFrame’s frequency domain. Interpolation between keyframes
     * allows animating phase changes over a wavetable.
     */
    class PhaseModifierKeyframe : public WavetableKeyframe {
      public:
        /**
         * @brief Constructs a keyframe with default phase and mix values.
         */
        PhaseModifierKeyframe();
        virtual ~PhaseModifierKeyframe() { }

        void copy(const WavetableKeyframe* keyframe) override;
        void interpolate(const WavetableKeyframe* from_keyframe,
                         const WavetableKeyframe* to_keyframe, float t) override;
        void render(vital::WaveFrame* wave_frame) override;
        json stateToJson() override;
        void jsonToState(json data) override;

        /**
         * @brief Gets the phase offset applied to harmonics.
         *
         * @return The phase offset in radians.
         */
        float getPhase() { return phase_; }

        /**
         * @brief Gets the mix ratio blending between original and modified phases.
         *
         * @return The mix ratio (0 to 1).
         */
        float getMix() { return mix_; }

        /**
         * @brief Sets the phase offset (in radians).
         *
         * @param phase The new phase offset.
         */
        void setPhase(float phase) { phase_ = phase; }

        /**
         * @brief Sets the mix ratio.
         *
         * @param mix The blend between original and modified signal.
         */
        void setMix(float mix) { mix_ = mix; }

        /**
         * @brief Sets the phase modification style.
         *
         * @param style The chosen PhaseStyle.
         */
        void setPhaseStyle(PhaseStyle style) { phase_style_ = style; }

      protected:
        float phase_;       ///< The phase offset in radians.
        float mix_;         ///< The blend between original and modified harmonic phases.
        PhaseStyle phase_style_; ///< Selected phase modification style.

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaseModifierKeyframe)
    };

    /**
     * @brief Constructs a PhaseModifier with a default normal phase style.
     */
    PhaseModifier() : phase_style_(kNormal) { }
    virtual ~PhaseModifier() = default;

    virtual WavetableKeyframe* createKeyframe(int position) override;
    virtual void render(vital::WaveFrame* wave_frame, float position) override;
    virtual WavetableComponentFactory::ComponentType getType() override;
    virtual json stateToJson() override;
    virtual void jsonToState(json data) override;

    /**
     * @brief Retrieves a PhaseModifierKeyframe by index.
     *
     * @param index The index of the desired keyframe.
     * @return Pointer to the PhaseModifierKeyframe.
     */
    PhaseModifierKeyframe* getKeyframe(int index);

    /**
     * @brief Sets the style of phase modification.
     *
     * @param style The chosen PhaseStyle.
     */
    void setPhaseStyle(PhaseStyle style) { phase_style_ = style; }

        /**
     * @brief Gets the current phase modification style.
     *
     * @return The current PhaseStyle.
     */
    PhaseStyle getPhaseStyle() const { return phase_style_; }

  protected:
    PhaseModifierKeyframe compute_frame_; ///< A keyframe for intermediate computation.
    PhaseStyle phase_style_;              ///< The selected style of phase modification.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaseModifier)
};

