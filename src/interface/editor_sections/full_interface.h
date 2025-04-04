#pragma once

#include "JuceHeader.h"

#include "authentication.h"
#include "authentication_section.h"
#include "download_section.h"
#include "header_section.h"
#include "effects_interface.h"
#include "memory.h"
#include "modulation_matrix.h"
#include "open_gl_background.h"
#include "shaders.h"
#include "synth_section.h"
#include "update_check_section.h"
#include "wavetable_creator.h"

class AboutSection;
class BankExporter;
class BendSection;
class DeleteSection;
class ExpiredSection;
class ExtraModSection;
class HeaderSection;
class KeyboardInterface;
class MasterControlsInterface;
class ModulationInterface;
class ModulationManager;
class PortamentoSection;
class PresetBrowser;
class SaveSection;
class SynthesisInterface;
struct SynthGuiData;
class SynthSlider;
class WavetableEditSection;
class VoiceSection;

/**
 * @class FullInterface
 * @brief The main GUI container for the entire synthesizer interface.
 *
 * This class encapsulates all sections of the synthesizer's graphical user interface (GUI).
 * It handles layout, OpenGL rendering, event coordination between different GUI sections,
 * and interaction with backend data and states.
 *
 * The FullInterface organizes and manages child components such as the header, synthesis view,
 * effects, modulation matrix, master controls, keyboard interface, and various popup and
 * overlay sections. It also responds to authentication events, update checks, preset browsing,
 * and rendering updates.
 *
 * As the main container, FullInterface provides methods to:
 * - Attach and manage multiple subsections (e.g., effects, modulation, wavetable editing).
 * - Handle resizing and scaling of the interface.
 * - Interact with OpenGL for advanced rendering and real-time animations.
 * - Manage layout for different display scales and DPI settings.
 * - Display and hide overlay sections like preset browsers, authentication dialogs, and updates.
 */
