#include "oscillator_advanced_section.h"

#include "oscillator_section.h"
#include "fonts.h"
#include "bar_renderer.h"
#include "synth_slider.h"
#include "synth_strings.h"
#include "synth_oscillator.h"
#include "text_selector.h"
#include "text_look_and_feel.h"

/**
 * @class OscillatorOptions
 * @brief A section providing toggleable oscillator options such as MIDI tracking and high-resolution wavetables.
 */
class OscillatorOptions : public SynthSection {
  public:
    /**
     * @brief Constructs an OscillatorOptions section for a specified oscillator index.
     * @param index The oscillator index this options section pertains to.
     */
    OscillatorOptions(int index) : SynthSection(String("OSC ") + String(index) + " OPTIONS"), index_(index) {
      createOffOverlay();
      oscillator_active_ = nullptr;

      String number(index);

      midi_track_ = std::make_unique<SynthButton>("osc_" + number + "_midi_track");
      addButton(midi_track_.get());
      midi_track_->setLookAndFeel(TextLookAndFeel::instance());
      midi_track_->setButtonText("NOTE TRACK");

      smooth_interpolation_ = std::make_unique<SynthButton>("osc_" + number + "_smooth_interpolation");
      addButton(smooth_interpolation_.get());
      smooth_interpolation_->setLookAndFeel(TextLookAndFeel::instance());
      smooth_interpolation_->setButtonText("HI-RES WAVETABLE");
    }

    /**
     * @brief Destructor.
     */
    virtual ~OscillatorOptions() { }

    /**
     * @brief Paints the background of the options section, including component backgrounds.
     * @param g The graphics context.
     */
    void paintBackground(Graphics& g) override {
      SynthSection::paintBackground(g);

      g.setColour(findColour(Skin::kTextComponentBackground, true));
      float rounding = findValue(Skin::kLabelBackgroundRounding);
      g.fillRoundedRectangle(midi_track_->getBounds().toFloat(), rounding);
      g.fillRoundedRectangle(smooth_interpolation_->getBounds().toFloat(), rounding);
    }

    /**
     * @brief Paints the background shadow if the oscillator is active.
     * @param g The graphics context.
     */
    void paintBackgroundShadow(Graphics& g) override {
      if (isActive())
        paintTabShadow(g);
    }

    /**
     * @brief Called when resized. Updates component bounds.
     */
    void resized() override {
      SynthSection::resized();

      int title_width = getTitleWidth();
      int widget_margin = getWidgetMargin();
      int x = title_width + widget_margin;
      int width = getWidth() - x - widget_margin;
      int section_height = getKnobSectionHeight();

      midi_track_->setBounds(x, widget_margin, width, section_height - 2 * widget_margin);

      int smooth_y = midi_track_->getBottom() + widget_margin;
      smooth_interpolation_->setBounds(x, smooth_y, width, getHeight() - smooth_y - widget_margin);
    }

    /**
     * @brief Sets all UI control values from a given control map.
     * @param controls A map of control values.
     */
    void setAllValues(vital::control_map& controls) override {
      SynthSection::setAllValues(controls);

      if (oscillator_active_)
        setActive(oscillator_active_->getToggleState());
    }

    /**
     * @brief Responds to button clicks within this section.
     * @param clicked_button The button that was clicked.
     */
    void buttonClicked(Button* clicked_button) override {
      if (clicked_button == oscillator_active_)
        setActive(oscillator_active_->getToggleState());
      else
        SynthSection::buttonClicked(clicked_button);
    }

    /**
     * @brief Associates this options section with an OscillatorSection for activation tracking.
     * @param oscillator A pointer to the OscillatorSection.
     */
    void passOscillatorSection(const OscillatorSection* oscillator) {
      oscillator_active_ = oscillator->activator();
      oscillator_active_->addListener(this);
      setActive(oscillator_active_->getToggleState());
    }

  private:
    int index_; ///< The oscillator index this option section controls.
    ToggleButton* oscillator_active_; ///< A pointer to the oscillator's activation toggle.

