/// @file synth_slider.h
/// @brief Declares the SynthSlider and related classes, providing various slider styles and functionality in the UI.

#pragma once

#include "JuceHeader.h"

#include "curve_look_and_feel.h"
#include "text_look_and_feel.h"
#include "open_gl_image_component.h"
#include "open_gl_multi_quad.h"
#include "synth_types.h"

class FullInterface;
class OpenGlSlider;
class SynthGuiInterface;
class SynthSection;

/// @class OpenGlSliderQuad
/// @brief A specialized OpenGlQuad for rendering a slider using OpenGL.
///
/// This component uses different shader fragments depending on whether the slider is
/// rotary, horizontal, vertical, or a modulation knob. It ties directly to an OpenGlSlider
/// for retrieving state and parameters.
class OpenGlSliderQuad : public OpenGlQuad {
  public:
    /// Constructor.
    /// @param slider The OpenGlSlider associated with this quad.
    OpenGlSliderQuad(OpenGlSlider* slider) : OpenGlQuad(Shaders::kRotarySliderFragment), slider_(slider) { }

    /// Initializes the OpenGL resources for this quad, selecting the appropriate shader.
    /// @param open_gl The OpenGlWrapper for managing OpenGL state.
    virtual void init(OpenGlWrapper& open_gl) override;

    /// Paints the background by re-triggering the slider image generation if needed.
    /// @param g The JUCE Graphics context.
    void paintBackground(Graphics& g) override;

  private:
    OpenGlSlider* slider_; ///< The associated OpenGlSlider.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGlSliderQuad)
};

/// @class OpenGlSlider
/// @brief An extended JUCE Slider that leverages OpenGL for rendering.
///
/// This slider can be rendered as a rotary knob, a horizontal bar, a vertical bar, or
/// a modulation knob. It integrates with the Vital skin system to determine colors,
/// dimensions, and other style parameters. It can also display an OpenGL-based image
/// component or quad for custom appearances.
class OpenGlSlider : public Slider {
  public:
    /// The default rotary arc angle used for rotary sliders.
    static constexpr float kRotaryAngle = 0.8f * vital::kPi;

    /// Constructor.
    /// @param name The name of the slider.
    OpenGlSlider(String name) : Slider(name), parent_(nullptr), modulation_knob_(false), modulation_amount_(0.0f),
                                paint_to_image_(false), active_(true), bipolar_(false), slider_quad_(this) {
      slider_quad_.setTargetComponent(this);
      setMaxArc(kRotaryAngle);

      image_component_.paintEntireComponent(false);
      image_component_.setComponent(this);
      image_component_.setScissor(true);

      slider_quad_.setActive(false);
      image_component_.setActive(false);
    }

    /// Called when the component is resized. Updates colors and display values.
    virtual void resized() override {
      Slider::resized();
      setColors();
      setSliderDisplayValues();
    }

    /// Called when the slider value changes. Redraws the image to reflect the new value.
    virtual void valueChanged() override {
      Slider::valueChanged();
      redoImage();
    }

    /// Called when the parent hierarchy changes. Used for retrieving parent sections.
    void parentHierarchyChanged() override {
      parent_ = findParentComponentOfClass<SynthSection>();
      Slider::parentHierarchyChanged();
    }

    /// Toggles whether the slider should paint into an image before rendering.
    /// @param paint If true, paints to an offscreen image; otherwise paints directly.
    void paintToImage(bool paint) {
      paint_to_image_ = paint;
    }

    /// Checks if the slider uses text-based rendering.
    /// @return True if a text look-and-feel is used.
    bool isText() const {
      return &getLookAndFeel() == TextLookAndFeel::instance();
    }

    /// Checks if the slider uses either text or curve look-and-feel.
    /// @return True if using text or curve LAF.
    bool isTextOrCurve() const {
      return isText() || &getLookAndFeel() == CurveLookAndFeel::instance();
    }

    /// Checks if the slider is a modulation knob type.
    /// @return True if it is a modulation knob.
    bool isModulationKnob() const {
      return modulation_knob_;
    }

