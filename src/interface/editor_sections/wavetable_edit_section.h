#pragma once

#include "JuceHeader.h"

#include "synth_section.h"
#include "preset_selector.h"
#include "wave_frame.h"
#include "wave_source_editor.h"
#include "wavetable.h"
#include "wavetable_component_list.h"
#include "wavetable_component_overlay.h"
#include "wavetable_creator.h"
#include "wavetable_organizer.h"
#include "wavetable_playhead.h"

class BarRenderer;
class WavetablePlayheadInfo;

/**
 * @class WavetableEditSection
 * @brief A UI section for editing, visualizing, and managing wavetables.
 *
 * This section provides a sophisticated interface for loading, editing,
 * and saving wavetables. Users can manipulate time- and frequency-domain
 * representations, resynthesize wavetables, and manage their frame
 * components. The section uses multiple subcomponents such as waveform
 * editors, frequency domain bar renderers, component lists, organizers,
 * and overlays to facilitate detailed wavetable editing.
 */
class WavetableEditSection : public SynthSection,
                             public PresetSelector::Listener,
                             WavetableOrganizer::Listener,
                             WavetableComponentList::Listener,
                             WavetablePlayhead::Listener,
                             WavetableComponentOverlay::Listener {
  public:
    static constexpr float kObscureAmount = 0.4f;  ///< Opacity factor for obscuring certain visuals.
    static constexpr float kAlphaFade = 0.3f;      ///< Alpha fade factor for overlay visuals.

    /**
     * @brief Computes a zoom scale factor from a zoom menu selection.
     * @param zoom The menu selection (e.g., kZoom1, kZoom2, etc.).
     * @return The computed scale factor (e.g., 1x, 2x, 4x zoom).
     */
    static inline float getZoomScale(int zoom) {
      return 1 << (zoom - kZoom1);
    }

    /**
     * @brief Extracts wavetable-specific data embedded in a .wav file's chunk.
     * @param input_stream Pointer to the InputStream of the .wav file.
     * @return A string containing wavetable data.
     */
    static String getWavetableDataString(InputStream* input_stream);

    /// Menu items for main menu actions.
    enum MenuItems {
      kCancelled,
      kSaveAsWavetable,
      kImportWavetable,
      kExportWavetable,
      kExportWav,
      kResynthesizeWavetable,
      kNumMenuItems
    };

    /// Menu items for the bar editor settings.
    enum BarEditorMenu {
      kCancel = 0,
      kPowerScale,
      kAmplitudeScale,
      kZoom1,
      kZoom2,
      kZoom4,
      kZoom8,
      kZoom16,
    };

    /**
     * @brief Constructs a WavetableEditSection.
     * @param index The oscillator index for which we are editing the wavetable.
     * @param wavetable_creator Pointer to a WavetableCreator managing wavetable data.
     */
    WavetableEditSection(int index, WavetableCreator* wavetable_creator);

    /** @brief Destructor. */
    virtual ~WavetableEditSection();

    /**
     * @brief Gets the bounds of the frame editing area.
     * @return The rectangle representing the editing area.
     */
    Rectangle<int> getFrameEditBounds();

    /**
     * @brief Gets the bounds of the timeline area.
     * @return The rectangle representing the timeline area.
     */
    Rectangle<int> getTimelineBounds();

    /**
     * @brief Paints the background of the wavetable edit section, including children.
     * @param g The Graphics context.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Paints any background shadow or tab shadows for the section.
     * @param g The Graphics context.
     */
    void paintBackgroundShadow(Graphics& g) override;

    /**
     * @brief Called when the component is resized, arranges the layout of UI elements.
     */
    void resized() override;

    /**
     * @brief Resets the editing section, clearing and re-initializing components.
     */
    void reset() override;

    /**
     * @brief Called when visibility changes, e.g., updating UI if made visible.
     */
    void visibilityChanged() override;

    /**
     * @brief Handles mouse wheel movement for zooming frequency domain views.
     * @param e The mouse event.
     * @param wheel The wheel details.
     */
    void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override;

    /**
     * @brief Callback when a wavetable component is added to the list.
     * @param component The added WavetableComponent.
     */
    void componentAdded(WavetableComponent* component) override;

    /**
     * @brief Callback when a wavetable component is removed.
     * @param component The removed WavetableComponent.
     */
    void componentRemoved(WavetableComponent* component) override;

    void componentsReordered() override { }

    /**
     * @brief Callback when components in the list change.
     */
    void componentsChanged() override;

    /**
     * @brief Gets the top section height for title and controls.
     * @return The height in pixels of the top area.
     */
    int getTopHeight() {
      static constexpr int kTopHeight = 48;
      return size_ratio_ * kTopHeight;
    }

    /**
     * @brief Callback for when the playhead moves to a different frame.
     * @param position The new frame position.
     */
    void playheadMoved(int position) override;

    /**
     * @brief Sets the WaveFrame slider to reflect current editing frame position.
     * @param slider Pointer to the wave frame slider.
     */
    void setWaveFrameSlider(Slider* slider) { wave_frame_slider_ = slider; }

    /**
     * @brief Callback when a frame finishes editing, triggers a waveform re-render.
     */
    void frameDoneEditing() override;

    /**
     * @brief Callback when a frame changes, triggers a display re-render.
     */
    void frameChanged() override;

    /**
     * @brief Callback when 'previous' is clicked, attempts to load previous wavetable.
     */
    void prevClicked() override;

    /**
     * @brief Callback when 'next' is clicked, attempts to load next wavetable.
     */
    void nextClicked() override;

    /**
     * @brief Callback when text is clicked, possibly showing a browser to load wavetable.
     * @param e The mouse event.
     */
    void textMouseDown(const MouseEvent& e) override;

    /**
     * @brief Loads the default wavetable (initializes a blank state).
     */
    void loadDefaultWavetable();

    /**
     * @brief Saves the current wavetable state as a file.
     */
    void saveAsWavetable();

    /**
     * @brief Imports an external wavetable file.
     */
    void importWavetable();

    /**
     * @brief Exports the current wavetable as a .vitaltable file.
     */
    void exportWavetable();

    /**
     * @brief Exports the current wavetable frames as a .wav file.
     */
    void exportToWav();

    /**
     * @brief Loads a wavetable file from disk.
     * @param wavetable_file The file to load.
     */
    void loadFile(const File& wavetable_file) override;

    /**
     * @brief Gets the current wavetable file being edited.
     * @return The current wavetable File.
     */
    File getCurrentFile() override { return File(wavetable_creator_->getLastFileLoaded()); }

    /**
     * @brief Loads a wavetable from JSON data.
     * @param wavetable_data The JSON object containing wavetable state.
     */
    void loadWavetable(json& wavetable_data);

    /**
     * @brief Gets the current wavetable state as JSON.
     * @return The JSON representation of the wavetable state.
     */
    json getWavetableJson() { return wavetable_creator_->stateToJson(); }

    /**
     * @brief Loads audio data as a wavetable.
     * @param name The name for the wavetable.
     * @param audio_stream InputStream to the audio file.
     * @param style The load style (e.g., splice, pitch-based).
     * @return True if loading successful, false otherwise.
     */
    bool loadAudioAsWavetable(String name, InputStream* audio_stream, WavetableCreator::AudioFileLoadStyle style);

    /**
     * @brief Resynthesizes the current preset into a wavetable.
     */
    void resynthesizeToWavetable();

    /**
     * @brief Handles button clicks for menu, settings, exit, etc.
     * @param clicked_button The clicked button.
     */
    virtual void buttonClicked(Button* clicked_button) override;

    /**
     * @brief Callback for when positions in the wavetable organizer are updated.
     */
    virtual void positionsUpdated() override;

    /**
     * @brief Callback when a frame is selected in the wavetable organizer.
     * @param keyframe The selected keyframe.
     */
    virtual void frameSelected(WavetableKeyframe* keyframe) override;

    /**
     * @brief Callback when a frame is dragged to a new position.
     * @param keyframe The dragged keyframe.
     * @param position The new frame position.
     */
    virtual void frameDragged(WavetableKeyframe* keyframe, int position) override;

    /**
     * @brief Callback for wheel movements in the component list area.
     * @param e The mouse event.
     * @param wheel The wheel details.
     */
    virtual void wheelMoved(const MouseEvent& e, const MouseWheelDetails& wheel) override;

    /**
     * @brief Renders the OpenGL components, including overlays.
     * @param open_gl Reference to the current OpenGlWrapper.
     * @param animate Whether to animate changes.
     */
    void renderOpenGlComponents(OpenGlWrapper& open_gl, bool animate) override;

    /**
     * @brief Sets whether the frequency display uses power scale or amplitude scale.
     * @param power_scale True for power scale, false for amplitude scale.
     */
    void setPowerScale(bool power_scale);

    /**
     * @brief Sets the zoom level for frequency domain rendering.
     * @param zoom The zoom factor (e.g., 1x, 2x).
     */
    void setZoom(int zoom);

    /**
     * @brief Clears the current wavetable editing state.
     */
    void clear();

    /**
     * @brief Initializes the wavetable state after loading or clearing.
     */
    void init();

    /**
     * @brief Gets the last browsed wavetable file.
     * @return The path to the last loaded wavetable file.
     */
    std::string getLastBrowsedWavetable() { return wavetable_creator_->getLastFileLoaded(); }

    /**
     * @brief Gets the current wavetable name.
     * @return The wavetable name.
     */
    std::string getName() { return wavetable_creator_->getName(); }

  private:
    void setPresetSelectorText();
    void showPopupMenu();
    void hideCurrentOverlay();
    void clearOverlays();
    void setColors();
    void render();
    void render(int position);
    void updateGlDisplay();
    void setOverlayPosition();
    void updateTimeDomain(float* time_domain);
    void updateFrequencyDomain(float* time_domain);
    int loadAudioFile(AudioSampleBuffer& destination, InputStream* audio_stream);

    int index_;
    float zoom_;
    bool power_scale_;
    bool obscure_time_domain_;
    bool obscure_freq_amplitude_;
    bool obscure_freq_phase_;

    AudioFormatManager format_manager_;

    std::unique_ptr<BarRenderer> frequency_amplitudes_;
    std::unique_ptr<BarRenderer> frequency_phases_;
    std::unique_ptr<WaveSourceEditor> oscillator_waveform_;
    std::unique_ptr<WavetableOrganizer> wavetable_organizer_;
    std::unique_ptr<WavetableComponentList> wavetable_component_list_;
    std::unique_ptr<WavetablePlayhead> wavetable_playhead_;
    std::unique_ptr<WavetablePlayheadInfo> wavetable_playhead_info_;
    std::unique_ptr<OpenGlShapeButton> exit_button_;
    std::unique_ptr<OpenGlShapeButton> frequency_amplitude_settings_;
    std::unique_ptr<PresetSelector> preset_selector_;
    std::unique_ptr<OpenGlShapeButton> menu_button_;

    Slider* wave_frame_slider_;

    vital::WaveFrame compute_frame_;
    WavetableCreator* wavetable_creator_;
    std::map<WavetableComponent*, WavetableComponentFactory::ComponentType> type_lookup_;
    std::unique_ptr<WavetableComponentOverlay> overlays_[WavetableComponentFactory::kNumComponentTypes];
    WavetableComponentOverlay* current_overlay_;
    Rectangle<int> edit_bounds_;
    Rectangle<int> title_bounds_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavetableEditSection)
};

