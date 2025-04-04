/*
Summary:
The SynthGuiInterface class mediates between the SynthBase (synth engine backend) and a graphical front end (FullInterface). It updates GUI elements when parameters or modulations change, and also handles user actions from the GUI such as connecting modulations or resizing the window. In HEADLESS mode, these functionalities become no-ops. The SynthGuiData struct provides a data snapshot of engine state (controls, modulations, wavetable creators) to the GUI for display and interaction.
 */
#pragma once

#include "JuceHeader.h"
#include "synth_base.h"

#if HEADLESS

class FullInterface { };
class AudioDeviceManager { };

#endif

class FullInterface;
class Authentication;

/**
 * @brief A struct holding references and data used by the GUI to interact with the SynthBase.
 *
 * SynthGuiData captures a snapshot of various control maps (parameters, modulations) from
 * the SynthBase and provides them to the GUI elements. It also links to the WavetableCreators
 * for each oscillator, enabling the GUI to display and manipulate wavetable data.
 */
struct SynthGuiData {
    /**
     * @brief Constructs SynthGuiData from a given SynthBase instance.
     *
     * @param synth_base A pointer to the SynthBase whose data is to be captured.
     */
    SynthGuiData(SynthBase* synth_base);

    vital::control_map controls;                   ///< Current set of parameter controls from the synth.
    vital::output_map mono_modulations;            ///< Mono (global) modulation values.
    vital::output_map poly_modulations;            ///< Polyphonic modulation values.
    vital::output_map modulation_sources;          ///< All available modulation sources.
    WavetableCreator* wavetable_creators[vital::kNumOscillators]; ///< Array of pointers to wavetable creators for each oscillator.
    SynthBase* synth;                              ///< Pointer back to the associated SynthBase.
};

/**
 * @brief An interface class linking the Vital synthesizer backend (SynthBase) with a GUI.
 *
 * SynthGuiInterface provides methods to:
 * - Update GUI controls based on internal parameter changes.
 * - Notify the GUI of changes to modulations and parameters.
 * - Connect and disconnect modulations through GUI interactions.
 * - Load and save presets, including invoking dialog boxes if applicable.
 *
 * On some builds (HEADLESS mode), these functionalities may be stubbed out,
 * meaning the GUI does nothing. When GUI is available, it typically integrates with
 * a FullInterface GUI component that displays and updates Vital’s parameters and states.
 */
class SynthGuiInterface {
public:
    /**
     * @brief Constructs the SynthGuiInterface, optionally creating a FullInterface GUI.
     *
     * @param synth A pointer to the SynthBase instance to manage GUI for.
     * @param use_gui If true, constructs a GUI interface. If false (or HEADLESS), no GUI is created.
     */
    SynthGuiInterface(SynthBase* synth, bool use_gui = true);

    /**
     * @brief Destroys the SynthGuiInterface, cleaning up any associated GUI resources.
     */
    virtual ~SynthGuiInterface();

    /**
     * @brief Retrieves the audio device manager if available.
     *
     * By default, this returns nullptr. Subclasses or custom builds might return a valid manager.
     *
     * @return A pointer to the AudioDeviceManager or nullptr if not implemented.
     */
    virtual AudioDeviceManager* getAudioDeviceManager() { return nullptr; }

    /**
     * @brief Returns the SynthBase instance this interface is managing.
     *
     * @return A pointer to the managed SynthBase.
     */
    SynthBase* getSynth() { return synth_; }

    /**
     * @brief Updates the entire GUI to reflect the current synth state.
     *
     * This method is typically called after loading a new preset or making significant changes
     * to parameters. It instructs the GUI to refresh all displayed values.
     */
    virtual void updateFullGui();

    /**
     * @brief Updates a single GUI control to reflect a new parameter value.
     *
     * @param name The parameter/control name.
     * @param value The new value to display on the GUI.
     */
    virtual void updateGuiControl(const std::string& name, vital::mono_float value);