    /// Checks if the slider should use a rotary quad OpenGL rendering.
    /// @return True if it's a rotary slider with OpenGL quad rendering enabled.
    bool isRotaryQuad() const {
      return !paint_to_image_ && getSliderStyle() == RotaryHorizontalVerticalDrag && !isTextOrCurve();
    }

    /// Checks if the slider should use a horizontal quad OpenGL rendering.
    /// @return True if it's a horizontal slider with OpenGL quad rendering.
    bool isHorizontalQuad() const {
      return !paint_to_image_ && getSliderStyle() == LinearBar && !isTextOrCurve();
    }

    /// Checks if the slider should use a vertical quad OpenGL rendering.
    /// @return True if it's a vertical slider with OpenGL quad rendering.
    bool isVerticalQuad() const {
      return !paint_to_image_ && getSliderStyle() == LinearBarVertical && !isTextOrCurve();
    }

    /// Gets the image component (if used).
    /// @return The OpenGlComponent for the image.
    OpenGlComponent* getImageComponent() {
      return &image_component_;
    }

    /// Gets the quad component used for rendering the slider (if used).
    /// @return The OpenGlComponent for the quad.
    OpenGlComponent* getQuadComponent() {
      return &slider_quad_;
    }

    /// Sets the maximum arc for a rotary slider.
    /// @param arc The maximum arc in radians.
    void setMaxArc(float arc) {
      slider_quad_.setMaxArc(arc);
    }

    /// Marks this slider as a modulation knob.
    void setModulationKnob() { modulation_knob_ = true; }

    /// Sets the amount of modulation applied to the slider.
    /// @param modulation The modulation amount.
    void setModulationAmount(float modulation) { modulation_amount_ = modulation; redoImage(); }

    /// Gets the current modulation amount.
    /// @return The modulation amount.
    float getModulationAmount() const { return modulation_amount_; }

    /// Provides a scaling factor for the knob size.
    /// @return The knob size scale factor.
    virtual float getKnobSizeScale() const { return 1.0f; }

    /// Checks if the slider is bipolar (centered at zero).
    /// @return True if bipolar, false otherwise.
    bool isBipolar() const { return bipolar_; }

    /// Checks if the slider is active.
    /// @return True if active, false otherwise.
    bool isActive() const { return active_; }

    /// Sets the slider to be bipolar.
    /// @param bipolar True if the slider should be bipolar.
    void setBipolar(bool bipolar = true) {
      if (bipolar_ == bipolar)
        return;

      bipolar_ = bipolar;
      redoImage();
    }

    /// Sets the slider as active or inactive.
    /// @param active True if active, false otherwise.
    void setActive(bool active = true) {
      if (active_ == active)
        return;

      active_ = active;
      setColors();
      redoImage();
    }

    /// Gets the color used for modulation displays.
    /// @return The modulation color.
    virtual Colour getModColor() const {
      return findColour(Skin::kModulationMeterControl, true);
    }

    /// Gets the background color.
    /// @return The background color.
    virtual Colour getBackgroundColor() const {
      return findColour(Skin::kWidgetBackground, true);
    }

    /// Gets the color for the unselected portion of the slider.
    /// @return The unselected color.
    virtual Colour getUnselectedColor() const {
      if (isModulationKnob())
        return findColour(Skin::kWidgetBackground, true);
      if (isRotary()) {
        if (active_)
          return findColour(Skin::kRotaryArcUnselected, true);
        return findColour(Skin::kRotaryArcUnselectedDisabled, true);
      }

      return findColour(Skin::kLinearSliderUnselected, true);
    }

    /// Gets the color for the selected portion of the slider.
    /// @return The selected color.
    virtual Colour getSelectedColor() const {
      if (isModulationKnob()) {
        Colour background = findColour(Skin::kWidgetBackground, true);
        if (active_)
          return findColour(Skin::kRotaryArc, true).interpolatedWith(background, 0.5f);
        return findColour(Skin::kRotaryArcDisabled, true).interpolatedWith(background, 0.5f);
      }
      if (isRotary()) {
        if (active_)
          return findColour(Skin::kRotaryArc, true);
        return findColour(Skin::kRotaryArcDisabled, true);
      }
      if (active_)
        return findColour(Skin::kLinearSlider, true);
      return findColour(Skin::kLinearSliderDisabled, true);
    }

