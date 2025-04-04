/// @file about_section.h
/// @brief Declares the AboutSection class, which displays information about the application and allows configuration of settings.

#pragma once

#include "JuceHeader.h"
#include "overlay.h"
#include "open_gl_multi_quad.h"
#include "open_gl_image_component.h"

class AppLogo;

/// @class OpenGlDeviceSelector
/// @brief A device selector component rendered using OpenGL for improved performance.
///
/// This class provides a device selector (for audio and MIDI devices) wrapped in an OpenGL component.
/// It updates its rendered image whenever resized, ensuring a smooth UI experience.
class OpenGlDeviceSelector : public OpenGlAutoImageComponent<AudioDeviceSelectorComponent> {
public:
    /// Constructor.
    /// @param device_manager The AudioDeviceManager controlling audio and MIDI devices.
    /// @param min_audio_input_channels Minimum number of audio input channels.
    /// @param max_audioInput_channels Maximum number of audio input channels.
    /// @param min_audio_output_channels Minimum number of audio output channels.
    /// @param max_audioOutput_channels Maximum number of audio output channels.
    /// @param show_midi_input_options Whether to show MIDI input options.
    /// @param show_midi_output_selector Whether to show a MIDI output selector.
    /// @param show_channels_as_stereo_pairs Whether to show channel pairs as stereo.
    /// @param hide_advanced_options_with_button Whether to hide advanced options behind a button.
    OpenGlDeviceSelector(AudioDeviceManager& device_manager,
                         int min_audio_input_channels, int max_audioInput_channels,
                         int min_audio_output_channels, int max_audioOutput_channels,
                         bool show_midi_input_options, bool show_midi_output_selector,
                         bool show_channels_as_stereo_pairs, bool hide_advanced_options_with_button)
            : OpenGlAutoImageComponent<AudioDeviceSelectorComponent>(device_manager,
                                                                     min_audio_input_channels, max_audioInput_channels,
                                                                     min_audio_output_channels, max_audioOutput_channels,
                                                                     show_midi_input_options, show_midi_output_selector,
                                                                     show_channels_as_stereo_pairs,
                                                                     hide_advanced_options_with_button) {
        setLookAndFeel(DefaultLookAndFeel::instance());
        image_component_.setComponent(this);
    }

    /// Called when the component is resized; triggers a redraw of the image.
    void resized() override {
        OpenGlAutoImageComponent<AudioDeviceSelectorComponent>::resized();
        if (isShowing())
            redoImage();
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGlDeviceSelector)
};

/// @class AboutSection
/// @brief An overlay section that displays application information, device settings, and UI scaling options.
///
/// The AboutSection shows the application's name, version, logo, and offers controls to:
/// - Configure device settings (via an OpenGlDeviceSelector).
/// - Toggle update checks.
/// - Adjust GUI size scaling.
/// Clicking outside the info rectangle hides this section.
class AboutSection : public Overlay {
public:
    /// The fixed dimensions and layout constants for this section.
    static constexpr int kInfoWidth = 430;
    static constexpr int kBasicInfoHeight = 250;
    static constexpr int kPaddingX = 25;
    static constexpr int kPaddingY = 15;
    static constexpr int kButtonHeight = 30;
    static constexpr int kLeftLogoBuffer = 95;
    static constexpr int kNameRightBuffer = 85;
    static constexpr int kLogoWidth = 96;

    /// Scaling multipliers for adjusting GUI size.
    static constexpr float kMultExtraSmall = 0.5f;
    static constexpr float kMultSmall = 0.7f;
    static constexpr float kMultLarge = 1.35f;
    static constexpr float kMultDouble = 2.0f;
    static constexpr float kMultTriple = 3.0f;
    static constexpr float kMultQuadruple = 4.0f;

    /// Constructor.
    /// @param name The name of this overlay component.
    AboutSection(const String& name);

    /// Destructor.
    virtual ~AboutSection();

    /// Sets the bounds of the logo within the info rectangle.
    void setLogoBounds();

    /// Handles resizing of the section, laying out child components and controls.
    void resized() override;

    /// Renders OpenGL components by forwarding to the base class rendering method.
    void renderOpenGlComponents(OpenGlWrapper& open_gl, bool animate) override {
        SynthSection::renderOpenGlComponents(open_gl, animate);
    }

    /// Gets the rectangle that contains the main info components.
    /// @return The rectangle for the info section.
    Rectangle<int> getInfoRect();

    /// If the mouse is released outside the info rectangle, hides the AboutSection.
    /// @param e The mouse event.
    void mouseUp(const MouseEvent& e) override;

    /// Sets visibility. When shown, updates the logo layout and redraws backgrounds.
    /// @param should_be_visible True to make visible, false otherwise.
    void setVisible(bool should_be_visible) override;

    /// Handles button clicks from size scaling buttons and the update check toggle.
    /// @param clicked_button The button that was clicked.
    void buttonClicked(Button* clicked_button) override;

private:
    /// Sets the GUI size according to the given multiplier.
    /// @param multiplier The scaling factor for the GUI.
    void setGuiSize(float multiplier);

    /// Toggles full screen mode.
    void fullScreen();

    std::unique_ptr<OpenGlDeviceSelector> device_selector_; ///< Device selector for audio/MIDI settings.
    std::unique_ptr<OpenGlToggleButton> check_for_updates_; ///< Toggle for checking updates.
    std::unique_ptr<PlainTextComponent> check_for_updates_text_;

    std::unique_ptr<OpenGlToggleButton> size_button_extra_small_;
    std::unique_ptr<OpenGlToggleButton> size_button_small_;
    std::unique_ptr<OpenGlToggleButton> size_button_normal_;
    std::unique_ptr<OpenGlToggleButton> size_button_large_;
    std::unique_ptr<OpenGlToggleButton> size_button_double_;
    std::unique_ptr<OpenGlToggleButton> size_button_triple_;
    std::unique_ptr<OpenGlToggleButton> size_button_quadruple_;

    OpenGlQuad body_;
    std::unique_ptr<AppLogo> logo_;
    std::unique_ptr<PlainTextComponent> name_text_;
    std::unique_ptr<PlainTextComponent> version_text_;
    std::unique_ptr<PlainTextComponent> check_updates_text_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AboutSection)
};
