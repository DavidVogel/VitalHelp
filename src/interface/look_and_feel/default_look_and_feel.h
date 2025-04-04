#pragma once

#include "JuceHeader.h"

class SynthSlider;

/**
 * @class LeftAlignedScrollBar
 * @brief A ScrollBar variant that is aligned to the left side when vertical.
 *
 * This class primarily exists to differentiate alignment behavior within draw calls.
 * By default, the scrollbar is considered right-aligned unless cast to this class.
 */
class LeftAlignedScrollBar : public ScrollBar {
public:
    /**
     * @brief Constructs a LeftAlignedScrollBar.
     * @param vertical True if the scrollbar is vertical, false if horizontal.
     */
    LeftAlignedScrollBar(bool vertical) : ScrollBar(vertical) { }
};

/**
 * @class DefaultLookAndFeel
 * @brief A base LookAndFeel class providing default styling for UI elements.
 *
 * This LookAndFeel sets up background colors, borders, popup menus, scrollbars,
 * combo boxes, and tick boxes with a consistent appearance. It also provides
 * default fonts for popup menus and slider popups.
 */
class DefaultLookAndFeel : public juce::LookAndFeel_V4 {
public:
    static constexpr int kPopupMenuBorder = 4; ///< Border size for popup menus.

    /** @brief Destructor. */
    ~DefaultLookAndFeel() { }

    /**
     * @brief Returns the border size for popup menus.
     * @return The border size in pixels.
     */
    virtual int getPopupMenuBorderSize() override { return kPopupMenuBorder; }

    /**
     * @brief Draws no outline for TextEditors by default.
     */
    void drawTextEditorOutline(Graphics& g, int width, int height, TextEditor& text_editor) override { }

    /**
     * @brief Fills the background of a TextEditor with a rounded rectangle and border.
     * @param g The Graphics context.
     * @param width The width of the text editor.
     * @param height The height of the text editor.
     * @param text_editor Reference to the TextEditor.
     */
    void fillTextEditorBackground(Graphics& g, int width, int height, TextEditor& text_editor) override;

    /**
     * @brief Draws the background for a popup menu with a rounded rectangle and border.
     * @param g The Graphics context.
     * @param width The width of the popup menu.
     * @param height The height of the popup menu.
     */
    void drawPopupMenuBackground(Graphics& g, int width, int height) override;

    /**
     * @brief Draws a custom scrollbar, potentially aligned differently if it's a LeftAlignedScrollBar.
     * @param g The Graphics context.
     * @param scroll_bar The ScrollBar to draw.
     * @param x, y, width, height The bounds of the scrollbar.
     * @param vertical True if vertical scrollbar, false if horizontal.
     * @param thumb_position The current thumb position.
     * @param thumb_size The size of the thumb.
     * @param mouse_over True if mouse is over the scrollbar.
     * @param mouse_down True if mouse is clicked down on the scrollbar.
     */
    virtual void drawScrollbar(Graphics& g, ScrollBar& scroll_bar, int x, int y, int width, int height,
                               bool vertical, int thumb_position, int thumb_size,
                               bool mouse_over, bool mouse_down) override;

    /**
     * @brief Draws the background and arrow of a ComboBox.
     * @param g The Graphics context.
     * @param width The width of the ComboBox.
     * @param height The height of the ComboBox.
     * @param button_down True if the mouse is down on the combo box.
     * @param button_x, button_y, button_w, button_h The button bounds within the combo box.
     * @param box The ComboBox to draw.
     */
    void drawComboBox(Graphics& g, int width, int height, const bool button_down,
                      int button_x, int button_y, int button_w, int button_h, ComboBox& box) override;

    /**
     * @brief Draws a tick box (check box) with a filled rectangle if ticked.
     * @param g The Graphics context.
     * @param component Reference to the parent component.
     * @param x, y, w, h The bounds of the tick box.
     * @param ticked True if the box is checked.
     * @param enabled True if the box is enabled.
     * @param mouse_over True if mouse is hovering.
     * @param button_down True if mouse is clicked down.
     */
    void drawTickBox(Graphics& g, Component& component,
                     float x, float y, float w, float h, bool ticked,
                     bool enabled, bool mouse_over, bool button_down) override;

    /**
     * @brief Draws the background of a CallOutBox with a simple rounded rectangle and stroke.
     * @param call_out_box The CallOutBox to draw.
     * @param g The Graphics context.
     * @param path The Path defining the callout shape.
     * @param unused_image Unused parameter.
     */
    void drawCallOutBoxBackground(CallOutBox& call_out_box, Graphics& g, const Path& path, Image&) override;

    /**
     * @brief Draws the background of a generic button, using a rounded rectangle.
     * @param g The Graphics context.
     * @param button The button to draw.
     * @param background_color The base background color.
     * @param hover True if mouse is hovering the button.
     * @param down True if mouse is pressed down on the button.
     */
    void drawButtonBackground(Graphics& g, Button& button, const Colour& background_color,
                              bool hover, bool down) override;

    /**
     * @brief Gets the popup placement for a slider. Delegates to SynthSlider if present.
     * @param slider The slider in question.
     * @return The popup placement flags.
     */
    int getSliderPopupPlacement(Slider& slider) override;

    /**
     * @brief Returns the font to use for popup menus.
     * @return The Font for popup menus.
     */
    Font getPopupMenuFont() override;

    /**
     * @brief Returns the font to use for slider popup text.
     * @param slider Reference to the slider.
     * @return The Font for the slider popup.
     */
    Font getSliderPopupFont(Slider& slider) override;

    /**
     * @brief Returns the window flags for menu windows. Defaults to 0.
     * @return The integer flags.
     */
    int getMenuWindowFlags() override { return 0; }

    /**
     * @brief Singleton instance accessor.
     * @return Pointer to the single DefaultLookAndFeel instance.
     */
    static DefaultLookAndFeel* instance() {
        static DefaultLookAndFeel instance;
        return &instance;
    }

protected:
    /** @brief Protected constructor to enforce singleton usage. */
    DefaultLookAndFeel();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DefaultLookAndFeel)
};