    /// Gets the color for the thumb/handle.
    /// @return The thumb color.
    virtual Colour getThumbColor() const {
      if (isModulationKnob())
        return findColour(Skin::kRotaryArc, true);
      if (isRotary())
        return findColour(Skin::kRotaryHand, true);
      if (active_)
        return findColour(Skin::kLinearSliderThumb, true);
      return findColour(Skin::kLinearSliderThumbDisabled, true);
    }

    /// Computes the width of the slider track for linear sliders.
    /// @return The linear slider width in pixels.
    int getLinearSliderWidth();

    /// Sets the slider display values (positions, sizes) based on current style.
    void setSliderDisplayValues();

    /// Redraws the slider image or quad.
    /// @param skip_image If true, skips redrawing the image component.
    void redoImage(bool skip_image = false);

    /// Updates internal colors based on the current skin and state.
    void setColors();

    /// Finds a skin value from the parent component.
    /// @param value_id The skin value ID.
    /// @return The retrieved value or 0 if none found.
    virtual float findValue(Skin::ValueId value_id) const {
      if (parent_)
        return parent_->findValue(value_id);
      return 0.0f;
    }

    /// Sets an alpha value for the slider quad rendering.
    /// @param alpha The alpha value.
    /// @param reset If true, resets alpha animations.
    void setAlpha(float alpha, bool reset = false) { slider_quad_.setAlpha(alpha, reset); }

    /// Forces the quad to draw even when not visible.
    /// @param draw True to always draw.
    void setDrawWhenNotVisible(bool draw) { slider_quad_.setDrawWhenNotVisible(draw); }

    /// Gets the parent SynthSection if available.
    /// @return The parent SynthSection or nullptr.
    SynthSection* getSectionParent() { return parent_; }

  protected:
    SynthSection* parent_; ///< The parent SynthSection.

  private:
    Colour thumb_color_;
    Colour selected_color_;
    Colour unselected_color_;
    Colour background_color_;
    Colour mod_color_;

    bool modulation_knob_;
    float modulation_amount_;
    bool paint_to_image_;
    bool active_;
    bool bipolar_;
    OpenGlSliderQuad slider_quad_;
    OpenGlImageComponent image_component_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGlSlider)
};

/// @class SynthSlider
/// @brief A specialized slider with extended functionality for modulation, parameter control, popup menus, and text entry.
///
/// This class supports various display scales, text entry, popup menus for MIDI learn or clearing modulations,
/// and a wide range of customization. It integrates closely with the synthesizer's parameter architecture.
class SynthSlider : public OpenGlSlider, public TextEditor::Listener {
  public:
    /// Menu item IDs for the right-click popup menu.
    enum MenuId {
      kCancel = 0,
      kArmMidiLearn,
      kClearMidiLearn,
      kDefaultValue,
      kManualEntry,
      kClearModulations,
      kModulationList
    };

    static constexpr int kDefaultFormatLength = 5;
    static constexpr int kDefaultFormatDecimalPlaces = 5;
    static constexpr float kDefaultTextEntryWidthPercent = 0.9f;
    static constexpr float kDefaultTextEntryHeightPercent = 0.35f;
    static constexpr float kModulationSensitivity = 0.003f;
    static constexpr float kTextEntryHeightPercent = 0.6f;

    static constexpr float kSlowDragMultiplier = 0.1f;
    static constexpr float kDefaultSensitivity = 1.0f;

    static constexpr float kDefaultTextHeightPercentage = 0.7f;
    static constexpr float kDefaultRotaryDragLength = 200.0f;
    static constexpr float kRotaryModulationControlPercent = 0.26f;

