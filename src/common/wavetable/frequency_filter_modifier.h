/*
Summary:
FrequencyFilterModifier is a wavetable component that modifies the harmonic content of a wave by applying frequency-domain filters like low-pass, band-pass, high-pass, or comb filtering. FrequencyFilterModifierKeyframe stores parameters (cutoff, shape) that define how frequencies are allowed or attenuated. By interpolating between keyframes and optionally normalizing the result, this component enables flexible and expressive frequency shaping of wavetables.
 */

#pragma once

#include "JuceHeader.h"
#include "wavetable_component.h"

/**
 * @brief A WavetableComponent that applies frequency-domain filtering to a wavetable frame.
 *
 * The FrequencyFilterModifier allows shaping the frequency content of a wave by applying a low-pass,
 * band-pass, high-pass, or comb filter directly to the frequency domain of a wavetable frame. It can
 * also optionally normalize the resulting frame to maintain consistent amplitude.
 */
class FrequencyFilterModifier : public WavetableComponent {
  public:
    /**
     * @enum FilterStyle
     * @brief Specifies the type of frequency-domain filter to apply.
     */
    enum FilterStyle {
        kLowPass,   ///< Attenuates frequencies above a cutoff.
        kBandPass,  ///< Allows frequencies around the cutoff and attenuates others.
        kHighPass,  ///< Attenuates frequencies below a cutoff.
        kComb,      ///< A comb-like filter pattern in frequency domain.
        kNumFilterStyles
    };

    /**
     * @brief A keyframe class for frequency filtering parameters at a given position.
     *
     * Each keyframe stores the filter style, cutoff frequency, and shape (slope) of the filter. It applies
     * these parameters to a WaveFrame, modifying its frequency-domain content and optionally normalizing
     * the result.
     */
    class FrequencyFilterModifierKeyframe : public WavetableKeyframe {
      public:
        /**
         * @brief Constructs a FrequencyFilterModifierKeyframe with default parameters.
         */
        FrequencyFilterModifierKeyframe();
        virtual ~FrequencyFilterModifierKeyframe() { }

        void copy(const WavetableKeyframe* keyframe) override;
        void interpolate(const WavetableKeyframe* from_keyframe,
                         const WavetableKeyframe* to_keyframe, float t) override;
        void render(vital::WaveFrame* wave_frame) override;
        json stateToJson() override;
        void jsonToState(json data) override;

        /**
         * @brief Computes a frequency-domain multiplier for a given frequency index.
         *
         * Determines how strongly a particular frequency bin is attenuated or allowed based on cutoff and shape.
         *
         * @param index The frequency bin index.
         * @return A multiplier for the frequency bin amplitude.
         */
        float getMultiplier(float index);

        float getCutoff() { return cutoff_; }
        float getShape() { return shape_; }

        void setStyle(FilterStyle style) { style_ = style; }
        void setCutoff(float cutoff) { cutoff_ = cutoff; }
        void setShape(float shape) { shape_ = shape; }
        void setNormalize(bool normalize) { normalize_ = normalize; }

      protected:
        FilterStyle style_;  ///< Current filter style applied by this keyframe.
        bool normalize_;     ///< Whether to normalize the wave after filtering.
        float cutoff_;       ///< Cutoff frequency parameter (logarithmic scale).
        float shape_;        ///< Shape or slope of the filter attenuation.

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FrequencyFilterModifierKeyframe)
      };

    /**
     * @brief Constructs a FrequencyFilterModifier with default low-pass style and normalization enabled.
     */
      FrequencyFilterModifier() : style_(kLowPass), normalize_(true) { }
      virtual ~FrequencyFilterModifier() { }

      virtual WavetableKeyframe* createKeyframe(int position) override;
      virtual void render(vital::WaveFrame* wave_frame, float position) override;
      virtual WavetableComponentFactory::ComponentType getType() override;
      virtual json stateToJson() override;
      virtual void jsonToState(json data) override;

      FrequencyFilterModifierKeyframe* getKeyframe(int index);

      FilterStyle getStyle() { return style_; }
      bool getNormalize() { return normalize_; }

      void setStyle(FilterStyle style) { style_ = style; }
      void setNormalize(bool normalize) { normalize_ = normalize; }

    protected:
    FilterStyle style_;                     ///< The filtering style currently used.
    bool normalize_;                        ///< Whether to normalize waves after filtering.
    FrequencyFilterModifierKeyframe compute_frame_; ///< A keyframe used for intermediate computation.

      JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FrequencyFilterModifier)
};

