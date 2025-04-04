/// @file bend_section.h
/// @brief Declares classes for pitch and modulation wheels, and the BendSection container.

#pragma once

#include "JuceHeader.h"
#include "synth_section.h"
#include "synth_slider.h"

class SynthGuiInterface;

/// @class ControlWheel
/// @brief A specialized SynthSlider representing a wheel control (pitch or modulation).
///
/// The ControlWheel class draws a rotary-like wheel using a vertical linear slider approach.
/// It visually simulates a rotating wheel using drawn line segments based on the slider value.
class ControlWheel : public SynthSlider {
public:
    /// Ratio of width used as a buffer space inside the wheel.
    static constexpr float kBufferWidthRatio = 0.05f;
    /// Ratio for the rounded amount of the wheel arc.
    static constexpr float kWheelRoundAmountRatio = 0.25f;
    /// Ratio for the rounded corner of the container.
    static constexpr float kContainerRoundAmountRatio = 0.15f;

    /// Constructor.
    /// @param name The name of this wheel control.
    ControlWheel(String name);

    /// Paints a line segment to visually represent the wheel's position.
    /// @param g The graphics context.
    /// @param y_percent The vertical position ratio for the line.
    /// @param line_color The color of the line.
    /// @param fill_color The color used for filling/shading the line area.
    void paintLine(Graphics& g, float y_percent, Colour line_color, Colour fill_color);

    /// Paints the wheel control.
    /// @param g The graphics context.
    void paint(Graphics& g) override;

    /// Called when the component's parent hierarchy changes.
    void parentHierarchyChanged() override;

protected:
    SynthGuiInterface* parent_; ///< Reference to the parent SynthGuiInterface.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlWheel)
};

/// @class ModWheel
/// @brief A modulation wheel control, typically for controlling modulation depth.
class ModWheel : public ControlWheel {
public:
    /// Constructor.
    ModWheel();
    /// Destructor.
    virtual ~ModWheel() = default;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModWheel)
};

/// @class PitchWheel
/// @brief A pitch wheel control, typically for controlling pitch bending.
///
/// The PitchWheel automatically resets to 0 when the mouse is released.
class PitchWheel : public ControlWheel {
public:
    /// Constructor.
    PitchWheel();
    /// Destructor.
    virtual ~PitchWheel() = default;

    /// Called when the mouse is released. Resets the value to zero.
    /// @param e The mouse event.
    void mouseUp(const MouseEvent& e) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchWheel)
};

/// @class BendSection
/// @brief A UI section containing both pitch and modulation wheels.
class BendSection : public SynthSection {
public:
    /// Constructor.
    /// @param name The name of this section.
    BendSection(const String& name);

    /// Destructor.
    ~BendSection();

    /// Paints the background of the bend section.
    /// @param g The graphics context.
    void paintBackground(Graphics& g) override;

    /// Paints a background shadow for this section.
    /// @param g The graphics context.
    void paintBackgroundShadow(Graphics& g) override { paintTabShadow(g); }

    /// Resizes and lays out the wheels.
    void resized() override;

    /// Called when a slider (wheel) value changes.
    /// @param changed_slider The slider that changed.
    void sliderValueChanged(Slider* changed_slider) override;

private:
    std::unique_ptr<PitchWheel> pitch_wheel_; ///< The pitch wheel control.
    std::unique_ptr<ModWheel> mod_wheel_;     ///< The modulation wheel control.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BendSection)
};