class FullInterface : public SynthSection, public AuthenticationSection::Listener, public HeaderSection::Listener,
                      public DownloadSection::Listener, public UpdateCheckSection::Listener,
                      public EffectsInterface::Listener, public ModulationMatrix::Listener,
                      public OpenGLRenderer, DragAndDropContainer {
  public:
    /**
     * @brief Minimum required OpenGL version for the interface to function properly.
     */
    static constexpr double kMinOpenGlVersion = 1.4;

    /**
     * @brief Constructs the full interface with the given synth GUI data.
     * @param synth_gui_data A pointer to a SynthGuiData structure that provides access to the synthesizer's data and state.
     */
    FullInterface(SynthGuiData* synth_gui_data);

    /**
     * @brief Constructs a minimal, empty full interface.
     *
     * Primarily used in scenarios where the full interface cannot be fully initialized.
     */
    FullInterface();

    /**
     * @brief Destructor.
     */
    virtual ~FullInterface();

    /**
     * @brief Assigns memory for oscilloscope visualization.
     * @param memory A pointer to poly_float memory holding oscilloscope data.
     */
    void setOscilloscopeMemory(const vital::poly_float* memory);

    /**
     * @brief Assigns stereo audio memory for visualization.
     * @param memory A pointer to StereoMemory holding audio samples for display.
     */
    void setAudioMemory(const vital::StereoMemory* memory);

    /**
     * @brief Creates modulation sliders for both mono and poly modulations.
     * @param mono_modulations A map of mono modulation outputs.
     * @param poly_modulations A map of poly modulation outputs.
     */
    void createModulationSliders(const vital::output_map& mono_modulations,
                                 const vital::output_map& poly_modulations);

    /**
     * @brief Paints the background of the entire interface.
     * @param g The graphics context used for drawing.
     */
    virtual void paintBackground(Graphics& g) override;

    /**
     * @brief Copies skin values from a Skin instance into the interface and look-and-feel.
     * @param skin The skin object containing color and style values.
     */
    void copySkinValues(const Skin& skin);

    /**
     * @brief Reloads and applies a new skin, then adjusts layout accordingly.
     * @param skin The skin object containing color and style values.
     */
    void reloadSkin(const Skin& skin);

    /**
     * @brief Repaints the background of a child section.
     * @param child The child SynthSection whose background needs repainting.
     */
    void repaintChildBackground(SynthSection* child);

    /**
     * @brief Repaints only the synthesis (oscillator/filter) section background.
     */
    void repaintSynthesisSection();

    /**
     * @brief Repaints the background that shows behind OpenGL components.
     * @param component The OpenGlComponent to repaint the background for.
     */
    void repaintOpenGlBackground(OpenGlComponent* component);

    /**
     * @brief Redraws the entire background image for the interface.
     *
     * Useful when skin changes or layout adjustments occur.
     */
    void redoBackground();

    /**
     * @brief Checks if the interface should be repositioned or resized based on scaling or display changes.
     * @param resize If true, forces a resize to adjust layout.
     */
    void checkShouldReposition(bool resize = true);

    /**
     * @brief Called when this component is added to a new parent.
     *
     * Performs a check to ensure correct positioning and scaling.
     */
    void parentHierarchyChanged() override {
      SynthSection::parentHierarchyChanged();
      checkShouldReposition();
    }

    /**
     * @brief Adjusts layout and sizes of child components after the interface is resized.
     */
    virtual void resized() override;

    /**
     * @brief Enables or disables animations (like continuous OpenGL repainting).
     * @param animate True to enable animations, false to disable.
     */
    void animate(bool animate) override;

    /**
     * @brief Resets the entire interface to a default state.
     */
    void reset() override;

    /**
     * @brief Sets all parameter values on the interface.
     * @param controls A map of control names to their associated parameters and values.
     */
    void setAllValues(vital::control_map& controls) override;

    /**
     * @brief Called when the data directory changes.
     *
     * Refreshes preset listings or other assets that depend on the data directory.
     */
    void dataDirectoryChanged() override;

    /**
     * @brief Indicates that no download is needed after checking updates.
     */
    void noDownloadNeeded() override;

    /**
     * @brief Indicates that a software update is available and needs action.
     */
    void needsUpdate() override;

    /**
     * @brief Called when the user has successfully logged in.
     */
    void loggedIn() override;

    /**
     * @brief Updates displayed wavetable names from the current wavetable data.
     */
    void setWavetableNames();

    /**
     * @brief Starts a download of additional content if needed.
     */
    void startDownload();

    /**
     * @brief Called when a new OpenGL context is created.
     */
    void newOpenGLContextCreated() override;

    /**
     * @brief Renders the GUI using OpenGL.
     */
    void renderOpenGL() override;

    /**
     * @brief Called when the OpenGL context is closing, allowing cleanup of resources.
     */
    void openGLContextClosing() override;

    /**
     * @brief Shows the "About" section overlay.
     */
    void showAboutSection() override;

    /**
     * @brief Handles a request to delete a preset file.
     * @param preset The file to be deleted.
     */
    void deleteRequested(File preset) override;

    /**
     * @brief Handles user selecting a main tab (e.g., Synthesis, Effects, etc.)
     * @param index The index of the selected tab.
     */
    void tabSelected(int index) override;

    /**
     * @brief Clears temporary overlay tabs such as the preset browser.
     * @param current_tab The currently selected main tab index.
     */
    void clearTemporaryTab(int current_tab) override;

    /**
     * @brief Toggles the visibility of the preset browser.
     * @param visible True to show, false to hide.
     * @param current_tab The currently selected main tab index.
     */
    void setPresetBrowserVisibility(bool visible, int current_tab) override;

    /**
     * @brief Toggles the visibility of the bank exporter.
     * @param visible True to show, false to hide.
     * @param current_tab The currently selected main tab index.
     */
    void setBankExporterVisibility(bool visible, int current_tab) override;

    /**
     * @brief Called when a new bank of presets has been imported, to refresh lists.
     */
    void bankImported() override;

    /**
     * @brief Called when the effects interface is moved, to update meter positions.
     */
    void effectsMoved() override;

    /**
     * @brief Called when modulations are scrolled, to update meter positions.
     */
    void modulationsScrolled() override;

    /**
     * @brief Sets keyboard focus to a relevant component (e.g., authentication).
     */
    void setFocus();

    /**
     * @brief Notifies that a parameter or state has changed.
     */
    void notifyChange();

    /**
     * @brief Notifies that the interface is in a fresh state (no unsaved changes).
     */
    void notifyFresh();

    /**
     * @brief Handles loading an external preset file into the interface.
     * @param preset The preset File being loaded.
     */
    void externalPresetLoaded(const File& preset);

    /**
     * @brief Displays a full-screen section overlay, hiding other sections.
     * @param full_screen Pointer to the SynthSection to show full-screen, or nullptr to restore normal view.
     */
    void showFullScreenSection(SynthSection* full_screen);

    /**
     * @brief Shows the wavetable edit section for a given oscillator index.
     * @param index The oscillator index, or -1 to hide all wavetable editors.
     */
    void showWavetableEditSection(int index);

    /**
     * @brief Gets the last browsed wavetable file path for a given oscillator.
     * @param index The oscillator index.
     * @return A string containing the last browsed wavetable path.
     */
    std::string getLastBrowsedWavetable(int index);

    /**
     * @brief Gets the current wavetable name for a given oscillator.
     * @param index The oscillator index.
     * @return A string containing the wavetable name.
     */
    std::string getWavetableName(int index);

    /**
     * @brief Gets the name of the currently signed-in user, if any.
     * @return A string with the signed-in user's name, or empty if not signed in.
     */
    std::string getSignedInName();

    /**
     * @brief Signs out the current user, if signed in.
     */
    void signOut();

    /**
     * @brief Opens the sign-in interface, if available.
     */
    void signIn();

    /**
     * @brief Hides any open wavetable editing section, returning to the normal view.
     */
    void hideWavetableEditSection();

    /**
     * @brief Loads a wavetable from a file into the given oscillator editor.
     * @param index The oscillator index.
     * @param wavetable The File containing the wavetable data.
     */
    void loadWavetableFile(int index, const File& wavetable);

    /**
     * @brief Loads a wavetable from a JSON object into the given oscillator editor.
     * @param index The oscillator index.
     * @param wavetable_data The JSON data representing the wavetable.
     */
    void loadWavetable(int index, json& wavetable_data);

    /**
     * @brief Loads a default wavetable into the given oscillator editor.
     * @param index The oscillator index.
     */
    void loadDefaultWavetable(int index);

    /**
     * @brief Performs resynthesis of the current wavetable for the given oscillator.
     * @param index The oscillator index.
     */
    void resynthesizeToWavetable(int index);

    /**
     * @brief Opens a save dialog to save the current wavetable of an oscillator.
     * @param index The oscillator index.
     */
    void saveWavetable(int index);

    /**
     * @brief Opens a save dialog to save a given LFO shape.
     * @param data The JSON data representing the LFO.
     */
    void saveLfo(const json& data);

    /**
     * @brief Retrieves the JSON data representing the current wavetable for an oscillator.
     * @param index The oscillator index.
     * @return The JSON object representing the wavetable.
     */
    json getWavetableJson(int index);

    /**
     * @brief Loads an audio file as a wavetable into the given oscillator editor.
     * @param index The oscillator index.
     * @param name The name of the wavetable.
     * @param audio_stream A pointer to an InputStream representing the audio file.
     * @param style The loading style (e.g., how to map audio data to wavetable frames).
     * @return True if successful, false otherwise.
     */
    bool loadAudioAsWavetable(int index, const String& name, InputStream* audio_stream,
                              WavetableCreator::AudioFileLoadStyle style);

    /**
     * @brief Opens a popup browser for file browsing.
     * @param owner The SynthSection that owns the browser.
     * @param bounds The bounds relative to the owner where the popup should appear.
     * @param directories A vector of directories to browse.
     * @param extensions The file extensions to filter by.
     * @param passthrough_name A name for passthrough directories.
     * @param additional_folders_name A name for additional folder groups.
     */
    void popupBrowser(SynthSection* owner, Rectangle<int> bounds, std::vector<File> directories,
                      String extensions, std::string passthrough_name, std::string additional_folders_name);

    /**
     * @brief Updates the popup browser's ownership, useful if the owner changed.
     * @param owner The new owner SynthSection.
     */
    void popupBrowserUpdate(SynthSection* owner);

    /**
     * @brief Opens a popup selector for choosing from a list of items.
     * @param source The component from which the popup position is derived.
     * @param position The position (relative to source) to place the popup.
     * @param options The PopupItems to display as choices.
     * @param callback Function called with the selected item index upon selection.
     * @param cancel Function called if the user cancels or closes the selector without choosing.
     */
    void popupSelector(Component* source, Point<int> position, const PopupItems& options,
                       std::function<void(int)> callback, std::function<void()> cancel);

    /**
     * @brief Opens a dual popup selector for choosing from a hierarchical list of items.
     * @param source The component from which the popup position is derived.
     * @param position The position (relative to source) to place the popup.
     * @param width The width of the popup.
     * @param options The PopupItems to display.
     * @param callback Function called with the selected item index upon selection.
     */
    void dualPopupSelector(Component* source, Point<int> position, int width,
                           const PopupItems& options, std::function<void(int)> callback);

    /**
     * @brief Shows a popup display (a tooltip-like bubble) with provided text.
     * @param source The component that the popup should point to.
     * @param text The text to display inside the popup.
     * @param placement The placement of the bubble relative to the component.
     * @param primary True if this is the primary popup, false if secondary.
     */
    void popupDisplay(Component* source, const std::string& text,
                      BubbleComponent::BubblePlacement placement, bool primary);

    /**
     * @brief Hides a previously shown popup display.
     * @param primary True if hiding the primary popup, false if the secondary popup.
     */
    void hideDisplay(bool primary);

    /**
     * @brief Called when modulations have changed, updating the modulation matrix and related components.
     */
    void modulationChanged();

    /**
     * @brief Called when a particular modulation value changes.
     * @param index The index of the changed modulation.
     */
    void modulationValueChanged(int index);

    void openSaveDialog() { save_section_->setIsPreset(true); save_section_->setVisible(true); }

    /**
     * @brief Enables or disables redrawing of the background when resized.
     * @param enable True to enable, false to disable.
     */
    void enableRedoBackground(bool enable) {
      enable_redo_background_ = enable;
      if (enable)
        resized();
    }

    /**
     * @brief Returns the scale factor for resizing operations.
     * @return The resizing scale factor.
     */
    float getResizingScale() const { return width_ * 1.0f / resized_width_; }

    /**
     * @brief Returns the current pixel scaling factor based on display scale.
     * @return The pixel scaling factor.
     */
    float getPixelScaling() const override { return display_scale_; }

    /**
     * @brief Returns the pixel multiple used for scaling pixel-perfect rendering.
     * @return The pixel multiple.
     */
    int getPixelMultiple() const override { return pixel_multiple_; }
    void toggleOscillatorZoom(int index);
    void toggleFilter1Zoom();
    void toggleFilter2Zoom();

  private:
    /**
     * @brief Checks if all wavetable editors are initialized.
     * @return True if all editors are initialized, false otherwise.
     */
    bool wavetableEditorsInitialized() {
      for (int i = 0; i < vital::kNumOscillators; ++i) {
        if (wavetable_edits_[i] == nullptr)
          return false;
      }
      return true;
    }

    Authentication auth_;
    std::map<std::string, SynthSlider*> slider_lookup_;
    std::map<std::string, Button*> button_lookup_;
    std::unique_ptr<ModulationManager> modulation_manager_;
    std::unique_ptr<ModulationMatrix> modulation_matrix_;

    std::unique_ptr<AboutSection> about_section_;
    std::unique_ptr<AuthenticationSection> authentication_;
    std::unique_ptr<UpdateCheckSection> update_check_section_;
    std::unique_ptr<Component> standalone_settings_section_;

    std::unique_ptr<HeaderSection> header_;
    std::unique_ptr<SynthesisInterface> synthesis_interface_;
    std::unique_ptr<MasterControlsInterface> master_controls_interface_;
    std::unique_ptr<ModulationInterface> modulation_interface_;
    std::unique_ptr<ExtraModSection> extra_mod_section_;
    std::unique_ptr<EffectsInterface> effects_interface_;
    std::unique_ptr<WavetableEditSection> wavetable_edits_[vital::kNumOscillators];
    std::unique_ptr<KeyboardInterface> keyboard_interface_;
    std::unique_ptr<BendSection> bend_section_;
    std::unique_ptr<PortamentoSection> portamento_section_;
    std::unique_ptr<VoiceSection> voice_section_;
    std::unique_ptr<PresetBrowser> preset_browser_;
    std::unique_ptr<PopupBrowser> popup_browser_;
    std::unique_ptr<SinglePopupSelector> popup_selector_;
    std::unique_ptr<DualPopupSelector> dual_popup_selector_;
    std::unique_ptr<PopupDisplay> popup_display_1_;
    std::unique_ptr<PopupDisplay> popup_display_2_;
    std::unique_ptr<BankExporter> bank_exporter_;
    std::unique_ptr<SaveSection> save_section_;
    std::unique_ptr<DeleteSection> delete_section_;
    std::unique_ptr<DownloadSection> download_section_;
    std::unique_ptr<ExpiredSection> expired_section_;
    SynthSection* full_screen_section_;

    int width_;
    int resized_width_;
    float last_render_scale_;
    float display_scale_;
    int pixel_multiple_;
    bool setting_all_values_;
    bool unsupported_;
    bool animate_;
    bool enable_redo_background_;
    bool needs_download_;
    CriticalSection open_gl_critical_section_;
    OpenGLContext open_gl_context_;
    std::unique_ptr<Shaders> shaders_;
    OpenGlWrapper open_gl_;
    Image background_image_;
    OpenGlBackground background_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FullInterface)
};