    static constexpr float kLinearWidthPercent = 0.26f;
    static constexpr float kLinearHandlePercent = 1.2f;
    static constexpr float kLinearModulationPercent = 0.1f;

    /// @class SliderListener
    /// @brief Listener interface for receiving slider events such as mouse interactions, modulation changes, and GUI changes.
    class SliderListener {
      public:
        virtual ~SliderListener() { }
        virtual void hoverStarted(SynthSlider* slider) { }
        virtual void hoverEnded(SynthSlider* slider) { }
        virtual void mouseDown(SynthSlider* slider) { }
        virtual void mouseUp(SynthSlider* slider) { }
        virtual void beginModulationEdit(SynthSlider* slider) { }
        virtual void endModulationEdit(SynthSlider* slider) { }
        virtual void menuFinished(SynthSlider* slider) { }
        virtual void focusLost(SynthSlider* slider) { }
        virtual void doubleClick(SynthSlider* slider) { }
        virtual void modulationsChanged(const std::string& name) { }
        virtual void modulationAmountChanged(SynthSlider* slider) { }
        virtual void modulationRemoved(SynthSlider* slider) { }
        virtual void guiChanged(SynthSlider* slider) { }
    };

    /// Constructor.
    /// @param name The name of the slider.
    SynthSlider(String name);

    /// Mouse event overrides for custom behavior.
    virtual void mouseDown(const MouseEvent& e) override;
    virtual void mouseDrag(const MouseEvent& e) override;
    virtual void mouseEnter(const MouseEvent& e) override;
    virtual void mouseExit(const MouseEvent& e) override;
    virtual void mouseUp(const MouseEvent& e) override;
    virtual void mouseDoubleClick(const MouseEvent& e) override;
    virtual void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override;
    virtual void focusLost(FocusChangeType cause) override;

    /// Called when the slider value changes. Notifies GUIs.
    virtual void valueChanged() override;

    /// Retrieves the raw text from a value (no formatting).
    /// @param value The slider value.
    /// @return The raw text representation of the value.
    String getRawTextFromValue(double value);

    /// Retrieves the slider text from a value, applying formatting and prefix.
    /// @param value The slider value.
    /// @return The formatted text representation of the value.
    String getSliderTextFromValue(double value);

    /// Converts a value to a string representation.
    /// @param value The value to convert.
    /// @return The text representation of the value.
    String getTextFromValue(double value) override;

    /// Converts a string to a slider value.
    /// @param text The text to parse.
    /// @return The value represented by the text.
    double getValueFromText(const String& text) override;

    /// Adjusts a raw slider value to its display scale.
    /// @param value The raw value.
    /// @return The adjusted value.
    double getAdjustedValue(double value);

    /// Reverses the adjusted value back to the raw value.
    /// @param value The adjusted value.
    /// @return The raw value.
    double getValueFromAdjusted(double value);

    /// Sets the slider value from an adjusted value.
    /// @param value The adjusted value.
    void setValueFromAdjusted(double value);

    /// Called when the parent hierarchy changes, updates references to synthesizer.
    virtual void parentHierarchyChanged() override;

    /// Snaps the slider value to a special value if snap is enabled.
    /// @param attemptedValue The attempted slider value.
    /// @param dragMode The drag mode.
    /// @return The snapped value if applicable.
    virtual double snapValue(double attemptedValue, DragMode dragMode) override;

    /// TextEditor::Listener overrides for handling text entry.
    void textEditorTextChanged(TextEditor&) override {
      text_entry_->redoImage();
    }
    void textEditorReturnKeyPressed(TextEditor& editor) override;
    void textEditorFocusLost(TextEditor& editor) override;

    /// Sets the slider position from the current text in the text editor.
    void setSliderPositionFromText();

    /// Shows the text entry box for manual value entry.
    void showTextEntry();

    /// Determines if a popup should be shown (override for custom logic).
    /// @return True if popup should be shown.
    virtual bool shouldShowPopup() { return true; }

    /// Draws a shadow behind the slider if needed.
    /// @param g The graphics context.
    virtual void drawShadow(Graphics& g);

