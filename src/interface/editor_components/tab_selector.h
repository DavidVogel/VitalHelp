/// @file tab_selector.h
/// @brief Declares the TabSelector class, a slider-based UI component for selecting tabs.

#pragma once

#include "JuceHeader.h"
#include "open_gl_image_component.h"

/// @class TabSelector
/// @brief A slider-based UI component that displays selectable tabs.
///
/// The TabSelector uses the slider's value to determine which tab is selected.
/// It displays each tab name horizontally and highlights the currently selected tab.
/// The component can be integrated with custom look-and-feels and supports OpenGL
/// image rendering for efficient drawing.
class TabSelector : public Slider {
public:
    /// @brief Default percentage of the font height relative to the component height.
    static constexpr float kDefaultFontHeightPercent = 0.26f;

    /// Constructor.
    /// @param name The name of the TabSelector component.
    TabSelector(String name);

    /// Paints the TabSelector, drawing tab names and highlighting the selected tab.
    /// @param g The JUCE Graphics context.
    void paint(Graphics& g) override;

    /// Handles mouse events that change the selected tab.
    /// @param e The mouse event object.
    void mouseEvent(const MouseEvent& e);

    /// Called when the mouse button is pressed down on the TabSelector.
    /// @param e The mouse event.
    void mouseDown(const MouseEvent& e) override;

    /// Called when the mouse is dragged while the button is held down on the TabSelector.
    /// @param e The mouse event.
    void mouseDrag(const MouseEvent& e) override;

    /// Called when the mouse button is released after being pressed on the TabSelector.
    /// @param e The mouse event.
    void mouseUp(const MouseEvent& e) override;

    /// Sets the names of the tabs.
    /// @param names A vector of strings representing each tab name.
    void setNames(std::vector<std::string> names) { names_ = names; }

    /// Sets the font height as a percentage of the component height.
    /// @param percent The font height percentage.
    void setFontHeightPercent(float percent) { font_height_percent_ = percent; }

    /// Gets the current font height percentage.
    /// @return The font height percentage.
    float getFontHeightPercent() { return font_height_percent_; }

    /// Sets whether the TabSelector is active.
    /// @param active True if active, false otherwise.
    void setActive(bool active) { active_ = active; }

    /// Called when the slider value changes, triggers a redraw of the image.
    void valueChanged() override {
        Slider::valueChanged();
        redoImage();
    }

    /// Retrieves the underlying OpenGlImageComponent used for rendering.
    /// @return A pointer to the OpenGlImageComponent.
    OpenGlImageComponent* getImageComponent() { return &image_component_; }

    /// Redraws the image component.
    void redoImage() { image_component_.redrawImage(true); }

private:
    /// Computes the x-position for a given tab index.
    /// @param position The tab index.
    /// @return The x-position in pixels.
    int getTabX(int position);

    OpenGlImageComponent image_component_; ///< The OpenGL image component for efficient drawing.
    float font_height_percent_;            ///< The font height percentage.
    bool active_;                          ///< Whether the TabSelector is active.
    std::vector<std::string> names_;       ///< The list of tab names.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabSelector)
};
