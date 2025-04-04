#pragma once

#include "JuceHeader.h"
#include "skin.h"

/**
 * @class IncrementerButtons
 * @brief A pair of buttons for incrementing and decrementing a Slider's value.
 *
 * The IncrementerButtons class provides two small triangular arrow buttons,
 * one for incrementing and one for decrementing the value of an associated
 * Slider. When either button is pressed, the Slider's value is adjusted by
 * one unit. The appearance of the buttons adapts to the currently active skin
 * colors, and can be toggled active or inactive.
 */
class IncrementerButtons : public Component, public Button::Listener {
public:
    /**
     * @brief Constructs the IncrementerButtons object.
     * @param slider A pointer to a Slider whose value will be modified when
     *               these buttons are pressed.
     */
    IncrementerButtons(Slider* slider) : slider_(slider), active_(true) {
        increment_ = std::make_unique<ShapeButton>("Increment", Colours::black, Colours::black, Colours::black);
        addAndMakeVisible(increment_.get());
        increment_->addListener(this);

        // Define the shape for the "increment" arrow (pointing up).
        Path increment_shape;
        increment_shape.startNewSubPath(Point<float>(0.5f, 0.1f));
        increment_shape.lineTo(Point<float>(0.2f, 0.45f));
        increment_shape.lineTo(Point<float>(0.8f, 0.45f));
        increment_shape.closeSubPath();

        // Add some dummy segments to ensure proper rendering.
        increment_shape.startNewSubPath(Point<float>(0.0f, 0.0f));
        increment_shape.closeSubPath();
        increment_shape.startNewSubPath(Point<float>(1.0f, 0.5f));
        increment_shape.closeSubPath();

        increment_shape.addLineSegment(Line<float>(0.0f, 0.0f, 0.0f, 0.0f), 0.2f);
        increment_shape.addLineSegment(Line<float>(0.5f, 0.5f, 0.5f, 0.5f), 0.2f);
        increment_->setShape(increment_shape, true, true, false);

        decrement_ = std::make_unique<ShapeButton>("Decrement", Colours::black, Colours::black, Colours::black);
        addAndMakeVisible(decrement_.get());
        decrement_->addListener(this);

        // Define the shape for the "decrement" arrow (pointing down).
        Path decrement_shape;
        decrement_shape.startNewSubPath(Point<float>(0.5f, 0.4f));
        decrement_shape.lineTo(Point<float>(0.2f, 0.05f));
        decrement_shape.lineTo(Point<float>(0.8f, 0.05f));
        decrement_shape.closeSubPath();

        // Add some dummy segments to ensure proper rendering.
        decrement_shape.startNewSubPath(Point<float>(0.0f, 0.0f));
        decrement_shape.closeSubPath();
        decrement_shape.startNewSubPath(Point<float>(1.0f, 0.5f));
        decrement_shape.closeSubPath();

        decrement_shape.addLineSegment(Line<float>(0.0f, 0.0f, 0.0f, 0.0f), 0.2f);
        decrement_shape.addLineSegment(Line<float>(0.5f, 0.5f, 0.5f, 0.5f), 0.2f);
        decrement_->setShape(decrement_shape, true, true, false);
    }

    /**
     * @brief Sets whether the incrementer buttons are active.
     * @param active True if active (enabled), false otherwise.
     *
     * When inactive, the buttons can be visually indicated as disabled.
     */
    void setActive(bool active) {
        active_ = active;
        repaint();
    }

    /**
     * @brief Resizes and positions the increment and decrement buttons.
     */
    void resized() override {
        Rectangle<int> increment_bounds = getLocalBounds();
        Rectangle<int> decrement_bounds = increment_bounds.removeFromBottom(getHeight() / 2);
        increment_->setBounds(increment_bounds);
        decrement_->setBounds(decrement_bounds);
    }

    /**
     * @brief Paints the component background and updates button colors.
     * @param g The graphics context to use for painting.
     */
    void paint(Graphics& g) override {
        setColors();
    }

    /**
     * @brief Called when one of the incrementer buttons is clicked.
     * @param clicked_button The button that was clicked.
     *
     * Increments or decrements the associated Slider's value by 1.0 depending
     * on which button is clicked.
     */
    void buttonClicked(Button* clicked_button) override {
        double value = slider_->getValue();
        if (clicked_button == increment_.get())
            slider_->setValue(value + 1.0);
        else if (clicked_button == decrement_.get())
            slider_->setValue(value - 1.0);
    }

private:
    /**
     * @brief Sets the button colors based on the current skin settings.
     */
    void setColors() {
        Colour normal = findColour(Skin::kIconButtonOff, true);
        Colour hover = findColour(Skin::kIconButtonOffHover, true);
        Colour down = findColour(Skin::kIconButtonOffPressed, true);
        increment_->setColours(normal, hover, down);
        decrement_->setColours(normal, hover, down);
    }

    Slider* slider_;  ///< The Slider that these buttons will increment/decrement.
    bool active_;      ///< Indicates if the incrementer buttons are currently active.

    std::unique_ptr<ShapeButton> increment_; ///< The increment (up) arrow button.
    std::unique_ptr<ShapeButton> decrement_; ///< The decrement (down) arrow button.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IncrementerButtons)
};
