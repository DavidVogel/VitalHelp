/*
Summary:
WaveWindowModifier applies a chosen windowing function to the beginning and end of a waveform, tapering its amplitude based on left_position_ and right_position_. Different window shapes (cosine, half-sine, linear, square, wiggle) produce various slopes and transitions. By interpolating between keyframes, users can animate the window size and shape over the wavetable, influencing how the waveform’s amplitude envelope evolves dynamically.
 */

#pragma once

#include "JuceHeader.h"
#include "wavetable_component.h"

/**
 * @brief A WavetableComponent that applies a windowing function to a waveform’s head and tail.
 *
 * WaveWindowModifier uses a specified window shape (cosine, half-sine, linear, square, or "wiggle")
 * to smoothly taper the waveform’s amplitude near its start and end. By adjusting the window’s left
 * and right positions, the user can control how much of the wave is affected and how abruptly it
 * transitions at the boundaries.
 */
class WaveWindowModifier : public WavetableComponent {
public:
    /**
     * @enum WindowShape
     * @brief Defines different windowing curves.
     */
    enum WindowShape {
        kCos,      ///< A cosine-based window (smooth rise/fall).
        kHalfSin,  ///< A half-sine window shape.
        kLinear,   ///< A linear ramp window.
        kSquare,   ///< A sudden step (square) window.
        kWiggle,   ///< A more complex wiggle-based shape.
        kNumWindowShapes
    };

    /**
     * @brief Applies the chosen window shape to a given normalized position.
     *
     * @param window_shape The WindowShape to apply.
     * @param t A normalized time parameter [0,1].
     * @return The windowed amplitude at time t.
     */
    static float applyWindow(WindowShape window_shape, float t);

    /**
     * @brief A keyframe class that stores the window shape and positions at a given table position.
     *
     * The WaveWindowModifierKeyframe holds parameters for the left and right positions along the wave
     * where the window begins and ends, as well as which window shape to use. Interpolating between
     * keyframes allows the windowed portion of the wave to change shape or size smoothly across the
     * wavetable dimension.
     */
    class WaveWindowModifierKeyframe : public WavetableKeyframe {
    public:
        /**
         * @brief Constructs a WaveWindowModifierKeyframe with a default window shape and positions.
         */
        WaveWindowModifierKeyframe();
        virtual ~WaveWindowModifierKeyframe() { }

        void copy(const WavetableKeyframe* keyframe) override;
        void interpolate(const WavetableKeyframe* from_keyframe,
                         const WavetableKeyframe* to_keyframe, float t) override;
        void render(vital::WaveFrame* wave_frame) override;
        json stateToJson() override;
        void jsonToState(json data) override;

        /**
         * @brief Sets the left position of the window (0 to 1).
         *
         * The portion of the wave before this position will be gradually introduced from 0 amplitude.
         *
         * @param left The new left position.
         */
        void setLeft(float left) { left_position_ = left; }

        /**
         * @brief Sets the right position of the window (0 to 1).
         *
         * The portion of the wave after this position will be gradually tapered to 0 amplitude.
         *
         * @param right The new right position.
         */
        void setRight(float right) { right_position_ = right; }

        /**
         * @brief Gets the left window position.
         *
         * @return The left position parameter.
         */
        float getLeft() { return left_position_; }

        /**
         * @brief Gets the right window position.
         *
         * @return The right position parameter.
         */
        float getRight() { return right_position_; }

        /**
         * @brief Sets the window shape for this keyframe.
         *
         * @param window_shape The WindowShape to apply.
         */
        void setWindowShape(WindowShape window_shape) { window_shape_ = window_shape; }

    protected:
        /**
         * @brief Applies the currently selected window shape at a given normalized time t.
         *
         * @param t The normalized time parameter [0,1].
         * @return The windowed amplitude.
         */
        inline float applyWindow(float t) { return WaveWindowModifier::applyWindow(window_shape_, t); }

        float left_position_;  ///< The left boundary of the windowing region.
        float right_position_; ///< The right boundary of the windowing region.
        WindowShape window_shape_; ///< The chosen window shape for this keyframe.

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveWindowModifierKeyframe)
    };

    /**
     * @brief Constructs a WaveWindowModifier with a default cosine window shape.
     */
    WaveWindowModifier() : window_shape_(kCos) { }
    virtual ~WaveWindowModifier() { }

    WavetableKeyframe* createKeyframe(int position) override;
    void render(vital::WaveFrame* wave_frame, float position) override;
    WavetableComponentFactory::ComponentType getType() override;
    json stateToJson() override;
    void jsonToState(json data) override;

    /**
     * @brief Retrieves a WaveWindowModifierKeyframe by index.
     *
     * @param index The index of the desired keyframe.
     * @return A pointer to the WaveWindowModifierKeyframe.
     */
    WaveWindowModifierKeyframe* getKeyframe(int index);

    /**
     * @brief Sets the window shape used by this modifier.
     *
     * @param window_shape The WindowShape to use.
     */
    void setWindowShape(WindowShape window_shape) { window_shape_ = window_shape; }

    /**
     * @brief Gets the current window shape.
     *
     * @return The current WindowShape.
     */
    WindowShape getWindowShape() { return window_shape_; }

protected:
    WaveWindowModifierKeyframe compute_frame_; ///< A keyframe for intermediate computations.
    WindowShape window_shape_;                 ///< The global window shape used.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveWindowModifier)
};
