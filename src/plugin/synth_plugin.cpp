/*
Summary:

SynthPlugin is the core plugin class handling parameter management, saving/loading state, preparing audio processing, and communicating with the host. It integrates the synth engine, GUI interface, and handles parameter automation through ValueBridge instances. It also ensures that parameter changes and preset loading are communicated effectively to the GUI and host.
 */

#include "synth_plugin.h"
#include "synth_editor.h"
#include "sound_engine.h"
#include "load_save.h"

SynthPlugin::SynthPlugin() {
  last_seconds_time_ = 0.0;

  // Register and create ValueBridges for all parameters.
  int num_params = vital::Parameters::getNumParameters();
  for (int i = 0; i < num_params; ++i) {
    const vital::ValueDetails* details = vital::Parameters::getDetails(i);
    if (controls_.count(details->name) == 0)
      continue;

    // Create a ValueBridge for each known parameter.
    ValueBridge* bridge = new ValueBridge(details->name, controls_[details->name]);
    bridge->setListener(this);
    bridge_lookup_[details->name] = bridge;
    addParameter(bridge);
  }

  bypass_parameter_ = bridge_lookup_["bypass"];
}

SynthPlugin::~SynthPlugin() {
  midi_manager_ = nullptr;
  keyboard_state_ = nullptr;
}

SynthGuiInterface* SynthPlugin::getGuiInterface() {
  // Return the active GUI interface if available.
  AudioProcessorEditor* editor = getActiveEditor();
  if (editor)
    return dynamic_cast<SynthGuiInterface*>(editor);
  return nullptr;
}

void SynthPlugin::beginChangeGesture(const std::string& name) {
  // Begin host automation gesture for the parameter if available.
  if (bridge_lookup_.count(name))
    bridge_lookup_[name]->beginChangeGesture();
}

void SynthPlugin::endChangeGesture(const std::string& name) {
  // End host automation gesture for the parameter if available.
  if (bridge_lookup_.count(name))
    bridge_lookup_[name]->endChangeGesture();
}

void SynthPlugin::setValueNotifyHost(const std::string& name, vital::mono_float value) {
  // Sets parameter value and notifies host if found.
  if (bridge_lookup_.count(name)) {
    vital::mono_float plugin_value = bridge_lookup_[name]->convertToPluginValue(value);
    bridge_lookup_[name]->setValueNotifyHost(plugin_value);
  }
}

const CriticalSection& SynthPlugin::getCriticalSection() {
  // Return the callback lock for thread safety.
  return getCallbackLock();
}

void SynthPlugin::pauseProcessing(bool pause) {
  // Suspend or resume processing.
  suspendProcessing(pause);
}

const String SynthPlugin::getName() const {
  return JucePlugin_Name;
}

const String SynthPlugin::getInputChannelName(int channel_index) const {
  return String(channel_index + 1);
}

const String SynthPlugin::getOutputChannelName(int channel_index) const {
  return String(channel_index + 1);
}

bool SynthPlugin::isInputChannelStereoPair(int index) const {
  return true;
}

bool SynthPlugin::isOutputChannelStereoPair(int index) const {
  return true;
}

bool SynthPlugin::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool SynthPlugin::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool SynthPlugin::silenceInProducesSilenceOut() const {
  return false;
}

double SynthPlugin::getTailLengthSeconds() const {
  return 0.0;
}

const String SynthPlugin::getProgramName(int index) {
  // Return current preset name if GUI and synth are available.
  SynthGuiInterface* editor = getGuiInterface();
  if (editor == nullptr || editor->getSynth() == nullptr)
    return "";

  return editor->getSynth()->getPresetName();
}

void SynthPlugin::prepareToPlay(double sample_rate, int buffer_size) {
  // Prepare the synth engine and managers for playback.
  engine_->setSampleRate(sample_rate);
  engine_->updateAllModulationSwitches();
  midi_manager_->setSampleRate(sample_rate);
}

void SynthPlugin::releaseResources() {
  // Free resources if needed (not implemented here).
}

void SynthPlugin::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midi_messages) {
  static constexpr double kSecondsPerMinute = 60.0f;

  // Check if bypassed
  if (bypass_parameter_->getValue()) {
    processBlockBypassed(buffer, midi_messages);
    return;
  }

  int total_samples = buffer.getNumSamples();
  int num_channels = getTotalNumOutputChannels();
  AudioPlayHead* play_head = getPlayHead();
  if (play_head) {
    play_head->getCurrentPosition(position_info_);
    if (position_info_.bpm)
      engine_->setBpm(position_info_.bpm);

    if (position_info_.isPlaying) {
      double bps = position_info_.bpm / kSecondsPerMinute;
      last_seconds_time_ = position_info_.ppqPosition / bps;
    }
  }

  processModulationChanges();
  if (total_samples)
    processKeyboardEvents(midi_messages, total_samples);

  double sample_time = 1.0 / AudioProcessor::getSampleRate();
  for (int sample_offset = 0; sample_offset < total_samples;) {
    int num_samples = std::min<int>(total_samples - sample_offset, vital::kMaxBufferSize);

    engine_->correctToTime(last_seconds_time_);
    processMidi(midi_messages, sample_offset, sample_offset + num_samples);
    processAudio(&buffer, num_channels, num_samples, sample_offset);

    last_seconds_time_ += num_samples * sample_time;
    sample_offset += num_samples;
  }
}

bool SynthPlugin::hasEditor() const {
  return true;
}

AudioProcessorEditor* SynthPlugin::createEditor() {
  // Create the main GUI editor.
  return new SynthEditor(*this);
}

void SynthPlugin::parameterChanged(std::string name, vital::mono_float value) {
  // Parameter changed from an external source, update synth parameters.
  valueChangedExternal(name, value);
}

void SynthPlugin::getStateInformation(MemoryBlock& dest_data) {
  // Save current state to JSON and then to the memory block.
  json data = LoadSave::stateToJson(this, getCallbackLock());
  data["tuning"] = getTuning()->stateToJson();

  String data_string = data.dump();
  MemoryOutputStream stream;
  stream.writeString(data_string);
  dest_data.append(stream.getData(), stream.getDataSize());
}

void SynthPlugin::setStateInformation(const void* data, int size_in_bytes) {
  // Restore plugin state from JSON data.
  MemoryInputStream stream(data, size_in_bytes, false);
  String data_string = stream.readEntireStreamAsString();

  pauseProcessing(true);
  try {
    json json_data = json::parse(data_string.toStdString());
    LoadSave::jsonToState(this, save_info_, json_data);

    if (json_data.count("tuning"))
      getTuning()->jsonToState(json_data["tuning"]);
  }
  catch (const json::exception& e) {
    std::string error = "There was an error open the preset. Preset file is corrupted.";
    AlertWindow::showNativeDialogBox("Error opening preset", error, false);
  }
  pauseProcessing(false);

  SynthGuiInterface* editor = getGuiInterface();
  if (editor)
    editor->updateFullGui();
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new SynthPlugin();
}
