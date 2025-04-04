/// @file synth_button.h
/// @brief Declares classes for OpenGL-based buttons used in the Vital synth UI.

#pragma once

#include "JuceHeader.h"

#include "open_gl_image_component.h"
#include "open_gl_multi_quad.h"
#include "synth_gui_interface.h"

/// @class OpenGlShapeButtonComponent
/// @brief A specialized OpenGL component for rendering a shape-based button.
///
/// This component displays a shape (path) that changes color and appearance based on
/// hover and pressed states. It is intended to be used in conjunction with a JUCE Button.
class OpenGlShapeButtonComponent : public OpenGlComponent {
  public:
    /// @brief The amount of change in hover transition per frame.
    static constexpr float kHoverInc = 0.2f;

    /// Constructor.
    /// @param button The JUCE Button associated with this shape component.
    OpenGlShapeButtonComponent(Button* button) : button_(button), down_(false), hover_(false), hover_amount_(0.0f),
                                                 use_on_colors_(false), shape_("shape") {
      shape_.setComponent(button);
      shape_.setScissor(true);
    }

    /// Called when the parent hierarchy changes, for example when the component is moved in the UI.
    void parentHierarchyChanged() override {
      if (findParentComponentOfClass<SynthGuiInterface>())
        setColors();
    }

    /// Sets the colors used for this shape button based on the current Skin.
    void setColors() {
      off_normal_color_ = button_->findColour(Skin::kIconButtonOff, true);
      off_hover_color_ = button_->findColour(Skin::kIconButtonOffHover, true);
      off_down_color_ = button_->findColour(Skin::kIconButtonOffPressed, true);
      on_normal_color_ = button_->findColour(Skin::kIconButtonOn, true);
      on_hover_color_ = button_->findColour(Skin::kIconButtonOnHover, true);
      on_down_color_ = button_->findColour(Skin::kIconButtonOnPressed, true);
    }

    /// Increments or decrements the hover amount, smoothing the hover transitions.
    void incrementHover();

    /// Initializes the OpenGL resources for this component.
    /// @param open_gl The OpenGlWrapper managing OpenGL state.
    virtual void init(OpenGlWrapper& open_gl) override {
      OpenGlComponent::init(open_gl);
      shape_.init(open_gl);
    };

    /// Renders the shape component.
    /// @param open_gl The OpenGlWrapper managing OpenGL state.
    /// @param animate If true, animates transitions like hover.
    virtual void render(OpenGlWrapper& open_gl, bool animate) override;

    /// Destroys OpenGL resources associated with this component.
    /// @param open_gl The OpenGlWrapper managing OpenGL state.
    virtual void destroy(OpenGlWrapper& open_gl) override {
      OpenGlComponent::destroy(open_gl);
      shape_.destroy(open_gl);
    };

    /// Redraws the image of the shape, useful after size or color changes.
    void redoImage() { shape_.redrawImage(true); setColors(); }

    /// Sets the shape (path) to be rendered by this component.
    /// @param shape The new path representing the shape.
    void setShape(const Path& shape) { shape_.setShape(shape); }

    /// Toggles whether the "on" colors should be used (for toggled states).
    /// @param use If true, uses the on-colors, otherwise uses off-colors.
    void useOnColors(bool use) { use_on_colors_ = use; }

    /// Sets the pressed state.
    /// @param down True if the button is pressed down.
    void setDown(bool down) { down_ = down; }

    /// Sets the hover state.
    /// @param hover True if the mouse is hovering over the button.
    void setHover(bool hover) { hover_ = hover; }

  private:
    Button* button_;                 ///< Associated JUCE Button.
    bool down_;                      ///< True if the button is currently pressed.
    bool hover_;                     ///< True if the mouse is hovering over the button.
    float hover_amount_;             ///< A smoothed value indicating hover intensity.
    bool use_on_colors_;             ///< True if using on-colors, false otherwise.
    PlainShapeComponent shape_;      ///< The shape component to render.

    // Colors
    Colour off_normal_color_;
    Colour off_hover_color_;
    Colour off_down_color_;
    Colour on_normal_color_;
    Colour on_hover_color_;
    Colour on_down_color_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGlShapeButtonComponent)
};

/// @class OpenGlShapeButton
/// @brief A ToggleButton that uses an OpenGlShapeButtonComponent for its rendering.
///
/// This class acts as a JUCE ToggleButton but overrides its look with a custom OpenGL shape.
class OpenGlShapeButton : public ToggleButton {
  public:
    /// Constructor.
    /// @param name The name of the button.
    OpenGlShapeButton(String name) : gl_component_(this) {
      setName(name);
    }