    std::unique_ptr<SynthButton> midi_track_; ///< MIDI tracking toggle button.
    std::unique_ptr<SynthButton> smooth_interpolation_; ///< High-resolution wavetable toggle button.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscillatorOptions)
};

/**
 * @class SpreadVisualizer
 * @brief A visualization component for unison voice spread of various oscillator parameters.
 */
class SpreadVisualizer : public BarRenderer {
  public:
    static constexpr int kNumSpreads = 3; ///< Number of parameter spreads visualized.

    /**
     * @brief Constructs a SpreadVisualizer for a given oscillator index and modulation maps.
     * @param index The oscillator index.
     * @param mono_modulations Map of monophonic modulation outputs.
     * @param poly_modulations Map of polyphonic modulation outputs.
     */
    SpreadVisualizer(int index, const vital::output_map& mono_modulations, const vital::output_map& poly_modulations) :
        BarRenderer(kNumSpreads * vital::SynthOscillator::kMaxUnison) {
      setAdditiveBlending(false);
      std::string number = std::to_string(index);

      // Retrieve modulation outputs for various parameters
      std::string voices_name = "osc_" + number + "_unison_voices";
      voices_outputs_ = {
        mono_modulations.at(voices_name),
        poly_modulations.at(voices_name)
      };

      std::string wave_frame_name = "osc_" + number + "_wave_frame";
      wave_frame_outputs_ = {
        mono_modulations.at(wave_frame_name),
        poly_modulations.at(wave_frame_name)
      };

      std::string spectral_morph_name = "osc_" + number + "_spectral_morph_amount";
      spectral_morph_outputs_ = {
        mono_modulations.at(spectral_morph_name),
        poly_modulations.at(spectral_morph_name)
      };

      std::string distortion_name = "osc_" + number + "_distortion_amount";
      distortion_outputs_ = {
        mono_modulations.at(distortion_name),
        poly_modulations.at(distortion_name)
      };

      std::string table_spread_name = "osc_" + number + "_frame_spread";
      table_spread_outputs_ = {
        mono_modulations.at(table_spread_name),
        poly_modulations.at(table_spread_name)
      };

      std::string spectral_spread_name = "osc_" + number + "_spectral_morph_spread";
      spectral_spread_outputs_ = {
        mono_modulations.at(spectral_spread_name),
        poly_modulations.at(spectral_spread_name)
      };

      std::string distortion_spread_name = "osc_" + number + "_distortion_spread";
      distortion_spread_outputs_ = {
        mono_modulations.at(distortion_spread_name),
        poly_modulations.at(distortion_spread_name)
      };

      voices_slider_ = nullptr;

      wave_frame_slider_ = nullptr;
      spectral_morph_slider_ = nullptr;
      distortion_slider_ = nullptr;

      table_spread_slider_ = nullptr;
      spectral_spread_slider_ = nullptr;
      distortion_spread_slider_ = nullptr;
    }

    /**
     * @brief Sets the slider controlling the number of unison voices.
     * @param slider Pointer to the SynthSlider controlling unison voices.
     */
    void setVoicesSlider(const SynthSlider* slider) { voices_slider_ = slider; }

    /**
     * @brief Sets the slider controlling the wave frame parameter.
     * @param slider The wave frame slider.
     */
    void setFrameSlider(const SynthSlider* slider) { wave_frame_slider_ = slider; }

    /**
     * @brief Sets the slider controlling the spectral morph parameter.
     * @param slider The spectral morph slider.
     */
    void setSpectralMorphSlider(const SynthSlider* slider) { spectral_morph_slider_ = slider; }

    /**
     * @brief Sets the slider controlling the distortion amount parameter.
     * @param slider The distortion slider.
     */
    void setDistortionSlider(const SynthSlider* slider) { distortion_slider_ = slider; }

    /**
     * @brief Sets the slider controlling the table spread (frame_spread).
     * @param slider The table spread slider.
     */
    void setTableSpreadSlider(const SynthSlider* slider) { table_spread_slider_ = slider; }

    /**
     * @brief Sets the slider controlling the spectral morph spread.
     * @param slider The spectral morph spread slider.
     */
    void setSpectralSpreadSlider(const SynthSlider* slider) { spectral_spread_slider_ = slider; }

