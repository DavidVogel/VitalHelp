#pragma once

#include "JuceHeader.h"
#include "fonts.h"
#include "paths.h"
#include "open_gl_image_component.h"
#include "open_gl_multi_quad.h"
#include "shaders.h"
#include "skin.h"
#include "synth_types.h"
#include "synth_button.h"

#include <functional>
#include <map>
#include <set>

class ModulationButton;
class OpenGlComponent;
class PresetSelector;
class SynthSlider;

/**
 * @struct PopupItems
 * @brief A hierarchical structure of popup menu items for a selector component.
 *
 * Each PopupItems instance can represent a menu (with nested items) or a single menu item,
 * identified by an ID, a name, and a selected state.
 */
struct PopupItems {
  int id;                ///< The numeric ID of this item.
  std::string name;      ///< The display name of this item.
  bool selected;         ///< Whether this item is currently selected.
  std::vector<PopupItems> items; ///< Nested items for submenus or hierarchical choices.

  PopupItems() : id(0), selected(false) { }
  PopupItems(std::string name) : id(0), name(std::move(name)), selected(false) { }

  PopupItems(int id, std::string name, bool selected, std::vector<PopupItems> items) {
    this->id = id;
    this->selected = selected;
    this->name = std::move(name);
    this->items = std::move(items);
  }

  /**
   * @brief Adds a new item as a submenu entry.
   * @param sub_id The ID of the new item.
   * @param sub_name The name of the new item.
   * @param sub_selected Initial selected state.
   */
  void addItem(int sub_id, const std::string& sub_name, bool sub_selected = false) {
    items.emplace_back(sub_id, sub_name, sub_selected, std::vector<PopupItems>());
  }

  /**
   * @brief Adds an existing PopupItems object as a submenu entry.
   * @param item The PopupItems to add.
   */
  void addItem(const PopupItems& item) { items.push_back(item); }

  /**
   * @brief Returns the number of nested items.
   * @return The count of items.
   */
  int size() const { return static_cast<int>(items.size()); }
};

/**
 * @class LoadingWheel
 * @brief An OpenGlQuad subclass that displays a rotating "loading" animation.
 *
 * This component draws a rotating arc to represent a loading spinner.
 * Once `completeRing()` is called, the arc expands until it covers the full circle.
 */
class LoadingWheel : public OpenGlQuad {
  public:
    LoadingWheel() : OpenGlQuad(Shaders::kRotaryModulationFragment), tick_(0), complete_(false), complete_ticks_(0) {
      setAlpha(1.0f);
    }

    void resized() override {
      OpenGlQuad::resized();

      Colour color = findColour(Skin::kWidgetAccent1, true);
      setColor(color);
      setModColor(color);
      setAltColor(color);
    }

  /**
   * @brief Renders the loading animation, updating the arc width and position over time.
   * @param open_gl Reference to the current OpenGL context wrapper.
   * @param animate Whether to animate changes.
   */
    virtual void render(OpenGlWrapper& open_gl, bool animate) override {
      static constexpr float kRotationMult = 0.05f;
      static constexpr float kWidthFrequency = 0.025f;
      static constexpr float kMinRads = 0.6f;
      static constexpr float kMaxRads = 4.0f;
      static constexpr float kRadRange = kMaxRads - kMinRads;
      static constexpr float kCompleteSpeed = 0.15f;
      static constexpr float kStartRads = -vital::kPi - 0.05f;

      tick_++;
      setStartPos(-tick_ * kRotationMult);

      float width = (sinf(tick_ * kWidthFrequency) * 0.5f + 0.5f) * kRadRange + kMinRads;
      if (complete_) {
        complete_ticks_++;
        width += kCompleteSpeed * complete_ticks_;
      }

      setShaderValue(0, kStartRads, 0);
      setShaderValue(0, kStartRads + width, 1);
      setShaderValue(0, kStartRads, 2);
      setShaderValue(0, kStartRads + width, 3);

      OpenGlQuad::render(open_gl, animate);
    }

  /**
   * @brief Transitions the loading wheel to the "complete" state where it fully expands the arc.
   */
    void completeRing() {
      complete_ = true;
    }