    /// Draws a shadow for rotary sliders.
    /// @param g The graphics context.
    void drawRotaryShadow(Graphics& g);

    /// Enables or disables snapping to a specific value.
    /// @param snap True to enable snapping.
    /// @param value The value to snap to.
    void snapToValue(bool snap, float value = 0.0) {
      snap_to_value_ = snap;
      snap_value_ = value;
    }

    /// Sets the scaling type of the parameter value.
    /// @param scaling_type The scaling type (linear, quadratic, etc.).
    void setScalingType(vital::ValueDetails::ValueScale scaling_type) {
      details_.value_scale = scaling_type;
    }

    /// Gets the scaling type.
    /// @return The scaling type.
    vital::ValueDetails::ValueScale getScalingType() const { return details_.value_scale; }

    /// Sets a lookup table for indexed parameters.
    /// @param lookup The lookup table.
    void setStringLookup(const std::string* lookup) {
      string_lookup_ = lookup;
    }

    /// Enables or disables mouse wheel scrolling.
    /// @param enabled True to enable scrolling.
    void setScrollEnabled(bool enabled) {
      scroll_enabled_ = enabled;
      setScrollWheelEnabled(enabled);
    }

    /// Gets the string lookup table.
    /// @return The lookup table.
    const std::string* getStringLookup() const { return string_lookup_; }

    /// Sets the display units.
    /// @param units The units string.
    void setUnits(const String& units) { details_.display_units = units.toStdString(); }

    /// Gets the display units.
    /// @return The units string.
    String getUnits() const { return details_.display_units; }

    /// Formats the value into a string.
    /// @param value The value to format.
    /// @return The formatted string.
    String formatValue(float value);

    /// Sets the default parameter range based on the parameter details.
    void setDefaultRange();

    /// Adds a slider listener to receive events.
    /// @param listener The listener to add.
    void addSliderListener(SliderListener* listener);

    /// Shows a popup display with the current value.
    /// @param primary If true, shows primary popup.
    void showPopup(bool primary);

    /// Hides the popup display.
    /// @param primary If true, hides primary popup.
    void hidePopup(bool primary);

    /// Sets the popup placement relative to the slider.
    /// @param placement The bubble placement.
    void setPopupPlacement(BubbleComponent::BubblePlacement placement) {
      popup_placement_ = placement;
    }

    /// Sets the modulation placement bubble direction.
    /// @param placement The bubble placement.
    void setModulationPlacement(BubbleComponent::BubblePlacement placement) {
      modulation_control_placement_ = placement;
    }

    /// Gets the current popup placement.
    /// @return The bubble placement.
    BubbleComponent::BubblePlacement getPopupPlacement() { return popup_placement_; }

    /// Gets the current modulation placement.
    /// @return The bubble placement.
    BubbleComponent::BubblePlacement getModulationPlacement() { return modulation_control_placement_; }

    /// Notifies GUI listeners of a value change.
    void notifyGuis();

    /// Handles the result of a popup menu action.
    /// @param result The selected menu item ID.
    void handlePopupResult(int result);

    /// Sets the slider's drag sensitivity.
    /// @param sensitivity The new sensitivity.
    void setSensitivity(double sensitivity) { sensitivity_ = sensitivity; }

    /// Gets the slider's drag sensitivity.
    /// @return The current sensitivity.
    double getSensitivity() { return sensitivity_; }

    /// Gets the modulation meter bounds if any.
    /// @return The bounds for drawing modulation meters.
    Rectangle<int> getModulationMeterBounds() const;

    /// Checks if the slider has a dedicated modulation area.
    /// @return True if modulation area is defined.
    bool hasModulationArea() const {
      return modulation_area_.getWidth();
    }

    /// Gets the modulation area.
    /// @return The modulation area bounds.
    Rectangle<int> getModulationArea() const {
      if (modulation_area_.getWidth())
        return modulation_area_;
      return getLocalBounds();
    }

    /// Sets the modulation area.
    /// @param area The new modulation area.
    void setModulationArea(Rectangle<int> area) { modulation_area_ = area; }