    /**
     * @brief Sets the slider controlling the distortion spread.
     * @param slider The distortion spread slider.
     */
    void setDistortionSpreadSlider(const SynthSlider* slider) { distortion_spread_slider_ = slider; }

    /**
     * @brief Retrieves the sum of mono and poly outputs or a default value if not enabled.
     * @param outputs A pair of mono and poly outputs.
     * @param default_value A default value if not animating.
     * @param animate Whether visualization animation is active.
     * @return The combined poly_float value.
     */
    inline vital::poly_float getOutputsTotal(std::pair<vital::Output*, vital::Output*> outputs,
                                             vital::poly_float default_value, bool animate) {
      if (!outputs.first->owner->enabled() || !animate)
        return default_value;
      return outputs.first->trigger_value + outputs.second->trigger_value;
    }

    /**
     * @brief Paints the background of the visualizer.
     * @param g The graphics context.
     */
    void paintBackground(Graphics& g) override {
      if (!isVisible())
        return;

      g.setColour(findColour(Skin::kWidgetBackground, true));
      g.fillRoundedRectangle(getLocalBounds().toFloat(), findValue(Skin::kWidgetRoundedCorner));
    }

    /**
     * @brief Renders the bars representing the spread of unison voices.
     * @param open_gl The OpenGlWrapper for rendering.
     * @param animate Whether the visualization should animate parameter changes.
     */
    void render(OpenGlWrapper& open_gl, bool animate) override {
      if (voices_slider_ == nullptr)
        return;

      int voices = getOutputsTotal(voices_outputs_, voices_slider_->getValue(), animate)[0];
      voices = std::max(voices, 0);
      if (voices <= 2)
        return;

      vital::poly_float frame = getOutputsTotal(wave_frame_outputs_, wave_frame_slider_->getValue(), animate);
      vital::poly_float morph = getOutputsTotal(spectral_morph_outputs_, spectral_morph_slider_->getValue(), animate);
      vital::poly_float distortion = getOutputsTotal(distortion_outputs_, distortion_slider_->getValue(), animate);

      vital::poly_float frame_spread = getOutputsTotal(table_spread_outputs_,
                                                       table_spread_slider_->getValue(), animate);
      vital::poly_float morph_spread = getOutputsTotal(spectral_spread_outputs_,
                                                       spectral_spread_slider_->getValue(), animate);
      vital::poly_float distortion_spread = getOutputsTotal(distortion_spread_outputs_,
                                                            distortion_spread_slider_->getValue(), animate);

      setColor(findColour(Skin::kWidgetSecondary1, true));
      setBarWidth(2.0f / getWidth());

      float height_buffer = 2.0f * findValue(Skin::kWidgetMargin) / getHeight();
      float height = (2.0f - height_buffer) / kNumSpreads - height_buffer;
      float y = height_buffer - 1.0f;
      for (int s = 0; s < kNumSpreads; ++s) {
        int start = s * vital::SynthOscillator::kMaxUnison;
        for (int i = 0; i < vital::SynthOscillator::kMaxUnison; ++i) {
          setBottom(start + i, y + height);
          setY(start + i, y);
        }

        y += height + height_buffer;
      }

      float buffer = 2.0f * findValue(Skin::kWidgetMargin) / getWidth();
      float mult = 2.0f - 2.0f * buffer;
      float offset = -1.0f + buffer - 1.0f / getWidth();
      float frame_scale = 1.0f / (vital::kNumOscillatorWaveFrames - 1.0f);
      voices += voices % 2;
      for (int i = 0; i < voices; i += 2) {
        float t = 2.0f * i / (voices - 2.0f);
        vital::poly_float voice_frame = frame + frame_spread * t;
        voice_frame = vital::utils::clamp(voice_frame * frame_scale, 0.0f, 1.0f);
        vital::poly_float voice_morph = vital::utils::clamp(morph + morph_spread * t, 0.0f, 1.0f);
        vital::poly_float voice_distortion = vital::utils::clamp(distortion + distortion_spread * t, 0.0f, 1.0f);

        setX(i, voice_frame[0] * mult + offset);
        setX(i + 1, voice_frame[1] * mult + offset);

        setX(i + vital::SynthOscillator::kMaxUnison, voice_morph[0] * mult + offset);
        setX(i + vital::SynthOscillator::kMaxUnison + 1, voice_morph[1] * mult + offset);

        setX(i + 2 * vital::SynthOscillator::kMaxUnison, voice_distortion[0] * mult + offset);
        setX(i + 2 * vital::SynthOscillator::kMaxUnison + 1, voice_distortion[1] * mult + offset);
      }

      for (int s = 0; s < kNumSpreads; ++s) {
        int start = s * vital::SynthOscillator::kMaxUnison;
        for (int i = voices; i < vital::SynthOscillator::kMaxUnison; ++i)
          setX(start + i, -2.0f);
      }

      BarRenderer::render(open_gl, animate);
    }

