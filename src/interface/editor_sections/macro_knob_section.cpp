#include "macro_knob_section.h"

#include "fonts.h"
#include "modulation_button.h"
#include "synth_slider.h"
#include "synth_gui_interface.h"

/**
 * @class MacroLabel
 * @brief A custom label for displaying macro names.
 *
 * MacroLabel is a simple OpenGlImageComponent that draws text centered within its bounds.
 * It allows changing the displayed text and text size, and does not intercept mouse clicks.
 */
class MacroLabel : public OpenGlImageComponent {
  public:
    /**
     * @brief Constructs a MacroLabel.
     *
     * @param name The component name.
     * @param text The initial text to display.
     */
    MacroLabel(String name, String text) : OpenGlImageComponent(name), text_(std::move(text)), text_size_(1.0f) {
      setInterceptsMouseClicks(false, false);
    }

    /**
     * @brief Sets the label text.
     *
     * @param text The new label text.
     */
    void setText(String text) { text_ = text; redrawImage(true); }

    /**
     * @brief Sets the text size in point height.
     *
     * @param size The new text size.
     */
    void setTextSize(float size) { text_size_ = size; redrawImage(true); }

    /**
     * @brief Gets the current text.
     *
     * @return The current label text.
     */
    String getText() { return text_; }

    /**
     * @brief Paints the label text.
     *
     * @param g The JUCE Graphics context.
     */
    void paint(Graphics& g) override {
      g.setColour(findColour(Skin::kBodyText, true));
      g.setFont(Fonts::instance()->proportional_regular().withPointHeight(text_size_));
      g.drawText(text_, 0, 0, getWidth(), getHeight(), Justification::centred, false);
    }

  private:
    String text_;      ///< The text displayed by the label.
    float text_size_;  ///< The font size for the label text.
};

/**
 * @class SingleMacroSection
 * @brief Represents a single macro knob and associated controls (label, edit button, source button).
 *
 * Each macro knob includes:
 * - A SynthSlider for adjusting the macro value
 * - A ModulationButton for macro source assignment
 * - A MacroLabel to display and possibly rename the macro
 * - An optional text editor to rename the macro
 *
 * Inherits from SynthSection to ensure consistent styling with the rest of the UI.
 */
class SingleMacroSection : public SynthSection, public TextEditor::Listener {
  public:
    /**
     * @brief Constructs a SingleMacroSection.
     *
     * @param name The name of this macro section.
     * @param index The index of the macro (0-based).
     */
    SingleMacroSection(String name, int index) : SynthSection(name), index_(index) {
      std::string number = std::to_string(index_ + 1);
      std::string control_name = "macro_control_" + number;

      macro_knob_ = std::make_unique<SynthSlider>(control_name);
      addSlider(macro_knob_.get());
      macro_knob_->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
      macro_knob_->setPopupPlacement(BubbleComponent::right);

      macro_source_ = std::make_unique<ModulationButton>(control_name);
      addModulationButton(macro_source_.get());
      macro_source_->overrideText("");

      macro_label_ = std::make_unique<MacroLabel>("Macro Label " + number, "MACRO " + number);
      addOpenGlComponent(macro_label_.get());

      edit_label_ = std::make_unique<OpenGlShapeButton>("Edit " + number);
      addAndMakeVisible(edit_label_.get());
      addOpenGlComponent(edit_label_->getGlComponent());
      edit_label_->addListener(this);
      edit_label_->setShape(Paths::pencil());
      edit_label_->setTriggeredOnMouseDown(true);

      setSkinOverride(Skin::kMacro);

#if !defined(NO_TEXT_ENTRY)
      macro_label_editor_ = std::make_unique<OpenGlTextEditor>("Search");
      macro_label_editor_->addListener(this);
      macro_label_editor_->setSelectAllWhenFocused(true);
      macro_label_editor_->setMultiLine(false, false);
      macro_label_editor_->setJustification(Justification::centred);
      addChildComponent(macro_label_editor_.get());
      addOpenGlComponent(macro_label_editor_->getImageComponent());
#endif
    }

    /**
     * @brief Called when the component is resized.
     *
     * Positions the knob, label, source button, and edit button.
     */
    void resized() override {
      int knob_height = getHeight() / 2;
      int button_height = getHeight() - knob_height;
      int width = getWidth();

      macro_knob_->setBounds(0, 0, width, knob_height);
      placeRotaryOption(edit_label_.get(), macro_knob_.get());

      macro_source_->setBounds(0, knob_height, width, button_height);
      macro_source_->setFontSize(0);

      macro_label_->setBounds(getLabelBackgroundBounds(macro_knob_.get()));
      macro_label_->setTextSize(findValue(Skin::kLabelHeight));
    }

    /**
     * @brief Paints the background for the macro section.
     *
     * @param g The JUCE Graphics context.
     */
    void paintBackground(Graphics& g) override {
      paintBody(g);
      paintMacroSourceBackground(g);
      setLabelFont(g);

      drawLabelBackgroundForComponent(g, macro_knob_.get());
      paintKnobShadows(g);
      paintChildrenBackgrounds(g);
      paintBorder(g);
    }