    /// Checks if the modulation is bipolar.
    /// @return True if bipolar modulation.
    bool isModulationBipolar() const { return bipolar_modulation_; }

    /// Checks if the modulation is stereo.
    /// @return True if stereo modulation.
    bool isModulationStereo() const { return stereo_modulation_; }

    /// Checks if modulation is bypassed.
    /// @return True if bypassed.
    bool isModulationBypassed() const { return bypass_modulation_; }

    /// Sets the text height percentage.
    /// @param percentage The text height percentage.
    void setTextHeightPercentage(float percentage) { text_height_percentage_ = percentage; }

    /// Gets the text height percentage.
    /// @return The text height percentage.
    float getTextHeightPercentage() { return text_height_percentage_; }

    /// Gets whether the mouse is hovering over the slider.
    /// @return A value indicating hover state.
    float mouseHovering() const { return hovering_; }

    /// Gets the modulation connections for this parameter.
    /// @return A vector of modulation connections.
    std::vector<vital::ModulationConnection*> getConnections();

    /// Sets the mouse wheel movement amount for value changes.
    /// @param movement The new movement increment.
    void setMouseWheelMovement(double movement) { mouse_wheel_index_movement_ = movement; }

    /// Sets the maximum display characters for formatting.
    /// @param characters The max characters.
    void setMaxDisplayCharacters(int characters) { max_display_characters_ = characters; }

    /// Sets the maximum decimal places for display.
    /// @param decimal_places The max decimal places.
    void setMaxDecimalPlaces(int decimal_places) { max_decimal_places_ = decimal_places; }

    /// Sets the size of the text entry in percentages of the component's size.
    /// @param width_percent The width percentage.
    /// @param height_percent The height percentage.
    void setTextEntrySizePercent(float width_percent, float height_percent) {
      text_entry_width_percent_ = width_percent;
      text_entry_height_percent_ = height_percent;
      redoImage();
    }

    /// Sets the width percentage used for text entry height calculation.
    /// @param percent The height percentage.
    void setTextEntryWidthPercent(float percent) { text_entry_height_percent_ = percent; redoImage(); }

    /// Sets an amount by which the index-based parameter changes when shift is held.
    /// @param shift_amount The shift index amount.
    void setShiftIndexAmount(int shift_amount) { shift_index_amount_ = shift_amount; }

    /// Sets whether to show a popup on hover.
    /// @param show True to show popup on hover.
    void setShowPopupOnHover(bool show) { show_popup_on_hover_ = show; }

    /// Sets a prefix for displayed values in the popup.
    /// @param prefix The popup prefix string.
    void setPopupPrefix(String prefix) { popup_prefix_ = prefix; }

    void setPopupSuffix(const juce::String& suffix) { popupSuffix_ = suffix; }

    /// Sets a scale factor for the knob size.
    /// @param scale The scale factor.
    void setKnobSizeScale(float scale) { knob_size_scale_ = scale; }

    float getKnobSizeScale() const override { return knob_size_scale_; }

    /// Uses a suffix (units) in the displayed values.
    /// @param use True to use suffix.
    void useSuffix(bool use) { use_suffix_ = use; }

    /// Sets an extra component to be used as a modulation target.
    /// @param component The extra modulation target component.
    void setExtraModulationTarget(Component* component) { extra_modulation_target_ = component; }

    /// Gets the extra modulation target component.
    /// @return The extra modulation target component.
    Component* getExtraModulationTarget() { return extra_modulation_target_; }

    /// Sets whether the modulation bar is on the right side (for text/curve).
    /// @param right True if modulation bar should be on the right side.
    void setModulationBarRight(bool right) { modulation_bar_right_ = right; }

    /// Checks if the modulation bar is on the right side.
    /// @return True if modulation bar is on the right.
    bool isModulationBarRight() { return modulation_bar_right_; }

    /// Sets a multiplier for display values.
    /// @param multiply The multiplier.
    void setDisplayMultiply(float multiply) { display_multiply_ = multiply; }