    /**
     * @brief Retrieves the current value of a named control from the synth.
     *
     * @param name The parameter/control name.
     * @return The current control value.
     */
    vital::mono_float getControlValue(const std::string& name);

    /**
     * @brief Notifies the GUI that modulation connections or states have changed.
     *
     * This prompts the GUI to refresh displays related to modulation routing or amounts.
     */
    void notifyModulationsChanged();

    /**
     * @brief Notifies the GUI that a specific modulation's value changed.
     *
     * @param index The modulation connection index that changed.
     */
    void notifyModulationValueChanged(int index);

    /**
     * @brief Connects a modulation source to a destination parameter through the GUI.
     *
     * If a new modulation connection is created, the GUI may initialize default modulation values.
     *
     * @param source The modulation source name.
     * @param destination The destination parameter name.
     */
    void connectModulation(std::string source, std::string destination);

    /**
     * @brief Connects a modulation using a pre-existing ModulationConnection object.
     *
     * @param connection Pointer to the ModulationConnection to connect.
     */
    void connectModulation(vital::ModulationConnection* connection);

    /**
     * @brief Sets various modulation parameters (amount, bipolar, stereo, bypass) for a given connection.
     *
     * This function updates both the backend and GUI to reflect the new modulation settings.
     *
     * @param source The modulation source name.
     * @param destination The parameter to modulate.
     * @param amount The modulation amount.
     * @param bipolar True for bipolar modulation, false for unipolar.
     * @param stereo True if modulation is stereo, false if mono.
     * @param bypass True if modulation is bypassed, false otherwise.
     */
    void setModulationValues(const std::string& source, const std::string& destination,
                             vital::mono_float amount, bool bipolar, bool stereo, bool bypass);

    /**
     * @brief Initializes modulation values for a newly created modulation connection.
     *
     * Resets line mappings and other parameters to default linear states.
     *
     * @param source The modulation source name.
     * @param destination The parameter to modulate.
     */
    void initModulationValues(const std::string& source, const std::string& destination);

    /**
     * @brief Disconnects a modulation from the GUI layer.
     *
     * @param source The modulation source name.
     * @param destination The destination parameter name.
     */
    void disconnectModulation(std::string source, std::string destination);

    /**
     * @brief Disconnects a modulation using a ModulationConnection object.
     *
     * @param connection Pointer to the ModulationConnection to disconnect.
     */
    void disconnectModulation(vital::ModulationConnection* connection);

    /**
     * @brief Brings the GUI window or main component into focus.
     */
    void setFocus();

    /**
     * @brief Notifies the GUI that a parameter or modulation changed, prompting GUI updates.
     */
    void notifyChange();

    /**
     * @brief Notifies the GUI that a fresh state (like a new preset load) has occurred, prompting a full refresh.
     */
    void notifyFresh();

    /**
     * @brief Opens a save dialog (e.g., to save a preset) through the GUI.
     */
    void openSaveDialog();

    /**
     * @brief Notifies the GUI that a preset was loaded externally (outside the GUI controls).
     *
     * @param preset The preset file that was loaded.
     */
    void externalPresetLoaded(File preset);

    /**
     * @brief Sets the GUI window or component size based on a scale factor.
     *
     * Adjusts the main interface dimensions relative to a default size and current display settings.
     *
     * @param scale A scaling factor for the GUI size.
     */
    void setGuiSize(float scale);

    /**
     * @brief Gets the FullInterface GUI component if it exists.
     *
     * @return A pointer to the FullInterface object or nullptr if no GUI is in use.
     */
    FullInterface* getGui() { return gui_.get(); }

protected:
    SynthBase* synth_;                       ///< The backend SynthBase this GUI interface controls.
    std::unique_ptr<FullInterface> gui_;     ///< The primary GUI component (if applicable).

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthGuiInterface)
};
