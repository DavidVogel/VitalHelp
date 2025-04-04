#include "synth_editor.h"

#include "authentication.h"
#include "default_look_and_feel.h"
#include "full_interface.h"
#include "synth_computer_keyboard.h"
#include "synth_constants.h"
#include "sound_engine.h"
#include "load_save.h"
#include "startup.h"
#include "utils.h"

/**
 * @brief Constructs a SynthEditor object.
 *
 * Initializes the audio device manager, sets the default sample rate,
 * creates the optional GUI, and starts a timer to poll for MIDI devices.
 *
 * @param use_gui Determines whether the GUI is created and displayed.
 */
SynthEditor::SynthEditor(bool use_gui) : SynthGuiInterface(this, use_gui) {
  static constexpr int kHeightBuffer = 50;

  // Create computer keyboard for MIDI note input
  computer_keyboard_ = std::make_unique<SynthComputerKeyboard>(engine_.get(), keyboard_state_.get());
  current_time_ = 0.0;

  // Initialize audio channels (input, output)
  setAudioChannels(0, vital::kNumChannels);

  // Configure AudioDeviceManager with default sample rate
  AudioDeviceManager::AudioDeviceSetup setup;
  deviceManager.getAudioDeviceSetup(setup);
  setup.sampleRate = vital::kDefaultSampleRate;
  deviceManager.initialise(0, vital::kNumChannels, nullptr, true, "", &setup);

  // If no device is active, choose one from the available device types
  if (deviceManager.getCurrentAudioDevice() == nullptr) {
    const OwnedArray<AudioIODeviceType>& device_types = deviceManager.getAvailableDeviceTypes();

    for (AudioIODeviceType* device_type : device_types) {
      deviceManager.setCurrentAudioDeviceType(device_type->getTypeName(), true);
      if (deviceManager.getCurrentAudioDevice())
        break;
    }
  }

  // Enable all available MIDI inputs.
  current_midi_ins_ = StringArray(MidiInput::getDevices());

  for (const String& midi_in : current_midi_ins_)
    deviceManager.setMidiInputEnabled(midi_in, true);

  // Register MIDI manager as a callback for MIDI events
  deviceManager.addMidiInputCallback("", midi_manager_.get());

  // If GUI is used, configure and display it
  if (use_gui) {
    setLookAndFeel(DefaultLookAndFeel::instance());
    addAndMakeVisible(gui_.get());
    gui_->reset();
    gui_->setOscilloscopeMemory(getOscilloscopeMemory());
    gui_->setAudioMemory(getAudioMemory());

    // Get screen size and apply saved window size constraints
    Rectangle<int> total_bounds = Desktop::getInstance().getDisplays().getTotalBounds(true);
    total_bounds.removeFromBottom(kHeightBuffer);

    float window_size = LoadSave::loadWindowSize();
    window_size = std::min(window_size, total_bounds.getWidth() / (1.0f * vital::kDefaultWindowWidth));
    window_size = std::min(window_size, total_bounds.getHeight() / (1.0f * vital::kDefaultWindowHeight));
    int width = std::round(window_size * vital::kDefaultWindowWidth);
    int height = std::round(window_size * vital::kDefaultWindowHeight);
    setSize(width, height);

    setWantsKeyboardFocus(true);
    addKeyListener(computer_keyboard_.get());
    setOpaque(true);
  }

  // Start a timer to check for new MIDI devices every 500 ms
  startTimer(500);
}

/**
 * @brief Destructor for SynthEditor.
 *
 * Cleans up the audio system and dismisses any active popup menus.
 */
SynthEditor::~SynthEditor() {
  PopupMenu::dismissAllActiveMenus();
  shutdownAudio();
}

/**
 * @brief Prepares the audio engine for playback.
 *
 * Sets the sample rate for the sound engine and the MIDI manager, and
 * updates any modulation switches in the engine.
 *
 * @param buffer_size The size of the audio buffer (in samples).
 * @param sample_rate The sampling rate (in Hz).
 */
void SynthEditor::prepareToPlay(int buffer_size, double sample_rate) {
  engine_->setSampleRate(sample_rate);
  engine_->updateAllModulationSwitches();
  midi_manager_->setSampleRate(sample_rate);
}

/**
 * @brief Called by the audio device to provide next audio block.
 *
 * Processes pending MIDI events, updates the synthesizer engine,
 * and renders audio data into the provided buffer.
 *
 * @param buffer Reference to the AudioSourceChannelInfo containing the buffer.
 */
void SynthEditor::getNextAudioBlock(const AudioSourceChannelInfo& buffer) {
  ScopedLock lock(getCriticalSection());

  int num_samples = buffer.buffer->getNumSamples();
  int synth_samples = std::min(num_samples, vital::kMaxBufferSize);

  // Process any modulation changes that may have occurred.
  processModulationChanges();

  // Retrieve and process MIDI messages.
  MidiBuffer midi_messages;
  midi_manager_->removeNextBlockOfMessages(midi_messages, num_samples);
  processKeyboardEvents(midi_messages, num_samples);

  double sample_time = 1.0 / getSampleRate();
  for (int b = 0; b < num_samples; b += synth_samples) {
    int current_samples = std::min<int>(synth_samples, num_samples - b);
    engine_->correctToTime(current_time_);

    processMidi(midi_messages, b, b + current_samples);
    processAudio(buffer.buffer, vital::kNumChannels, current_samples, b);
    current_time_ += current_samples * sample_time;
  }
}

/**
 * @brief Releases any resources allocated for audio playback.
 *
 * Currently empty, but can be used to cleanup resources if needed.
 */
void SynthEditor::releaseResources() {
}

/**
 * @brief Called when the component is resized.
 *
 * Ensures the GUI interface is resized to fill the entire window.
 */
void SynthEditor::resized() {
  if (gui_)
    gui_->setBounds(getBounds());
}

/**
 * @brief Periodically checks for new MIDI devices and enables them.
 *
 * This callback runs at intervals set by startTimer().
 */
void SynthEditor::timerCallback() {
  // Periodically check for new MIDI inputs and enable them.
  StringArray midi_ins(MidiInput::getDevices());

  // Enable newly-discovered MIDI inputs
  for (int i = 0; i < midi_ins.size(); ++i) {
    if (!current_midi_ins_.contains(midi_ins[i]))
      deviceManager.setMidiInputEnabled(midi_ins[i], true);
  }

  current_midi_ins_ = midi_ins;
}

/**
 * @brief Toggles GUI animation such as oscilloscopes or VU meters.
 *
 * @param animate If true, animations are turned on; otherwise, off.
 */
void SynthEditor::animate(bool animate) {
  if (gui_)
    gui_->animate(animate);
}