    /// Sets the exponential base for display conversions.
    /// @param base The exponential base.
    void setDisplayExponentialBase(float base) { display_exponential_base_ = base; }

    /// Overrides a specific Skin value.
    /// @param value_id The Skin Value ID.
    /// @param value The overriding value.
    void overrideValue(Skin::ValueId value_id, float value) {
      value_lookup_[value_id] = value;
    }

    /// Gets a Skin value, considering overrides.
    /// @param value_id The Skin Value ID.
    /// @return The found or overridden value.
    float findValue(Skin::ValueId value_id) const override {
      if (value_lookup_.count(value_id))
        return value_lookup_.at(value_id);
      return OpenGlSlider::findValue(value_id);
    }

    /// Sets alternate display settings.
    /// @param id The Skin Value ID to check.
    /// @param value The value that triggers alternate display.
    /// @param details The alternate ValueDetails.
    void setAlternateDisplay(Skin::ValueId id, float value, vital::ValueDetails details) {
      alternate_display_setting_ = { id, value };
      alternate_details_ = details;
    }

    /// Gets the appropriate ValueDetails for display (normal or alternate).
    /// @return The ValueDetails currently active.
    vital::ValueDetails* getDisplayDetails();

    /// Gets the OpenGL component used for the text editor.
    /// @return The text editor's OpenGlComponent.
    OpenGlComponent* getTextEditorComponent() { return text_entry_->getImageComponent(); }

  protected:
      /// Creates the popup menu items.
    /// @return The popup menu items.
    PopupItems createPopupMenu();

    /// Sets bounds for text entry in rotary mode.
    void setRotaryTextEntryBounds();

    /// Sets bounds for text entry in linear mode.
    void setLinearTextEntryBounds();

    /// Notifies listeners that modulation amount has changed.
    void notifyModulationAmountChanged();

    /// Notifies listeners that a modulation was removed.
    void notifyModulationRemoved();

    /// Notifies listeners that modulations changed.
    void notifyModulationsChanged();

    bool show_popup_on_hover_;
    String popup_prefix_;
    String popupSuffix_;
    bool scroll_enabled_;
    bool bipolar_modulation_;
    bool stereo_modulation_;
    bool bypass_modulation_;
    bool modulation_bar_right_;
    Rectangle<int> modulation_area_;
    bool sensitive_mode_;
    bool snap_to_value_;
    bool hovering_;
    bool has_parameter_assignment_;
    bool use_suffix_;
    float snap_value_;
    float text_height_percentage_;
    float knob_size_scale_;
    double sensitivity_;
    BubbleComponent::BubblePlacement popup_placement_;
    BubbleComponent::BubblePlacement modulation_control_placement_;
    int max_display_characters_;
    int max_decimal_places_;
    int shift_index_amount_;
    bool shift_is_multiplicative_;
    double mouse_wheel_index_movement_;
    float text_entry_width_percent_;
    float text_entry_height_percent_;

    std::map<Skin::ValueId, float> value_lookup_;
    Point<int> last_modulation_edit_position_;
    Point<int> mouse_down_position_;

    vital::ValueDetails details_;
    float display_multiply_;
    float display_exponential_base_;

    std::pair<Skin::ValueId, float> alternate_display_setting_;
    vital::ValueDetails alternate_details_;

    const std::string* string_lookup_;

    Component* extra_modulation_target_;
    SynthGuiInterface* synth_interface_;
    std::unique_ptr<OpenGlTextEditor> text_entry_;

    std::vector<SliderListener*> slider_listeners_;

//  // This is what the popup actually displays:
//  virtual juce::String getPopupText() {
//    // Typically it’s something like:
//    // return prefix_ + getDisplayValueAsString();
//    // Modify it to:
//    juce::String text = prefix_ + getDisplayValueAsString();
//    if (popupSuffix_.isNotEmpty())
//      text += ("\n" + popupSuffix_);   // or ("\n" + popupSuffix_), see below
//    return text;
//  }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthSlider)
};