  private:
    const SynthSlider* voices_slider_; ///< Slider for unison voices.

    const SynthSlider* wave_frame_slider_; ///< Wave frame parameter slider.
    const SynthSlider* spectral_morph_slider_; ///< Spectral morph parameter slider.
    const SynthSlider* distortion_slider_; ///< Distortion amount parameter slider.

    const SynthSlider* table_spread_slider_; ///< Table spread parameter slider.
    const SynthSlider* spectral_spread_slider_; ///< Spectral morph spread parameter slider.
    const SynthSlider* distortion_spread_slider_; ///< Distortion spread parameter slider.

    std::pair<vital::Output*, vital::Output*> voices_outputs_; ///< Voices outputs (mono, poly).

    std::pair<vital::Output*, vital::Output*> wave_frame_outputs_; ///< Wave frame outputs.
    std::pair<vital::Output*, vital::Output*> spectral_morph_outputs_; ///< Spectral morph outputs.
    std::pair<vital::Output*, vital::Output*> distortion_outputs_; ///< Distortion outputs.

    std::pair<vital::Output*, vital::Output*> table_spread_outputs_; ///< Table spread outputs.
    std::pair<vital::Output*, vital::Output*> spectral_spread_outputs_; ///< Spectral spread outputs.
    std::pair<vital::Output*, vital::Output*> distortion_spread_outputs_; ///< Distortion spread outputs.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpreadVisualizer)
};

/**
 * @class OscillatorUnison
 * @brief A section providing control over unison parameters such as blend, detune range, stereo spread, and other spreads.
 */
class OscillatorUnison : public SynthSection {
  public:
    /**
     * @brief Constructs an OscillatorUnison section for a specified oscillator.
     * @param index The oscillator index.
     * @param mono_modulations Map of monophonic modulation outputs.
     * @param poly_modulations Map of polyphonic modulation outputs.
     */
    OscillatorUnison(int index, const vital::output_map& mono_modulations, const vital::output_map& poly_modulations) :
        SynthSection(String("OSC ") + String(index) + " UNISON"), index_(index) {
      createOffOverlay();
      voices_slider_ = nullptr;
      oscillator_active_ = nullptr;

      String number(index);

      spectral_unison_ = std::make_unique<SynthButton>("osc_" + number + "_spectral_unison");
      addButton(spectral_unison_.get());
      spectral_unison_->setLookAndFeel(TextLookAndFeel::instance());
      spectral_unison_->setButtonText("SPECTRAL UNISON");

      stack_style_ = std::make_unique<TextSelector>("osc_" + number + "_stack_style");
      addSlider(stack_style_.get());
      stack_style_->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
      stack_style_->setLookAndFeel(TextLookAndFeel::instance());
      stack_style_->setLongStringLookup(strings::kUnisonStackNames);

      blend_ = std::make_unique<SynthSlider>("osc_" + number + "_unison_blend");
      addSlider(blend_.get());
      blend_->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);

      detune_range_ = std::make_unique<SynthSlider>("osc_" + number + "_detune_range");
      addSlider(detune_range_.get());
      detune_range_->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
      detune_range_->setLookAndFeel(TextLookAndFeel::instance());

      frame_spread_ = std::make_unique<SynthSlider>("osc_" + number + "_frame_spread");
      addSlider(frame_spread_.get());
      frame_spread_->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
      frame_spread_->setBipolar(true);