    /**
     * @brief Paints a background shadow if needed.
     *
     * @param g The JUCE Graphics context.
     */
    void paintBackgroundShadow(Graphics& g) override { paintTabShadow(g); }

    /**
     * @brief Paints the background for the macro source area.
     *
     * @param g The JUCE Graphics context.
     */
    void paintMacroSourceBackground(Graphics& g) {
      g.saveState();
      Rectangle<int> bounds = getLocalArea(macro_source_.get(), macro_source_->getLocalBounds());
      g.reduceClipRegion(bounds);
      g.setOrigin(bounds.getTopLeft());
      macro_source_->paintBackground(g);
      g.restoreState();
    }

    /**
     * @brief Handles button click events, including toggling the label editor.
     *
     * @param clicked_button The button that was clicked.
     */
    void buttonClicked(Button* clicked_button) override {
      if (macro_label_editor_) {
        if (macro_label_editor_->isVisible()) {
          saveMacroLabel();
          return;
        }

        Rectangle<int> bounds = macro_label_->getBounds();
        float text_height = findValue(Skin::kLabelHeight);
        macro_label_editor_->setFont(Fonts::instance()->proportional_regular().withPointHeight(text_height));
        macro_label_editor_->setText(macro_label_->getText());
        macro_label_editor_->setBounds(bounds.translated(0, -1));
        macro_label_editor_->setVisible(true);
        macro_label_editor_->grabKeyboardFocus();
      }
    }

    /**
     * @brief Saves the macro label name after editing.
     *
     * Updates the synth's macro name and hides the text editor.
     */
    void saveMacroLabel() {
      macro_label_editor_->setVisible(false);
      String text = macro_label_editor_->getText().trim().toUpperCase();
      if (text.isEmpty())
        return;

      macro_label_->setText(text);

      SynthGuiInterface* synth_gui_interface = findParentComponentOfClass<SynthGuiInterface>();
      if (synth_gui_interface)
        synth_gui_interface->getSynth()->setMacroName(index_, text);
    }

    /**
     * @brief Called when the return key is pressed in the text editor.
     *
     * @param text_editor The text editor that triggered the event.
     */
    void textEditorReturnKeyPressed(TextEditor& text_editor) override {
      saveMacroLabel();
    }

    /**
     * @brief Called when the text editor loses focus.
     *
     * @param text_editor The text editor that lost focus.
     */
    void textEditorFocusLost(TextEditor& text_editor) override {
      saveMacroLabel();
    }

    /**
     * @brief Called when the escape key is pressed in the text editor.
     *
     * Hides the text editor without saving.
     *
     * @param editor The text editor.
     */
    void textEditorEscapeKeyPressed(TextEditor& editor) override {
      macro_label_editor_->setVisible(false);
    }

    /**
     * @brief Resets the macro label to the current synthesizer's macro name.
     */
    void reset() override {
      SynthGuiInterface* synth_gui_interface = findParentComponentOfClass<SynthGuiInterface>();
      if (synth_gui_interface == nullptr)
        return;

      macro_label_->setText(synth_gui_interface->getSynth()->getMacroName(index_));
    }

  private:
    int index_;  ///< The macro index.
    std::unique_ptr<SynthSlider> macro_knob_;         ///< The knob for adjusting the macro value.
    std::unique_ptr<ModulationButton> macro_source_;  ///< The button for selecting macro modulation sources.
    std::unique_ptr<MacroLabel> macro_label_;         ///< The label displaying the macro name.
    std::unique_ptr<OpenGlTextEditor> macro_label_editor_; ///< The text editor for renaming the macro.
    std::unique_ptr<OpenGlShapeButton> edit_label_;   ///< The button to toggle label editing.
};

MacroKnobSection::MacroKnobSection(String name) : SynthSection(name) {
  setWantsKeyboardFocus(true);
  for (int i = 0; i < vital::kNumMacros; ++i) {
    macros_[i] = std::make_unique<SingleMacroSection>(name + String(i), i);
    addSubSection(macros_[i].get());
  }

  setSkinOverride(Skin::kMacro);
}

MacroKnobSection::~MacroKnobSection() { }

void MacroKnobSection::paintBackground(Graphics& g) {
  paintChildrenBackgrounds(g);
}

void MacroKnobSection::resized() {
  int padding = getPadding();
  int knob_section_height = getKnobSectionHeight();
  int widget_margin = getWidgetMargin();
  int width = getWidth();
  int y = 0;
  for (int i = 0; i < vital::kNumMacros; ++i) {
    float next_y = (i + 1) * (2 * knob_section_height - widget_margin + padding);
    macros_[i]->setBounds(0, y, width, next_y - y - padding);
    y = next_y;
  }

  int last_y = macros_[vital::kNumMacros - 2]->getBottom() + padding;
  macros_[vital::kNumMacros - 1]->setBounds(0, last_y, width, getHeight() - last_y);

  reset();
  SynthSection::resized();
}