    /// Retrieves the underlying OpenGL component.
    /// @return A pointer to the OpenGlComponent used for rendering.
    OpenGlComponent* getGlComponent() { return &gl_component_; }

    /// Sets the shape to be rendered by the button.
    /// @param shape The new path representing the shape.
    void setShape(const Path& shape) { gl_component_.setShape(shape); }

    /// Toggles whether to use on-colors.
    /// @param use If true, use on-colors; otherwise off-colors.
    void useOnColors(bool use) { gl_component_.useOnColors(use); }

    /// Called when the button is resized. Updates the internal image.
    void resized() override {
      ToggleButton::resized();
      gl_component_.redoImage();
    }

    /// Called when the mouse enters the button area.
    /// @param e The mouse event.
    void mouseEnter(const MouseEvent& e) override {
      ToggleButton::mouseEnter(e);
      gl_component_.setHover(true);
    }

    /// Called when the mouse leaves the button area.
    /// @param e The mouse event.
    void mouseExit(const MouseEvent& e) override {
      ToggleButton::mouseExit(e);
      gl_component_.setHover(false);
    }

    /// Called when the mouse is pressed down on the button.
    /// @param e The mouse event.
    void mouseDown(const MouseEvent& e) override {
      ToggleButton::mouseDown(e);
      gl_component_.setDown(true);
    }

    /// Called when the mouse is released from the button.
    /// @param e The mouse event.
    void mouseUp(const MouseEvent& e) override {
      ToggleButton::mouseUp(e);
      gl_component_.setDown(false);
    }

  private:
    OpenGlShapeButtonComponent gl_component_; ///< The OpenGL shape rendering component.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGlShapeButton)
};

/// @class OpenGlButtonComponent
/// @brief A specialized OpenGL component for rendering various styles of buttons.
///
/// This component supports multiple styles (text button, just text, power button, UI button, lighten button)
/// and changes its appearance based on hover, press, toggle state, and other parameters.
class OpenGlButtonComponent : public OpenGlComponent {
  public:
    /// The amount of change in hover transition per frame.
    static constexpr float kHoverInc = 0.2f;

    /// Enumeration of different button styles.
    enum ButtonStyle {
      kTextButton,
      kJustText,
      kPowerButton,
      kUiButton,
      kLightenButton,
      kNumButtonStyles
    };

    /// Constructor.
    /// @param button The JUCE Button associated with this OpenGlButtonComponent.
    OpenGlButtonComponent(Button* button) : style_(kTextButton), button_(button),
                                            show_on_colors_(true), primary_ui_button_(false),
                                            down_(false), hover_(false), hover_amount_(0.0f),
                                            background_(Shaders::kRoundedRectangleFragment), text_("text", "") {
      background_.setTargetComponent(button);
      background_.setColor(Colours::orange);
      background_.setQuad(0, -1.0f, -1.0f, 2.0f, 2.0f);

      addChildComponent(text_);
      text_.setActive(false);
      text_.setScissor(true);
      text_.setComponent(button);
      text_.setFontType(PlainTextComponent::kMono);
    }

    /// Initializes the OpenGL resources for this component.
    /// @param open_gl The OpenGlWrapper for managing OpenGL state.
    virtual void init(OpenGlWrapper& open_gl) override {
      if (style_ == kPowerButton)
        background_.setFragmentShader(Shaders::kCircleFragment);

      background_.init(open_gl);
      text_.init(open_gl);

      setColors();
    }

    /// Sets the colors based on the current style and Skin.
    void setColors();

    /// Renders the button as a text button.
    /// @param open_gl The OpenGlWrapper for managing OpenGL state.
    /// @param animate If true, animates transitions.
    void renderTextButton(OpenGlWrapper& open_gl, bool animate);

    /// Renders the button as a power button.
    /// @param open_gl The OpenGlWrapper for managing OpenGL state.
    /// @param animate If true, animates transitions.
    void renderPowerButton(OpenGlWrapper& open_gl, bool animate);

    /// Renders the button as a UI button.
    /// @param open_gl The OpenGlWrapper for managing OpenGL state.
    /// @param animate If true, animates transitions.
    void renderUiButton(OpenGlWrapper& open_gl, bool animate);

    /// Renders the button as a lighten button.
    /// @param open_gl The OpenGlWrapper for managing OpenGL state.
    /// @param animate If true, animates transitions.
    void renderLightenButton(OpenGlWrapper& open_gl, bool animate);

    /// Increments or decrements the hover amount, smoothing the hover transitions.
    void incrementHover();

