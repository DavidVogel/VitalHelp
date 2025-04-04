#pragma once

#include "JuceHeader.h"
#include "default_look_and_feel.h"

/**
 * @class CurveLookAndFeel
 * @brief A specialized LookAndFeel class for drawing curve-shaped rotary sliders.
 *
 * This LookAndFeel renders a rotary slider as a power-scale curve instead of a standard arc.
 * The curve shape can represent a parameter's response curve visually.
 * It supports both active/inactive states and bipolar values.
 */
class CurveLookAndFeel : public DefaultLookAndFeel {
public:
    /** @brief Destructor. */
    virtual ~CurveLookAndFeel() { }

    /**
     * @brief Draws a rotary slider with a curve-shaped indicator.
     * @param g The Graphics context.
     * @param x The X coordinate of the slider's bounding box.
     * @param y The Y coordinate of the slider's bounding box.
     * @param width The width of the slider's bounding box.
     * @param height The height of the slider's bounding box.
     * @param slider_t The slider's current normalized value (0 to 1).
     * @param start_angle The start angle for the rotary slider (not used for this curve).
     * @param end_angle The end angle for the rotary slider (not used for this curve).
     * @param slider Reference to the slider being drawn.
     */
    void drawRotarySlider(Graphics& g, int x, int y, int width, int height,
                          float slider_t, float start_angle, float end_angle,
                          Slider& slider) override;

    /**
     * @brief Draws the power-scale curve for the slider.
     * @param g The Graphics context.
     * @param slider Reference to the Slider being drawn.
     * @param x The X coordinate of the curve area.
     * @param y The Y coordinate of the curve area.
     * @param width The width of the curve area.
     * @param height The height of the curve area.
     * @param active True if the slider is active (enabled).
     * @param bipolar True if the slider value is bipolar (e.g., can be negative).
     */
    void drawCurve(Graphics& g, Slider& slider, int x, int y, int width, int height, bool active, bool bipolar);

    /**
     * @brief Gets the singleton instance of CurveLookAndFeel.
     * @return Pointer to the singleton instance.
     */
    static CurveLookAndFeel* instance() {
        static CurveLookAndFeel instance;
        return &instance;
    }

private:
    /** @brief Private constructor for singleton pattern. */
    CurveLookAndFeel();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CurveLookAndFeel)
};
