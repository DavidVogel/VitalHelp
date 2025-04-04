#pragma once

#include "JuceHeader.h"

#include "bar_renderer.h"
#include "modulation_button.h"
#include "open_gl_component.h"
#include "open_gl_multi_quad.h"
#include "synth_module.h"
#include "synth_section.h"
#include "synth_slider.h"

#include <set>

class ExpandModulationButton;
class ModulationMatrix;
class ModulationMeter;
class ModulationDestination;

/**
 * @class ModulationAmountKnob
 * @brief A specialized SynthSlider that represents a single modulation amount control.
 *
 * ModulationAmountKnob provides popup menus for removing, bypassing, and changing modulation parameters
 * (bipolar, stereo). It can also represent auxiliary connections (chained modulation paths).
 */
class ModulationAmountKnob : public SynthSlider {
  public:
    /**
     * @enum MenuOptions
     * @brief Options in the context menu of the modulation amount knob.
     */
    enum MenuOptions {
        kDisconnect = 0xff,   ///< Removes the modulation connection.
        kToggleBypass,        ///< Toggles bypassing the modulation.
        kToggleBipolar,       ///< Toggles bipolar (positive and negative) modulation.
        kToggleStereo,        ///< Toggles stereo modulation mode.
    };

    /**
     * @class Listener
     * @brief Interface for objects interested in ModulationAmountKnob events.
     */
    class Listener {
      public:
        virtual ~Listener() {}
        /**
         * @brief Called when the modulation is disconnected.
         * @param modulation_knob The knob that triggered the event.
         */
        virtual void disconnectModulation(ModulationAmountKnob* modulation_knob) = 0;

        /**
         * @brief Called when modulation bypass state changes.
         * @param modulation_knob The knob that triggered the event.
         * @param bypass True if bypassed, false otherwise.
         */
        virtual void setModulationBypass(ModulationAmountKnob* modulation_knob, bool bypass) = 0;

        /**
         * @brief Called when modulation bipolar state changes.
         * @param modulation_knob The knob that triggered the event.
         * @param bipolar True if bipolar, false otherwise.
         */
        virtual void setModulationBipolar(ModulationAmountKnob* modulation_knob, bool bipolar) = 0;

        /**
         * @brief Called when modulation stereo state changes.
         * @param modulation_knob The knob that triggered the event.
         * @param stereo True if stereo, false otherwise.
         */
        virtual void setModulationStereo(ModulationAmountKnob* modulation_knob, bool stereo) = 0;
    };

    /**
     * @brief Constructs a ModulationAmountKnob.
     * @param name The name of the knob.
     * @param index The index identifying this modulation connection.
     */
    ModulationAmountKnob(String name, int index);

    void mouseDown(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;
    void mouseExit(const MouseEvent& e) override;

    /**
     * @brief Handles a selection from the modulation context menu.
     * @param result The menu option chosen.
     */
    void handleModulationMenuCallback(int result);

    /**
     * @brief Toggles the knob's visibility smoothly or immediately.
     * @param visible True to show, false to hide.
     */
    void makeVisible(bool visible);

    /**
     * @brief Hides the knob immediately without animations.
     */
    void hideImmediately();

    /**
     * @brief Marks this knob as representing the currently selected modulator.
     * @param current True if this knob is for the current modulator.
     */
    void setCurrentModulator(bool current);

    /**
     * @brief Associates the knob with a destination component (slider or UI element).
     * @param component The component representing the modulation destination.
     * @param name The destination parameter name.
     */
    void setDestinationComponent(Component* component, const std::string& name);

    /**
     * @brief Returns the internal color for drawing this knob.
     * @return A Colour object.
     */
    Colour getInternalColor();

    /**
     * @brief Sets the source parameter name for this modulation.
     * @param name The source parameter name.
     */
    void setSource(const std::string& name);

    Colour withBypassSaturation(Colour color) const {
      if (bypass_)
        return color.withSaturation(0.0f);
      return color;
    }

    virtual Colour getUnselectedColor() const override {
      return withBypassSaturation(SynthSlider::getUnselectedColor());
    }

    virtual Colour getSelectedColor() const override {
      return withBypassSaturation(SynthSlider::getSelectedColor());
    }

    virtual Colour getThumbColor() const override {
      return withBypassSaturation(SynthSlider::getThumbColor());
    }

    void setBypass(bool bypass) { bypass_ = bypass; setColors(); }
    void setStereo(bool stereo) { stereo_ = stereo; }
    void setBipolar(bool bipolar) { bipolar_ = bipolar; }
    bool isBypass() { return bypass_; }
    bool isStereo() { return stereo_; }
    bool isBipolar() { return bipolar_; }
    bool enteringValue() { return text_entry_ && text_entry_->isVisible(); }
    bool isCurrentModulator() { return current_modulator_; }
    int index() { return index_; }

    /**
     * @brief Sets an auxiliary modulation name for this knob (for chained modulations).
     * @param name The auxiliary modulation name.
     */
    void setAux(String name) {
      aux_name_ = name;
      setName(aux_name_);
      setModulationAmount(1.0f);
    }

    /**
     * @brief Checks if this knob currently has an auxiliary modulation.
     * @return True if auxiliary modulation is active.
     */
    bool hasAux() { return !aux_name_.isEmpty(); }

    /**
     * @brief Removes the auxiliary modulation connection.
     */
    void removeAux() {
      aux_name_ = "";
      setName(name_);
      setModulationAmount(0.0f);
    }

    /**
     * @brief Gets the knob's original name before auxiliary assignment.
     * @return The original name as a String.
     */
    String getOriginalName() { return name_; }

    /**
     * @brief Checks if the mouse is currently hovering over this knob.
     * @return True if hovering.
     */
    force_inline bool hovering() const {
      return hovering_;
    }

    /**
     * @brief Adds a listener interested in this knob's modulation events.
     * @param listener A pointer to a Listener.
     */
    void addModulationAmountListener(Listener* listener) { listeners_.push_back(listener); }

  private:
    void toggleBypass();

    std::vector<Listener*> listeners_;

    Point<int> mouse_down_position_;
    Component* color_component_;
    String aux_name_;
    String name_;
    bool editing_;
    int index_;
    bool showing_;
    bool hovering_;
    bool current_modulator_;
    bool bypass_;
    bool stereo_;
    bool bipolar_;
    bool draw_background_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationAmountKnob)
};