    /// Renders the button based on its current style.
    /// @param open_gl The OpenGlWrapper for managing OpenGL state.
    /// @param animate If true, animates transitions.
    virtual void render(OpenGlWrapper& open_gl, bool animate) override {
      if (style_ == kTextButton || style_ == kJustText)
        renderTextButton(open_gl, animate);
      else if (style_ == kPowerButton)
        renderPowerButton(open_gl, animate);
      else if (style_ == kUiButton)
        renderUiButton(open_gl, animate);
      else if (style_ == kLightenButton)
        renderLightenButton(open_gl, animate);
    }

    /// Sets the text displayed on the button.
    void setText() {
      String text = button_->getButtonText();
      if (!text.isEmpty()) {
        text_.setActive(true);
        text_.setText(text);
      }
    }

    /// Sets the pressed state.
    /// @param down True if the button is currently pressed.
    void setDown(bool down) { down_ = down; }

    /// Sets the hover state.
    /// @param hover True if the mouse is hovering over the button.
    void setHover(bool hover) { hover_ = hover; }

    /// Destroys the OpenGL resources associated with this component.
    /// @param open_gl The OpenGlWrapper managing OpenGL state.
    virtual void destroy(OpenGlWrapper& open_gl) override {
      background_.destroy(open_gl);
      text_.destroy(open_gl);
    }

    /// Sets the text justification mode.
    /// @param justification The JUCE Justification mode.
    void setJustification(Justification justification) { text_.setJustification(justification); }

    /// Sets the button style.
    /// @param style The style to use.
    void setStyle(ButtonStyle style) { style_ = style; }

    /// Toggles whether to show on-colors when toggled on.
    /// @param show True to show on-colors, false otherwise.
    void setShowOnColors(bool show) { show_on_colors_ = show; }

    /// Sets whether this is a primary UI button.
    /// @param primary True if primary, otherwise false.
    void setPrimaryUiButton(bool primary) { primary_ui_button_ = primary; }

    /// Overrides the default background painting, does nothing as we paint with OpenGL.
    void paintBackground(Graphics& g) override { }

    /// Gets the background quad component.
    /// @return A reference to the OpenGlQuad used for the button background.
    OpenGlQuad& background() { return background_; }

    /// Gets the text component.
    /// @return A reference to the PlainTextComponent used for the button text.
    PlainTextComponent& text() { return text_; }

    /// Gets the current button style.
    /// @return The current ButtonStyle.
    ButtonStyle style() { return style_; }

  protected:
    ButtonStyle style_;         ///< Current button style.
    Button* button_;            ///< Associated JUCE Button.
    bool show_on_colors_;       ///< True if showing on-colors when toggled on.
    bool primary_ui_button_;    ///< True if this is a primary UI button.
    bool down_;                 ///< True if the button is pressed.
    bool hover_;                ///< True if the mouse is hovering over the button.
    float hover_amount_;        ///< A smoothed value for hover transitions.
    OpenGlQuad background_;     ///< Background quad for rendering button body.
    PlainTextComponent text_;   ///< Text component for rendering button label.

    // Colors
    Colour on_color_;
    Colour on_pressed_color_;
    Colour on_hover_color_;
    Colour off_color_;
    Colour off_pressed_color_;
    Colour off_hover_color_;
    Colour background_color_;
    Colour body_color_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGlButtonComponent)
};

/// @class OpenGlToggleButton
/// @brief A ToggleButton that uses an OpenGlButtonComponent for its rendering.
///
/// It supports various styles and uses OpenGL for drawing, making it suitable for modern UIs.
class OpenGlToggleButton : public ToggleButton {
  public:
    /// Constructor.
    /// @param name The name of the toggle button.
    OpenGlToggleButton(String name) : ToggleButton(name), active_(true), button_component_(this) { }

    /// Retrieves the underlying OpenGL component.
    /// @return A pointer to the OpenGlButtonComponent.
    OpenGlButtonComponent* getGlComponent() { return &button_component_; }

    /// Sets the active state of the button.
    /// @param active True if active, false otherwise.
    void setActive(bool active = true) { active_ = active; }

    /// Checks if the button is active.
    /// @return True if active, otherwise false.
    bool isActive() const { return active_; }

    /// Called when the button is resized, adjusts text size and colors accordingly.
    void resized() override;

    /// Sets the text to be displayed on the button.
    /// @param text The text to set.
    void setText(String text) {
      setButtonText(text);
      button_component_.setText();
    }

    /// Configures the button as a power button.
    void setPowerButton() {
      button_component_.setStyle(OpenGlButtonComponent::kPowerButton);
    }