      distortion_spread_ = std::make_unique<SynthSlider>("osc_" + number + "_distortion_spread");
      addSlider(distortion_spread_.get());
      distortion_spread_->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
      distortion_spread_->setBipolar(true);

      spectral_morph_spread_ = std::make_unique<SynthSlider>("osc_" + number + "_spectral_morph_spread");
      addSlider(spectral_morph_spread_.get());
      spectral_morph_spread_->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
      spectral_morph_spread_->setBipolar(true);

      spread_visualizer_ = std::make_unique<SpreadVisualizer>(index, mono_modulations, poly_modulations);
      addOpenGlComponent(spread_visualizer_.get());
      spread_visualizer_->setTableSpreadSlider(frame_spread_.get());
      spread_visualizer_->setSpectralSpreadSlider(spectral_morph_spread_.get());
      spread_visualizer_->setDistortionSpreadSlider(distortion_spread_.get());

      stereo_spread_ = std::make_unique<SynthSlider>("osc_" + number + "_stereo_spread");
      addSlider(stereo_spread_.get());
      stereo_spread_->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    }

    /**
     * @brief Destructor.
     */
    virtual ~OscillatorUnison() = default;

    /**
     * @brief Paints the background and labels for the unison section.
     * @param g The graphics context.
     */
    void paintBackground(Graphics& g) override {
      SynthSection::paintBackground(g);
      setLabelFont(g);

      drawTextComponentBackground(g, stack_style_->getBounds(), true);
      drawTextComponentBackground(g, detune_range_->getBounds(), true);
      drawLabelForComponent(g, TRANS("STACK"), stack_style_.get(), true);
      drawLabelForComponent(g, TRANS("DETUNE RANGE"), detune_range_.get(), true);
      drawLabelForComponent(g, TRANS("UNISON BLEND"), blend_.get());
      drawLabelForComponent(g, TRANS("STEREO UNISON"), stereo_spread_.get());
      drawLabelForComponent(g, TRANS("TABLE SPREAD"), frame_spread_.get());
      drawLabelForComponent(g, TRANS("DIST SPREAD"), distortion_spread_.get());
      drawLabelForComponent(g, TRANS("SPECT SPREAD"), spectral_morph_spread_.get());
    }

    /**
     * @brief Paints background shadows if active.
     * @param g The graphics context.
     */
    void paintBackgroundShadow(Graphics& g) override {
      if (isActive())
        paintTabShadow(g);
    }

    /**
     * @brief Resizes the unison controls.
     */
    void resized() override {
      static constexpr float kTextComponentWidthRatio = 0.23f;
      SynthSection::resized();

      int knob_section_height = getKnobSectionHeight();
      int title_width = getTitleWidth();
      int widget_margin = getWidgetMargin();

      int text_width = getWidth() * kTextComponentWidthRatio - 2 * widget_margin;
      int text_height = knob_section_height - 2 * widget_margin;
      stack_style_->setBounds(title_width + widget_margin, widget_margin, text_width, text_height);
      detune_range_->setBounds(title_width + widget_margin, knob_section_height, text_width, text_height);

      int controls_x = detune_range_->getRight();
      placeKnobsInArea(Rectangle<int>(controls_x, 0, getWidth() - controls_x, knob_section_height),
                       { blend_.get(), nullptr, nullptr, nullptr });
      stack_style_->setBounds(stack_style_->getBounds().withTop(widget_margin));

      int knob_y2 = getHeight() - knob_section_height;
      placeKnobsInArea(Rectangle<int>(controls_x, knob_y2, getWidth() - controls_x, knob_section_height),
                       { stereo_spread_.get(), frame_spread_.get(),
                         spectral_morph_spread_.get(), distortion_spread_.get() });

      spread_visualizer_->setBounds(frame_spread_->getX(), widget_margin,
                                    distortion_spread_->getRight() - frame_spread_->getX(), text_height);
    }

    /**
     * @brief Passes a reference to the associated OscillatorSection for voice and activity tracking.
     * @param oscillator A pointer to the OscillatorSection.
     */
    void passOscillatorSection(const OscillatorSection* oscillator) {
      voices_slider_ = oscillator->getVoicesSlider();
      oscillator_active_ = oscillator->activator();
      oscillator_active_->addListener(this);
      voices_slider_->addListener(this);

      spread_visualizer_->setVoicesSlider(voices_slider_);
      spread_visualizer_->setFrameSlider(oscillator->getWaveFrameSlider());
      spread_visualizer_->setSpectralMorphSlider(oscillator->getSpectralMorphSlider());
      spread_visualizer_->setDistortionSlider(oscillator->getDistortionSlider());

      checkActive();
    }

    /**
     * @brief Checks if this section should be active based on voices and oscillator activation states.
     */
    void checkActive() {
      if (voices_slider_ && oscillator_active_)
        setActive(voices_slider_->getValue() > 1.0 && oscillator_active_->getToggleState());
    }

    /**
     * @brief Handles slider value changes.
     * @param changed_slider The slider that changed.
     */
    void sliderValueChanged(Slider* changed_slider) override {
      if (changed_slider == voices_slider_)
        checkActive();
      else
        SynthSection::sliderValueChanged(changed_slider);
    }

    /**
     * @brief Handles button clicks, including activation toggle.
     * @param clicked_button The button that was clicked.
     */
    void buttonClicked(Button* clicked_button) override {
      if (clicked_button == oscillator_active_)
        checkActive();
      else
        SynthSection::buttonClicked(clicked_button);
    }

    /**
     * @brief Sets control values and updates active state.
     * @param controls A map of control values.
     */
    void setAllValues(vital::control_map& controls) override {
      SynthSection::setAllValues(controls);
      checkActive();
    }

  private:
    int index_; ///< The oscillator index this unison section pertains to.

    std::unique_ptr<SynthButton> spectral_unison_; ///< Button enabling spectral unison.
    std::unique_ptr<TextSelector> stack_style_; ///< Selector for unison stack style.
    std::unique_ptr<SynthSlider> detune_range_; ///< Slider for detune range.
    std::unique_ptr<SynthSlider> stereo_spread_; ///< Slider for stereo spread.
    std::unique_ptr<SynthSlider> blend_; ///< Slider for unison blend factor.

    std::unique_ptr<SpreadVisualizer> spread_visualizer_; ///< Visualizer for unison spreads.
    std::unique_ptr<SynthSlider> frame_spread_; ///< Slider for frame (table) spread.
    std::unique_ptr<SynthSlider> distortion_spread_; ///< Slider for distortion spread.
    std::unique_ptr<SynthSlider> spectral_morph_spread_; ///< Slider for spectral morph spread.

    ToggleButton* oscillator_active_; ///< Pointer to oscillator activation toggle.
    SynthSlider* voices_slider_; ///< Pointer to voices slider for determining active state.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscillatorUnison)
};

OscillatorAdvancedSection::OscillatorAdvancedSection(int index,
                                                     const vital::output_map& mono_modulations,
                                                     const vital::output_map& poly_modulations) :
    SynthSection(String("OSC ") + String(index)) {
  oscillator_options_ = std::make_unique<OscillatorOptions>(index);
  addSubSection(oscillator_options_.get());

  oscillator_unison_ = std::make_unique<OscillatorUnison>(index, mono_modulations, poly_modulations);
  addSubSection(oscillator_unison_.get());
}

OscillatorAdvancedSection::~OscillatorAdvancedSection() { }

void OscillatorAdvancedSection::resized() {
  SynthSection::resized();
  int padding = findValue(Skin::kPadding);
  int options_width = getWidth() * 0.22f;
  oscillator_options_->setBounds(0, 0, options_width, getHeight());

  int unison_x = oscillator_options_->getRight() + padding;
  oscillator_unison_->setBounds(unison_x, 0, getWidth() - unison_x, getHeight());
}

void OscillatorAdvancedSection::passOscillatorSection(const OscillatorSection* oscillator) {
  oscillator_options_->passOscillatorSection(oscillator);
  oscillator_unison_->passOscillatorSection(oscillator);
}
