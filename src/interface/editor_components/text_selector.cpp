/// @file text_selector.cpp
/// @brief Implements the TextSelector and PaintPatternSelector classes.

#include "text_selector.h"

#include "default_look_and_feel.h"
#include "lfo_section.h"
#include "fonts.h"
#include "skin.h"

/// Constructs a TextSelector with the given name.
TextSelector::TextSelector(String name) : SynthSlider(name), long_lookup_(nullptr) { }

/// Handles mouse-down events. Shows a popup menu with text options if it's a normal click,
/// and defers to SynthSlider's mouseDown if it's a context menu click.
/// @param e The mouse event.
void TextSelector::mouseDown(const juce::MouseEvent &e) {
    if (e.mods.isPopupMenu()) {
        SynthSlider::mouseDown(e);
        return;
    }

    const std::string* lookup = long_lookup_ ? long_lookup_ : string_lookup_;

    PopupItems options;
    for (int i = 0; i <= getMaximum(); ++i)
        options.addItem(i, lookup[i]);

    parent_->showPopupSelector(this, Point<int>(0, getHeight()), options, [=](int value) { setValue(value); });
}

/// Handles mouse-up events. If it's a context-menu click, calls SynthSlider's mouseUp.
/// @param e The mouse event.
void TextSelector::mouseUp(const MouseEvent& e) {
    if (e.mods.isPopupMenu()) {
        SynthSlider::mouseUp(e);
        return;
    }
}

/// Paints a pattern based on the current slider value. The pattern is determined by LfoSection::getPaintPattern().
/// Fills and strokes the resulting path using colors depending on the slider's active state.
/// @param g The graphics context.
void PaintPatternSelector::paint(Graphics& g) {
    static constexpr float kLineWidthHeightRatio = 0.05f;

    std::vector<std::pair<float, float>> pattern = LfoSection::getPaintPattern(getValue());
    int height = getHeight() - 2 * padding_ - 1;
    int width = getWidth() - 2 * padding_ - 1;
    float buffer = padding_ + 0.5f;
    Path path;
    path.startNewSubPath(buffer, buffer + height);
    for (auto& pair : pattern)
        path.lineTo(buffer + pair.first * width, buffer + (1.0f - pair.second) * height);

    path.lineTo(buffer + width, buffer + height);

    if (isActive()) {
        g.setColour(findColour(Skin::kWidgetSecondary1, true));
        g.fillPath(path);
        g.setColour(findColour(Skin::kWidgetSecondary2, true));
        g.fillPath(path);
    }
    else {
        g.setColour(findColour(Skin::kLightenScreen, true));
        g.fillPath(path);
    }

    g.setColour(isActive() ? findColour(Skin::kWidgetCenterLine, true) : findColour(Skin::kLightenScreen, true));

    int line_width = getHeight() * kLineWidthHeightRatio;
    line_width += (line_width + 1) % 2; // Ensures an even number
    g.strokePath(path, PathStrokeType(line_width, PathStrokeType::curved, PathStrokeType::rounded));
}
