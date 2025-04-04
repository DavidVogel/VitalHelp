#pragma once

#include "JuceHeader.h"
#include "default_look_and_feel.h"

/**
 * @class TextLookAndFeel
 * @brief A custom look and feel class that renders UI elements with text-focused styling.
 *
 * This class overrides certain drawing methods to provide a more text-oriented aesthetic for
 * sliders, toggle buttons, and combo boxes. It relies on text and font adjustments rather than
 * heavy graphical elements.
 */
class TextLookAndFeel : public DefaultLookAndFeel {
public:
    /**
     * @brief Destructor.
     */
    virtual ~TextLookAndFeel() { }

    /**
     * @brief Draws a rotary slider using a text-focused design.
     *
     * Instead of showing a traditional rotary arc, this implementation draws text in the center
     * to represent the slider's value.
     *
     * @param g          Graphics context to draw into.
     * @param x          Left x-coordinate of slider area.
     * @param y          Top y-coordinate of slider area.
     * @param width      Width of the slider area.
     * @param height     Height of the slider area.
     * @param slider_t   Current slider value mapped 0.0 to 1.0.
     * @param start_angle Starting angle of the slider arc (unused).
     * @param end_angle   Ending angle of the slider arc (unused).
     * @param slider      The slider to draw.
     */
    void drawRotarySlider(Graphics& g, int x, int y, int width, int height,
                          float slider_t, float start_angle, float end_angle,
                          Slider& slider) override;

    /**
     * @brief Draws a toggle button with a text-centered style.
     *
     * The button appearance changes based on toggle state, hover, and pressed states.
     * If associated with a SynthButton, it can lookup display strings from a provided array.
     *
     * @param g        Graphics context.
     * @param button   The toggle button to draw.
     * @param hover    Whether the mouse is hovering over the button.
     * @param is_down  Whether the button is currently pressed.
     */
    void drawToggleButton(Graphics& g, ToggleButton& button, bool hover, bool is_down) override;

    /**
     * @brief Draws a tick box (for checkboxes) with minimal text-focused styling.
     *
     * By default, only changes appear if toggled (e.g., filling a rectangle).
     *
     * @param g          Graphics context.
     * @param component  The parent component of the tick box.
     * @param x          Left x-coordinate of the tick box.
     * @param y          Top y-coordinate of the tick box.
     * @param w          Width of the tick box.
     * @param h          Height of the tick box.
     * @param ticked     Whether the box is ticked.
     * @param enabled    Whether the box is enabled.
     * @param mouse_over Whether the mouse is over the box.
     * @param button_downWhether the box is pressed.
     */
    void drawTickBox(Graphics& g, Component& component, float x, float y, float w, float h, bool ticked,
                     bool enabled, bool mouse_over, bool button_down) override;

    /**
     * @brief Draws a label with text-focused style.
     *
     * Sets the label's text color to match the general body text color.
     *
     * @param g     Graphics context.
     * @param label The label to draw.
     */
    void drawLabel(Graphics& g, Label& label) override;

    /**
     * @brief Draws a combo box with text styling.
     *
     * Uses text and simple backgrounds rather than heavy graphics.
     *
     * @param g         Graphics context.
     * @param width     Width of the combo box.
     * @param height    Height of the combo box.
     * @param is_down   Whether the combo box button is pressed down.
     * @param button_x  Button region x.
     * @param button_y  Button region y.
     * @param button_w  Button region width.
     * @param button_h  Button region height.
     * @param box       The combo box to draw.
     */
    void drawComboBox(Graphics& g, int width, int height, bool is_down,
                      int button_x, int button_y, int button_w, int button_h, ComboBox& box) override;

    /**
     * @brief Singleton instance access.
     *
     * @return A pointer to the TextLookAndFeel instance.
     */
    static TextLookAndFeel* instance() {
        static TextLookAndFeel instance;
        return &instance;
    }

private:
    /**
     * @brief Private constructor to enforce singleton pattern.
     */
    TextLookAndFeel();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TextLookAndFeel)
};