  private:
    int tick_;
    bool complete_;
    int complete_ticks_;
};

/**
 * @class AppLogo
 * @brief Displays the application's logo using paths and gradients.
 *
 * This component renders a stylized "Vital" V letter and ring with shadow and gradient fills.
 */
class AppLogo : public OpenGlImageComponent {
  public:
  /**
   * @brief Constructs an AppLogo with a given name.
   * @param name The component name.
   */
    AppLogo(String name) : OpenGlImageComponent(std::move(name)) {
      logo_letter_ = Paths::vitalV();
      logo_ring_ = Paths::vitalRing();
    }

    /**
     * @brief Paints the logo's letter and ring with gradients and shadows.
     * @param g Graphics context.
     */
    void paint(Graphics& g) override {
      const DropShadow shadow(findColour(Skin::kShadow, true), 10.0f, Point<int>(0, 0));

      logo_letter_.applyTransform(logo_letter_.getTransformToScaleToFit(getLocalBounds().toFloat(), true));
      logo_ring_.applyTransform(logo_ring_.getTransformToScaleToFit(getLocalBounds().toFloat(), true));

      shadow.drawForPath(g, logo_letter_);
      shadow.drawForPath(g, logo_ring_);

      Colour letter_top_color = findColour(Skin::kWidgetSecondary1, true);
      Colour letter_bottom_color = findColour(Skin::kWidgetSecondary2, true);
      Colour ring_top_color = findColour(Skin::kWidgetPrimary1, true);
      Colour ring_bottom_color = findColour(Skin::kWidgetPrimary2, true);
      ColourGradient letter_gradient(letter_top_color, 0.0f, 12.0f, letter_bottom_color, 0.0f, 96.0f, false);
      ColourGradient ring_gradient(ring_top_color, 0.0f, 12.0f, ring_bottom_color, 0.0f, 96.0f, false);
      g.setGradientFill(letter_gradient);
      g.fillPath(logo_letter_);

      g.setGradientFill(ring_gradient);
      g.fillPath(logo_ring_);
    }

  private:
    Path logo_letter_;
    Path logo_ring_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppLogo)
};

/**
 * @class SynthSection
 * @brief Base class for all synthesizer sections, providing UI layout, painting, and interaction logic.
 *
 * SynthSection serves as a container for UI elements in Vital. It supports nested sections,
 * OpenGL components, sliders, buttons, and modulations. It handles painting backgrounds,
 * labels, shadows, and complex UI layouts like joint controls and tempo controls.
 */
