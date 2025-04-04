/*
Summary:
WaveWarpModifier allows nonlinear reshaping of a waveform by warping it horizontally (time-axis) and vertically (amplitude-axis). Users can set warp powers and choose asymmetric transformations, creating a wide range of waveform distortions. Interpolation between keyframes and these nonlinear transformations can produce evolving, dynamic waves that move beyond simple linear scaling.
 */

#pragma once

#include "JuceHeader.h"
#include "wavetable_component.h"

/**
 * @brief A WavetableComponent that applies nonlinear horizontal and vertical warping to a waveform.
 *
 * WaveWarpModifier deforms the waveform’s time-domain shape by stretching or compressing the wave along both
 * the horizontal (time) and vertical (amplitude) axes. It uses exponential-like mappings to create nonlinear
 * transformations. Users can control two independent warp powers:
 * - Horizontal power: adjusts how the sample positions are warped along the waveform cycle.
 * - Vertical power: adjusts how the waveform amplitude is warped.
 *
 * Furthermore, the warping can be symmetric or asymmetric:
 * - Asymmetric warping: transforms only one side (e.g., positive or negative half) more strongly than the other.
 * - Symmetric warping: applies a balanced transformation around a center point.
 */
class WaveWarpModifier : public WavetableComponent {
public:
    /**
     * @brief A keyframe class holding horizontal and vertical warp parameters at a given position.
     *
     * WaveWarpModifierKeyframe stores two float parameters, `horizontal_power_` and `vertical_power_`, which control
     * how strongly the waveform is warped along the horizontal (time) and vertical (amplitude) axes.
     * By interpolating between keyframes, we can smoothly transition from one warp setting to another.
     *
     * Asymmetric flags allow treating the waveform differently in its positive/negative or left/right sections.
     */
    class WaveWarpModifierKeyframe : public WavetableKeyframe {
    public:
        /**
         * @brief Constructs a WaveWarpModifierKeyframe with default warp powers and symmetric warping.
         */
        WaveWarpModifierKeyframe();
        virtual ~WaveWarpModifierKeyframe() { }

        void copy(const WavetableKeyframe* keyframe) override;
        void interpolate(const WavetableKeyframe* from_keyframe,
                         const WavetableKeyframe* to_keyframe, float t) override;
        void render(vital::WaveFrame* wave_frame) override;
        json stateToJson() override;
        void jsonToState(json data) override;

        /**
         * @brief Gets the horizontal warp power.
         *
         * @return The horizontal warp power.
         */
        float getHorizontalPower() { return horizontal_power_; }

        /**
         * @brief Gets the vertical warp power.
         *
         * @return The vertical warp power.
         */
        float getVerticalPower() { return vertical_power_; }

        /**
         * @brief Sets the horizontal warp power.
         *
         * @param horizontal_power The new horizontal warp power.
         */
        void setHorizontalPower(float horizontal_power) { horizontal_power_ = horizontal_power; }

        /**
         * @brief Sets the vertical warp power.
         *
         * @param vertical_power The new vertical warp power.
         */
        void setVerticalPower(float vertical_power) { vertical_power_ = vertical_power; }

        /**
         * @brief Sets whether horizontal warping is asymmetric.
         *
         * If true, the transformation differs for values on different sides of the waveform.
         *
         * @param horizontal_asymmetric True for asymmetric, false for symmetric.
         */
        void setHorizontalAsymmetric(bool horizontal_asymmetric) { horizontal_asymmetric_ = horizontal_asymmetric; }

        /**
         * @brief Sets whether vertical warping is asymmetric.
         *
         * @param vertical_asymmetric True for asymmetric, false for symmetric.
         */
        void setVerticalAsymmetric(bool vertical_asymmetric) { vertical_asymmetric_ = vertical_asymmetric; }

    protected:
        float horizontal_power_; ///< Controls horizontal (time-axis) warping.
        float vertical_power_;   ///< Controls vertical (amplitude-axis) warping.

        bool horizontal_asymmetric_; ///< If true, horizontal warping is asymmetric.
        bool vertical_asymmetric_;   ///< If true, vertical warping is asymmetric.

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveWarpModifierKeyframe)
    };

    /**
     * @brief Constructs a WaveWarpModifier with symmetric warping by default.
     */
    WaveWarpModifier() : horizontal_asymmetric_(false), vertical_asymmetric_(false) { }
    virtual ~WaveWarpModifier() = default;

    WavetableKeyframe* createKeyframe(int position) override;
    void render(vital::WaveFrame* wave_frame, float position) override;
    WavetableComponentFactory::ComponentType getType() override;
    json stateToJson() override;
    void jsonToState(json data) override;

    /**
     * @brief Sets whether horizontal warping is asymmetric.
     *
     * @param horizontal_asymmetric True for asymmetric, false for symmetric.
     */
    void setHorizontalAsymmetric(bool horizontal_asymmetric) { horizontal_asymmetric_ = horizontal_asymmetric; }

    /**
     * @brief Sets whether vertical warping is asymmetric.
     *
     * @param vertical_asymmetric True for asymmetric, false for symmetric.
     */
    void setVerticalAsymmetric(bool vertical_asymmetric) { vertical_asymmetric_ = vertical_asymmetric; }

    /**
     * @brief Checks if horizontal warping is asymmetric.
     *
     * @return True if horizontally asymmetric, false otherwise.
     */
    bool getHorizontalAsymmetric() const { return horizontal_asymmetric_; }

    /**
     * @brief Checks if vertical warping is asymmetric.
     *
     * @return True if vertically asymmetric, false otherwise.
     */
    bool getVerticalAsymmetric() const { return vertical_asymmetric_; }

    /**
     * @brief Retrieves a WaveWarpModifierKeyframe by index.
     *
     * @param index The index of the desired keyframe.
     * @return A pointer to the WaveWarpModifierKeyframe.
     */
    WaveWarpModifierKeyframe* getKeyframe(int index);

protected:
    WaveWarpModifierKeyframe compute_frame_; ///< Keyframe used for intermediate computation.
    bool horizontal_asymmetric_;             ///< Controls horizontal warping symmetry.
    bool vertical_asymmetric_;               ///< Controls vertical warping symmetry.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveWarpModifier)
};