    /// Removes the background, showing just text.
    void setNoBackground() {
      button_component_.setStyle(OpenGlButtonComponent::kJustText);
    }

    /// Sets the text justification mode.
    /// @param justification The JUCE Justification mode.
    void setJustification(Justification justification) {
      button_component_.setJustification(justification);
    }

    /// Configures the button as a lighten button.
    void setLightenButton() {
      button_component_.setStyle(OpenGlButtonComponent::kLightenButton);
    }

    /// Toggles showing on-colors when toggled on.
    /// @param show True to show on-colors, otherwise off-colors.
    void setShowOnColors(bool show) {
      button_component_.setShowOnColors(show);
    }

    /// Configures the button as a UI button.
    /// @param primary True if this is a primary UI button.
    void setUiButton(bool primary) {
      button_component_.setStyle(OpenGlButtonComponent::kUiButton);
      button_component_.setPrimaryUiButton(primary);
    }

    /// Called when the button enablement changes.
    virtual void enablementChanged() override {
      ToggleButton::enablementChanged();
      button_component_.setColors();
    }

    /// Called when the mouse enters the button area.
    /// @param e The mouse event.
    void mouseEnter(const MouseEvent& e) override {
      ToggleButton::mouseEnter(e);
      button_component_.setHover(true);
    }

    /// Called when the mouse leaves the button area.
    /// @param e The mouse event.
    void mouseExit(const MouseEvent& e) override {
      ToggleButton::mouseExit(e);
      button_component_.setHover(false);
    }

    /// Called when the mouse is pressed down on the button.
    /// @param e The mouse event.
    void mouseDown(const MouseEvent& e) override {
      ToggleButton::mouseDown(e);
      button_component_.setDown(true);
    }

    /// Called when the mouse is released from the button.
    /// @param e The mouse event.
    void mouseUp(const MouseEvent& e) override {
      ToggleButton::mouseUp(e);
      button_component_.setDown(false);
    }

  private:
    bool active_;                      ///< True if the button is active.
    OpenGlButtonComponent button_component_; ///< The OpenGL component for rendering the button.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGlToggleButton)
};

/// @class SynthButton
/// @brief A specialized OpenGlToggleButton with additional functionality for the Vital synth.
///
/// This button supports MIDI learn operations and can display different text or behavior depending on
/// whether it is toggled. It also notifies registered ButtonListeners of changes.
class SynthButton : public OpenGlToggleButton {
  public:
    /// Possible menu IDs for popup operations.
    enum MenuId {
      kCancel = 0,
      kArmMidiLearn,
      kClearMidiLearn
    };

    /// @class ButtonListener
    /// @brief Interface for objects interested in changes to SynthButton state.
    class ButtonListener {
      public:
        /// Virtual destructor.
        virtual ~ButtonListener() { }

        /// Called when the button state changes in the GUI.
        /// @param button The SynthButton that changed.
        virtual void guiChanged(SynthButton* button) { }
    };

    /// Constructor.
    /// @param name The name of the button, often tied to a parameter.
    SynthButton(String name);

    /// Sets a string lookup array for on/off text.
    /// @param lookup A pointer to an array of two std::string items: [offText, onText].
    void setStringLookup(const std::string* lookup) {
      string_lookup_ = lookup;
    }

    /// Gets the string lookup array.
    /// @return The string lookup array set by setStringLookup().
    const std::string* getStringLookup() const { return string_lookup_; }

    /// Retrieves the text corresponding to the on/off state.
    /// @param value True for on, false for off.
    /// @return The corresponding text string.
    String getTextFromValue(bool value);

    /// Handles the result of the popup menu selection.
    /// @param result The selected menu ID.
    void handlePopupResult(int result);

    /// Called when the mouse is pressed down on the button.
    /// @param e The mouse event.
    virtual void mouseDown(const MouseEvent& e) override;


    /// Called when the mouse is released from the button.
    /// @param e The mouse event.
    virtual void mouseUp(const MouseEvent& e) override;


    /// Adds a button listener to be notified of changes.
    /// @param listener The ButtonListener to add.
    void addButtonListener(ButtonListener* listener);

    /// Called when the button is clicked.
    virtual void clicked() override {
      OpenGlToggleButton::clicked();
      if (string_lookup_)
        setText(string_lookup_[getToggleState() ? 1 : 0]);
    }

  private:
    /// Internal clicked handler that also checks for modifier keys.
    /// @param modifiers The current ModifierKeys state.
    void clicked(const ModifierKeys& modifiers) override;

    const std::string* string_lookup_;    ///< Optional array for on/off text.
    std::vector<ButtonListener*> button_listeners_; ///< Registered listeners.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthButton)
};