class SynthSection : public Component, public Slider::Listener,
                     public Button::Listener, public SynthButton::ButtonListener {
public:
    // Constants controlling layout and behavior:
    static constexpr int kDefaultPowerButtonOffset = 0;
    static constexpr float kPowerButtonPaddingPercent = 0.29f;
    static constexpr float kTransposeHeightPercent = 0.5f;
    static constexpr float kTuneHeightPercent = 0.4f;
    static constexpr float kJointModulationRadiusPercent = 0.1f;
    static constexpr float kJointModulationExtensionPercent = 0.6666f;
    static constexpr float kPitchLabelPercent = 0.33f;
    static constexpr float kJointLabelHeightPercent = 0.4f;
    static constexpr double kTransposeMouseSensitivity = 0.2;
    static constexpr float kJointLabelBorderRatioX = 0.05f;

    static constexpr int kDefaultBodyRounding = 4;
    static constexpr int kDefaultLabelHeight = 10;
    static constexpr int kDefaultLabelBackgroundHeight = 16;
    static constexpr int kDefaultLabelBackgroundWidth = 56;
    static constexpr int kDefaultLabelBackgroundRounding = 4;
    static constexpr int kDefaultPadding = 2;
    static constexpr int kDefaultPopupMenuWidth = 150;
    static constexpr int kDefaultDualPopupMenuWidth = 340;
    static constexpr int kDefaultStandardKnobSize = 32;
    static constexpr int kDefaultKnobThickness = 2;
    static constexpr float kDefaultKnobModulationAmountThickness = 2.0f;
    static constexpr int kDefaultKnobModulationMeterSize = 43;
    static constexpr int kDefaultKnobModulationMeterThickness = 4;
    static constexpr int kDefaultModulationButtonWidth = 64;
    static constexpr int kDefaultModFontSize = 10;
    static constexpr int kDefaultKnobSectionHeight = 64;
    static constexpr int kDefaultSliderWidth = 24;
    static constexpr int kDefaultTextWidth = 80;
    static constexpr int kDefaultTextHeight = 24;
    static constexpr int kDefaultWidgetMargin = 6;
    static constexpr float kDefaultWidgetFillFade = 0.3f;
    static constexpr float kDefaultWidgetLineWidth = 4.0f;
    static constexpr float kDefaultWidgetFillCenter = 0.0f;

    /**
     * @class OffOverlay
     * @brief A semi-transparent overlay shown when the section is inactive.
     */
    class OffOverlay : public OpenGlQuad {
      public:
        OffOverlay() : OpenGlQuad(Shaders::kColorFragment) { }

        void paintBackground(Graphics& g) override { }
    };

    /**
     * @brief Constructs a SynthSection with a given name.
     * @param name The name of the section.
     */
    SynthSection(const String& name);
    virtual ~SynthSection() = default;

    /**
     * @brief Sets the parent SynthSection.
     * @param parent The parent section.
     */
    void setParent(const SynthSection* parent) { parent_ = parent; }

    /**
     * @brief Finds a value in the skin overrides or from the parent if not found locally.
     * @param value_id The Skin value ID to lookup.
     * @return The found value or 0.0f if not found.
     */
    float findValue(Skin::ValueId value_id) const;

    /**
     * @brief Resets the section and all sub-sections.
     */
    virtual void reset();

    /**
     * @brief Called when the component is resized. Arranges layout of child components.
     */
    virtual void resized() override;

    /**
     * @brief Called when the component should paint itself.
     * @param g The Graphics context.
     */
    virtual void paint(Graphics& g) override;

    /**
     * @brief Paints the section name heading text vertically if sideways_heading_ is true.
     * @param g The Graphics context.
     */
    virtual void paintSidewaysHeadingText(Graphics& g);

    /**
     * @brief Paints the heading text for this section, either sideways or horizontally.
     * @param g The Graphics context.
     */
    virtual void paintHeadingText(Graphics& g);

    /**
     * @brief Paints the background of the section. Calls paintContainer, heading, knobs, children.
     * @param g The Graphics context.
     */
    virtual void paintBackground(Graphics& g);

    /**
     * @brief Sets skin values (colors, sizes) and applies them to sub-sections.
     * @param skin The Skin object.
     * @param top_level True if this is the top-level call.
     */
    virtual void setSkinValues(const Skin& skin, bool top_level);

    void setSkinOverride(Skin::SectionOverride skin_override) { skin_override_ = skin_override; }

    /**
     * @brief Requests a repaint of the background.
     */
    virtual void repaintBackground();

    /**
     * @brief Shows a file browser popup (e.g., for loading samples or wavetables).
     * @param owner The SynthSection requesting the browser.
     * @param bounds The area where the browser should appear.
     * @param directories A list of directories to browse.
     * @param extensions The file extensions allowed.
     * @param passthrough_name A name for a passthrough folder.
     * @param additional_folders_name A name for additional folders category.
     */
    void showPopupBrowser(SynthSection* owner, Rectangle<int> bounds, std::vector<File> directories,
                          String extensions, std::string passthrough_name, std::string additional_folders_name);

    /**
     * @brief Updates the currently visible popup browser if any.
     * @param owner The SynthSection that owns the browser.
     */
    void updatePopupBrowser(SynthSection* owner);

    /**
     * @brief Shows a popup selector with options.
     * @param source The component launching the selector.
     * @param position The position relative to source to show the popup.
     * @param options The PopupItems to display.
     * @param callback Callback when selection is made.
     * @param cancel Optional callback if cancelled.
     */
    void showPopupSelector(Component* source, Point<int> position, const PopupItems& options,
                           std::function<void(int)> callback, std::function<void()> cancel = { });

    /**
     * @brief Shows a dual popup selector for hierarchical selection.
     * @param source The component launching the selector.
     * @param position The position relative to source for the popup.
     * @param width Desired width of the popup.
     * @param options The PopupItems to display.
     * @param callback Callback when selection is made.
     */
    void showDualPopupSelector(Component* source, Point<int> position, int width,
                               const PopupItems& options, std::function<void(int)> callback);

    /**
     * @brief Shows a brief popup display (like a tooltip).
     * @param source The component requesting the display.
     * @param text The text to show.
     * @param placement The bubble placement relative to the source.
     * @param primary True if using the primary popup display slot.
     */
    void showPopupDisplay(Component* source, const std::string& text,
                          BubbleComponent::BubblePlacement placement, bool primary);

    /**
     * @brief Hides the currently shown popup display.
     * @param primary True if hiding the primary popup display.
     */
    void hidePopupDisplay(bool primary);

    /**
     * @brief Loads a file (e.g., a sample or wavetable). Overridden by subclasses.
     * @param file The file to load.
     */
    virtual void loadFile(const File& file) { }

    /**
     * @brief Gets the currently loaded file. Overridden by subclasses.
     * @return The currently loaded file.
     */
    virtual File getCurrentFile() { return File(); }

    /**
     * @brief Gets the name of the currently loaded file. Overridden by subclasses.
     * @return The file name string.
     */
    virtual std::string getFileName() { return ""; }

    /**
     * @brief Gets the author metadata of the currently loaded file. Overridden by subclasses.
     * @return The author string.
     */
    virtual std::string getFileAuthor() { return ""; }

    /**
     * @brief Paints the container background, body, heading, etc.
     * @param g The Graphics context.
     */
    virtual void paintContainer(Graphics& g);

    /**
     * @brief Paints the body background within given bounds.
     * @param g The Graphics context.
     * @param bounds The area to paint.
     */
    virtual void paintBody(Graphics& g, Rectangle<int> bounds);

    /**
     * @brief Paints the border around given bounds.
     * @param g The Graphics context.
     * @param bounds The rectangle area.
     */
    virtual void paintBorder(Graphics& g, Rectangle<int> bounds);

    /**
     * @brief Paints the section body background using the entire component area.
     * @param g The Graphics context.
     */
    virtual void paintBody(Graphics& g);

    /**
     * @brief Paints the border around the entire component.
     * @param g The Graphics context.
     */
    virtual void paintBorder(Graphics& g);

    /**
     * @brief Gets the width of shadow around components.
     * @return The shadow width in pixels.
     */
    int getComponentShadowWidth();

    /**
     * @brief Paints a tab-like shadow effect around the component.
     * @param g The Graphics context.
     */
    virtual void paintTabShadow(Graphics& g);

    /**
     * @brief Paints a tab shadow effect within specified bounds.
     * @param g The Graphics context.
     * @param bounds The area to apply the shadow.
     */
    void paintTabShadow(Graphics& g, Rectangle<int> bounds);

    /**
     * @brief Stub for painting background shadows. Overridden by subclasses if needed.
     * @param g The Graphics context.
     */
    virtual void paintBackgroundShadow(Graphics& g) { }

    /**
     * @brief Sets the size ratio for scaling UI elements.
     * @param ratio The scaling ratio.
     */
    virtual void setSizeRatio(float ratio);

    /**
     * @brief Paints knob shadows for all sliders.
     * @param g The Graphics context.
     */
    void paintKnobShadows(Graphics& g);

    /**
     * @brief Gets a suitable font for label text.
     * @return The label font.
     */
    Font getLabelFont();

    /**
     * @brief Sets the Graphics context font and color for labels.
     * @param g The Graphics context.
     */
    void setLabelFont(Graphics& g);

    /**
     * @brief Draws a rectangular connection between labels of two components.
     * @param g The Graphics context.
     * @param left The left component.
     * @param right The right component.
     */
    void drawLabelConnectionForComponents(Graphics& g, Component* left, Component* right);

    /**
     * @brief Draws a background for a label area.
     * @param g The Graphics context.
     * @param bounds The rectangle area for the label.
     * @param text_component True if drawing for a text component.
     */
    void drawLabelBackground(Graphics& g, Rectangle<int> bounds, bool text_component = false);

    /**
     * @brief Draws label background for a specific component.
     * @param g The Graphics context.
     * @param component The component needing label background.
     */
    void drawLabelBackgroundForComponent(Graphics& g, Component* component);

    /**
     * @brief Divides an area into equal sections with buffering, returns the specified section.
     * @param full_area The full rectangle area.
     * @param num_sections Total number of sections.
     * @param section The index of the section to retrieve.
     * @param buffer The spacing between sections.
     * @return The rectangle for the specified section.
     */
    Rectangle<int> getDividedAreaBuffered(Rectangle<int> full_area, int num_sections, int section, int buffer);

    /**
     * @brief Divides an area into equal sections without extra buffering, returns the specified section.
     * @param full_area The full rectangle area.
     * @param num_sections Total number of sections.
     * @param section The index of the section to retrieve.
     * @param buffer The spacing between sections.
     * @return The rectangle for the specified section.
     */
    Rectangle<int> getDividedAreaUnbuffered(Rectangle<int> full_area, int num_sections, int section, int buffer);

    /**
     * @brief Gets the background bounds for a label.
     * @param bounds The component bounds.
     * @param text_component True if for a text component.
     * @return The label background rectangle.
     */
    Rectangle<int> getLabelBackgroundBounds(Rectangle<int> bounds, bool text_component = false);

    /**
     * @brief Gets the label background bounds for a component.
     * @param component The component.
     * @param text_component True if for a text component.
     * @return The label background rectangle.
     */
    Rectangle<int> getLabelBackgroundBounds(Component* component, bool text_component = false) {
      return getLabelBackgroundBounds(component->getBounds(), text_component);
    }

    /**
     * @brief Draws a label text below a component.
     * @param g The Graphics context.
     * @param text The label text.
     * @param component The reference component.
     * @param text_component True if for a text component.
     */
    void drawLabel(Graphics& g, String text, Rectangle<int> component_bounds, bool text_component = false);

    /**
     * @brief Draws a label for a given component.
     * @param g The Graphics context.
     * @param text The label text.
     * @param component The component to label.
     * @param text_component True if for a text component.
     */
    void drawLabelForComponent(Graphics& g, String text, Component* component, bool text_component = false) {
      drawLabel(g, std::move(text), component->getBounds(), text_component);
    }

    /**
     * @brief Draws text below a component with optional padding.
     * @param g The Graphics context.
     * @param text The text to draw.
     * @param component The reference component.
     * @param space Vertical space below the component.
     * @param padding Horizontal padding.
     */
    void drawTextBelowComponent(Graphics& g, String text, Component* component, int space, int padding = 0);

    /**
     * @brief Paints shadows for child sections.
     * @param g The Graphics context.
     */
    virtual void paintChildrenShadows(Graphics& g);

    /**
     * @brief Paints the backgrounds for all child sections.
     * @param g The Graphics context.
     */
    void paintChildrenBackgrounds(Graphics& g);

    /**
     * @brief Paints the backgrounds for all OpenGL child components.
     * @param g The Graphics context.
     */
    void paintOpenGlChildrenBackgrounds(Graphics& g);

    /**
     * @brief Paints a child's background specifically.
     * @param g The Graphics context.
     * @param child The child SynthSection.
     */
    void paintChildBackground(Graphics& g, SynthSection* child);

    /**
     * @brief Paints a child's shadow specifically.
     * @param g The Graphics context.
     * @param child The child SynthSection.
     */
    void paintChildShadow(Graphics& g, SynthSection* child);

    /**
     * @brief Paints the background of an OpenGlComponent child.
     * @param g The Graphics context.
     * @param child The OpenGlComponent child.
     */
    void paintOpenGlBackground(Graphics& g, OpenGlComponent* child);

    /**
     * @brief Draws a background for a text component area.
     * @param g The Graphics context.
     * @param bounds The area to draw.
     * @param extend_to_label True to extend background up to label area.
     */
    void drawTextComponentBackground(Graphics& g, Rectangle<int> bounds, bool extend_to_label);

    /**
     * @brief Draws a divider line for tempo-related controls.
     * @param g The Graphics context.
     * @param sync The tempo sync component.
     */
    void drawTempoDivider(Graphics& g, Component* sync);

    /**
     * @brief Initializes all OpenGL components in this section and sub-sections.
     * @param open_gl The OpenGlWrapper context.
     */
    virtual void initOpenGlComponents(OpenGlWrapper& open_gl);

    /**
     * @brief Renders all OpenGL components in this section and sub-sections.
     * @param open_gl The OpenGlWrapper context.
     * @param animate True if animation is desired.
     */
    virtual void renderOpenGlComponents(OpenGlWrapper& open_gl, bool animate);

    /**
     * @brief Destroys all OpenGL components in this section and sub-sections.
     * @param open_gl The OpenGlWrapper context.
     */
    virtual void destroyOpenGlComponents(OpenGlWrapper& open_gl);

    /**
     * @brief Called when a slider value changes. Updates the synth parameter accordingly.
     * @param moved_slider The slider that changed.
     */
    virtual void sliderValueChanged(Slider* moved_slider) override;

    /**
     * @brief Called when a button is clicked. Updates the synth parameter accordingly.
     * @param clicked_button The button clicked.
     */
    virtual void buttonClicked(Button* clicked_button) override;

    /**
     * @brief Called when a SynthButton state changes (GUI interaction).
     * @param button The SynthButton changed.
     */
    virtual void guiChanged(SynthButton* button) override;

    /**
     * @brief Gets all sliders registered in this section.
     * @return A map of slider name to pointer.
     */
    std::map<std::string, SynthSlider*> getAllSliders() { return all_sliders_; }

    /**
     * @brief Gets all toggle buttons registered in this section.
     * @return A map of button name to pointer.
     */
    std::map<std::string, ToggleButton*> getAllButtons() { return all_buttons_; }

    /**
     * @brief Gets all modulation buttons registered in this section.
     * @return A map of modulation button name to pointer.
     */
    std::map<std::string, ModulationButton*> getAllModulationButtons() {
      return all_modulation_buttons_;
    }

    /**
     * @brief Sets the active state of this section and sub-sections.
     * @param active True to activate, false to deactivate.
     */
    virtual void setActive(bool active);

    /**
     * @brief Checks if the section is currently active.
     * @return True if active, false if not.
     */
    bool isActive() const { return active_; }

    /**
     * @brief Triggers animation state change in sub-sections if needed.
     * @param animate True if animation is desired.
     */
    virtual void animate(bool animate);

    /**
     * @brief Sets values for all known parameters from a control map.
     * @param controls The map of parameter names to control objects.
     */
    virtual void setAllValues(vital::control_map& controls);

    /**
     * @brief Sets a single parameter value for a known control.
     * @param name The parameter name.
     * @param value The new value.
     * @param notification Whether to send notifications to listeners.
     */
    virtual void setValue(const std::string& name, vital::mono_float value, NotificationType notification);

    /**
     * @brief Adds a modulation button to this section.
     * @param button The ModulationButton pointer.
     * @param show True to add to visible layout.
     */
    void addModulationButton(ModulationButton* button, bool show = true);

    /**
     * @brief Adds a subsection (another SynthSection) as a child.
     * @param section The sub-section to add.
     * @param show True to make it visible.
     */
    void addSubSection(SynthSection* section, bool show = true);

    /**
     * @brief Removes a previously added subsection.
     * @param section The sub-section to remove.
     */
    void removeSubSection(SynthSection* section);

    /**
     * @brief Enables or disables scroll wheel support for this section and sub-sections.
     * @param enabled True to enable scroll wheel changes, false to disable.
     */
    virtual void setScrollWheelEnabled(bool enabled);

    /**
     * @brief Gets the activator toggle button if any.
     * @return Pointer to the activator ToggleButton.
     */
    ToggleButton* activator() const { return activator_; }

    /**
     * @brief Sets custom skin values for this section.
     * @param values A map of Skin::ValueId to float values.
     */
    void setSkinValues(std::map<Skin::ValueId, float> values) { value_lookup_ = std::move(values); }

    /**
     * @brief Sets a single skin value override.
     * @param id The Skin::ValueId enum.
     * @param value The value to set.
     */
    void setSkinValue(Skin::ValueId id, float value) { value_lookup_[id] = value; }

    float getTitleWidth();
    float getPadding();
    float getPowerButtonOffset() const { return size_ratio_ * kDefaultPowerButtonOffset; }
    float getKnobSectionHeight();
    float getSliderWidth();
    float getSliderOverlap();
    float getSliderOverlapWithSpace();
    float getTextComponentHeight();
    float getStandardKnobSize();
    float getTotalKnobHeight() { return getStandardKnobSize(); }
    float getTextSectionYOffset();
    float getModButtonWidth();
    float getModFontSize();
    float getWidgetMargin();
    float getWidgetRounding();
    float getSizeRatio() const { return size_ratio_; }
    int getPopupWidth() const { return kDefaultPopupMenuWidth * size_ratio_; }
    int getDualPopupWidth() const { return kDefaultDualPopupMenuWidth * size_ratio_; }

  protected:
    void setSliderHasHzAlternateDisplay(SynthSlider* slider);
    void setSidewaysHeading(bool sideways) { sideways_heading_ = sideways; }
    void addToggleButton(ToggleButton* button, bool show);
    void addButton(OpenGlToggleButton* button, bool show = true);
    void addButton(OpenGlShapeButton* button, bool show = true);
    void addSlider(SynthSlider* slider, bool show = true, bool listen = true);
    void addOpenGlComponent(OpenGlComponent* open_gl_component, bool to_beginning = false);
    void setActivator(SynthButton* activator);
    void createOffOverlay();
    void setPresetSelector(PresetSelector* preset_selector, bool half = false) {
      preset_selector_ = preset_selector;
      preset_selector_half_width_ = half;
    }
    void paintJointControlSliderBackground(Graphics& g, int x, int y, int width, int height);
    void paintJointControlBackground(Graphics& g, int x, int y, int width, int height);
    void paintJointControl(Graphics& g, int x, int y, int width, int height, const std::string& name);
    void placeJointControls(int x, int y, int width, int height,
                            SynthSlider* left, SynthSlider* right, Component* widget = nullptr);
    void placeTempoControls(int x, int y, int width, int height, SynthSlider* tempo, SynthSlider* sync);
    void placeRotaryOption(Component* option, SynthSlider* rotary);
    void placeKnobsInArea(Rectangle<int> area, std::vector<Component*> knobs);

    void lockCriticalSection();
    void unlockCriticalSection();
    Rectangle<int> getPresetBrowserBounds();
    int getTitleTextRight();
    Rectangle<int> getPowerButtonBounds();
    Rectangle<int> getTitleBounds();
    float getDisplayScale() const;
    virtual int getPixelMultiple() const;

    std::map<Skin::ValueId, float> value_lookup_;

    std::vector<SynthSection*> sub_sections_;
    std::vector<OpenGlComponent*> open_gl_components_;

    std::map<std::string, SynthSlider*> slider_lookup_;
    std::map<std::string, Button*> button_lookup_;
    std::map<std::string, ModulationButton*> modulation_buttons_;

    std::map<std::string, SynthSlider*> all_sliders_;
    std::map<std::string, ToggleButton*> all_buttons_;
    std::map<std::string, ModulationButton*> all_modulation_buttons_;

    const SynthSection* parent_;
    SynthButton* activator_;
    PresetSelector* preset_selector_;
    bool preset_selector_half_width_;
    std::unique_ptr<OffOverlay> off_overlay_;

    Skin::SectionOverride skin_override_;
    float size_ratio_;
    bool active_;
    bool sideways_heading_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthSection)

};