/**
 * @class ModulationExpansionBox
 * @brief A popup box that displays multiple ModulationAmountKnob controls in a grid.
 *
 * The ModulationExpansionBox is used when there are too many modulations to fit directly
 * around a ModulationButton. It creates a callout or popup area where multiple modulations
 * can be edited.
 */
class ModulationExpansionBox : public OpenGlQuad {
  public:
    /**
     * @class Listener
     * @brief Interface for objects interested in focus changes of the ModulationExpansionBox.
     */
    class Listener {
      public:
        virtual ~Listener() { }

        /**
         * @brief Called when the expansion box loses focus.
         */
        virtual void expansionFocusLost() = 0;
    };

    ModulationExpansionBox() : OpenGlQuad(Shaders::kRoundedRectangleFragment) { }

    void focusLost(FocusChangeType cause) override {
      OpenGlQuad::focusLost(cause);

      for (Listener* listener : listeners_)
        listener->expansionFocusLost();
    }

    /**
     * @brief Sets the amount controls displayed inside this expansion box.
     * @param amount_controls A vector of pointers to ModulationAmountKnob objects.
     */
    void setAmountControls(std::vector<ModulationAmountKnob*> amount_controls) { amount_controls_ = amount_controls; }

    /**
     * @brief Adds a Listener for expansion box focus events.
     * @param listener A pointer to a Listener.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

  private:
    std::vector<ModulationAmountKnob*> amount_controls_;
    std::vector<Listener*> listeners_;
};

/**
 * @class ModulationManager
 * @brief A top-level section for managing all modulation connections, amounts, and related UI components.
 *
 * The ModulationManager handles:
 * - All modulation connections (adding, removing, and modifying them).
 * - Displaying modulation meters on sliders and knobs.
 * - Showing and hiding ModulationAmountKnob controls for each modulation connection.
 * - Handling dragging and dropping of modulations, including auxiliary (chained) modulations.
 * - Managing popup and expansion boxes for complex modulation routings.
 *
 * It integrates with the synthesizer model (via SynthGuiInterface) to reflect and change modulation states.
 */
class ModulationManager : public SynthSection,
                          public ModulationButton::Listener,
                          public ModulationAmountKnob::Listener,
                          public SynthSlider::SliderListener,
                          public ModulationExpansionBox::Listener {
  public:
    static constexpr int kIndicesPerMeter = 6;
    static constexpr float kDragImageWidthPercent = 0.018f;

    /**
     * @brief Constructs a ModulationManager.
     * @param modulation_buttons A map of parameter names to their ModulationButton.
     * @param sliders A map of parameter names to their associated SynthSlider.
     * @param mono_modulations The monophonic modulation output map.
     * @param poly_modulations The polyphonic modulation output map.
     */
    ModulationManager(std::map<std::string, ModulationButton*> modulation_buttons,
                      std::map<std::string, SynthSlider*> sliders,
                      vital::output_map mono_modulations,
                      vital::output_map poly_modulations);

    /**
     * @brief Destructor.
     */
    ~ModulationManager();

    /**
     * @brief Creates a modulation meter for a given slider and places it into an OpenGlMultiQuad for visualization.
     * @param mono_total The monophonic modulation output for this parameter.
     * @param poly_total The polyphonic modulation output for this parameter (or nullptr if none).
     * @param slider The associated SynthSlider.
     * @param quads The OpenGlMultiQuad to draw the meter in.
     * @param index The index of this meter in the quad.
     */
    void createModulationMeter(const vital::Output* mono_total, const vital::Output* poly_total,
                               SynthSlider* slider, OpenGlMultiQuad* quads, int index);

    /**
     * @brief Creates internal structures for handling modulation of a given parameter (slider).
     * @param name The parameter name.
     * @param slider The slider controlling that parameter.
     * @param poly True if this parameter supports polyphonic modulations.
     */
    void createModulationSlider(std::string name, SynthSlider* slider, bool poly);

    /**
     * @brief Connects a modulation source to a destination parameter.
     * @param source The modulation source name.
     * @param destination The parameter name of the destination.
     */
    void connectModulation(std::string source, std::string destination);

    /**
     * @brief Removes a modulation connection between a source and a destination.
     * @param source The modulation source name.
     * @param destination The parameter name of the destination.
     */
    void removeModulation(std::string source, std::string destination);

    /**
     * @brief Sets the modulation value for a given modulation index.
     * @param index The modulation connection index.
     * @param value The new modulation amount.
     */
    void setModulationSliderValue(int index, float value);

    /**
     * @brief Sets the bipolar state for a given modulation index.
     * @param index The modulation connection index.
     * @param bipolar True if bipolar, false otherwise.
     */
    void setModulationSliderBipolar(int index, bool bipolar);

    /**
     * @brief Sets the modulation values and updates all connected auxiliary nodes.
     * @param index The modulation connection index.
     * @param value The new modulation amount.
     */
    void setModulationSliderValues(int index, float value);

    /**
     * @brief Adjusts the displayed range of a modulation slider based on parameter scaling.
     * @param index The modulation connection index.
     */
    void setModulationSliderScale(int index);

    /**
     * @brief Sets multiple modulation properties at once.
     * @param source The modulation source name.
     * @param destination The destination parameter name.
     * @param amount The modulation amount.
     * @param bipolar Whether modulation is bipolar.
     * @param stereo Whether modulation is stereo.
     * @param bypass Whether modulation is bypassed.
     */
    void setModulationValues(std::string source, std::string destination,
                             vital::mono_float amount, bool bipolar, bool stereo, bool bypass);
    void reset() override;
    void initAuxConnections();

    void resized() override;
    void parentHierarchyChanged() override;
    void updateModulationMeterLocations();
    void modulationAmountChanged(SynthSlider* slider) override;
    void modulationRemoved(SynthSlider* slider) override;

    void modulationDisconnected(vital::ModulationConnection* connection, bool last) override;
    void modulationSelected(ModulationButton* source) override;
    void modulationClicked(ModulationButton* source) override;
    void modulationCleared() override;
    bool hasFreeConnection();
    void startModulationMap(ModulationButton* source, const MouseEvent& e) override;
    void modulationDragged(const MouseEvent& e) override;
    void modulationWheelMoved(const MouseEvent& e, const MouseWheelDetails& wheel) override;
    void clearTemporaryModulation();
    void clearTemporaryHoverModulation();
    void modulationDraggedToHoverSlider(ModulationAmountKnob* hover_slider);
    void modulationDraggedToComponent(Component* component, bool bipolar);
    void setTemporaryModulationBipolar(Component* component, bool bipolar);
    void endModulationMap() override;
    void modulationLostFocus(ModulationButton* source) override;

    void expansionFocusLost() override {
      hideModulationAmountCallout();
    }

    void clearModulationSource();

    void disconnectModulation(ModulationAmountKnob* modulation_knob) override;
    void setModulationSettings(ModulationAmountKnob* modulation_knob);
    void setModulationBypass(ModulationAmountKnob* modulation_knob, bool bypass) override;
    void setModulationBipolar(ModulationAmountKnob* modulation_knob, bool bipolar) override;
    void setModulationStereo(ModulationAmountKnob* modulation_knob, bool stereo) override;

    void initOpenGlComponents(OpenGlWrapper& open_gl) override;
    void drawModulationDestinations(OpenGlWrapper& open_gl);
    void drawCurrentModulator(OpenGlWrapper& open_gl);
    void drawDraggingModulation(OpenGlWrapper& open_gl);
    void renderOpenGlComponents(OpenGlWrapper& open_gl, bool animate) override;
    void renderMeters(OpenGlWrapper& open_gl, bool animate);
    void renderSourceMeters(OpenGlWrapper& open_gl, int index);
    void updateSmoothModValues();
    void destroyOpenGlComponents(OpenGlWrapper& open_gl) override;
    void paintBackground(Graphics& g) override { positionModulationAmountSliders(); }

    void setModulationAmounts();
    void setVisibleMeterBounds();

    void hoverStarted(SynthSlider* slider) override;
    void hoverEnded(SynthSlider* slider) override;
    void menuFinished(SynthSlider* slider) override;
    void modulationsChanged(const std::string& name) override;
    int getIndexForModulationSlider(Slider* slider);
    int getModulationIndex(std::string source, std::string destination);
    vital::ModulationConnection* getConnectionForModulationSlider(Slider* slider);
    vital::ModulationConnection* getConnection(int index);
    vital::ModulationConnection* getConnection(const std::string& source, const std::string& dest);
    void mouseDown(SynthSlider* slider) override;
    void mouseUp(SynthSlider* slider) override;
    void doubleClick(SynthSlider* slider) override;
    void beginModulationEdit(SynthSlider* slider) override;
    void endModulationEdit(SynthSlider* slider) override;
    void sliderValueChanged(Slider* slider) override;
    void buttonClicked(Button* button) override;
    void hideUnusedHoverModulations();
    float getAuxMultiplier(int index);
    void addAuxConnection(int from_index, int to_index);
    void removeAuxDestinationConnection(int to_index);
    void removeAuxSourceConnection(int from_index);

  private:
    void setDestinationQuadBounds(ModulationDestination* destination);
    void makeCurrentModulatorAmountsVisible();
    void makeModulationsVisible(SynthSlider* destination, bool visible);
    void positionModulationAmountSlidersInside(const std::string& source,
                                               std::vector<vital::ModulationConnection*> connections);
    void positionModulationAmountSlidersCallout(const std::string& source,
                                                std::vector<vital::ModulationConnection*> connections);
    void showModulationAmountCallout(const std::string& source);
    void hideModulationAmountCallout();
    void positionModulationAmountSliders(const std::string& source);
    void positionModulationAmountSliders();
    bool enteringHoverValue();
    void showModulationAmountOverlay(ModulationAmountKnob* slider);
    void hideModulationAmountOverlay();

    std::unique_ptr<Component> modulation_destinations_;

    ModulationButton* current_source_;
    ExpandModulationButton* current_expanded_modulation_;
    ModulationDestination* temporarily_set_destination_;
    SynthSlider* temporarily_set_synth_slider_;
    ModulationAmountKnob* temporarily_set_hover_slider_;
    bool temporarily_set_bipolar_;
    OpenGlQuad drag_quad_;
    ModulationExpansionBox modulation_expansion_box_;
    OpenGlQuad current_modulator_quad_;
    OpenGlQuad editing_rotary_amount_quad_;
    OpenGlQuad editing_linear_amount_quad_;
    std::map<Viewport*, std::unique_ptr<OpenGlMultiQuad>> rotary_destinations_;
    std::map<Viewport*, std::unique_ptr<OpenGlMultiQuad>> linear_destinations_;
    std::map<Viewport*, std::unique_ptr<OpenGlMultiQuad>> rotary_meters_;
    std::map<Viewport*, std::unique_ptr<OpenGlMultiQuad>> linear_meters_;
    Point<int> mouse_drag_start_;
    Point<int> mouse_drag_position_;
    bool modifying_;
    bool dragging_;
    bool changing_hover_modulation_;

    ModulationButton* current_modulator_;
    std::map<std::string, ModulationButton*> modulation_buttons_;
    std::map<std::string, std::unique_ptr<ExpandModulationButton>> modulation_callout_buttons_;
    std::map<std::string, const vital::StatusOutput*> modulation_source_readouts_;
    std::map<std::string, vital::poly_float> smooth_mod_values_;
    std::map<std::string, bool> active_mod_values_;
    const vital::StatusOutput* num_voices_readout_;
    long long last_milliseconds_;
    std::unique_ptr<BarRenderer> modulation_source_meters_;

    std::map<std::string, ModulationDestination*> destination_lookup_;
    std::map<std::string, SynthSlider*> slider_model_lookup_;
    std::map<std::string, ModulationAmountKnob*> modulation_amount_lookup_;

    std::vector<std::unique_ptr<ModulationDestination>> all_destinations_;
    std::map<std::string, std::unique_ptr<ModulationMeter>> meter_lookup_;
    std::map<int, int> aux_connections_from_to_;
    std::map<int, int> aux_connections_to_from_;
    std::unique_ptr<ModulationAmountKnob> modulation_amount_sliders_[vital::kMaxModulationConnections];
    std::unique_ptr<ModulationAmountKnob> modulation_hover_sliders_[vital::kMaxModulationConnections];
    std::unique_ptr<ModulationAmountKnob> selected_modulation_sliders_[vital::kMaxModulationConnections];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationManager)
};

