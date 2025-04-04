/*
Summary:
The SynthGuiInterface class mediates between the SynthBase (synth engine backend) and a graphical front end (FullInterface). It updates GUI elements when parameters or modulations change, and also handles user actions from the GUI such as connecting modulations or resizing the window. In HEADLESS mode, these functionalities become no-ops. The SynthGuiData struct provides a data snapshot of engine state (controls, modulations, wavetable creators) to the GUI for display and interaction.
 */


#include "synth_gui_interface.h"
#include "authentication.h"
#include "modulation_connection_processor.h"
#include "sound_engine.h"
#include "load_save.h"
#include "synth_base.h"

SynthGuiData::SynthGuiData(SynthBase* synth_base) : synth(synth_base) {
  // Capture current control and modulation maps, as well as wavetable creators.
  controls = synth->getControls();
  mono_modulations = synth->getEngine()->getMonoModulations();
  poly_modulations = synth->getEngine()->getPolyModulations();
  modulation_sources = synth->getEngine()->getModulationSources();
  for (int i = 0; i < vital::kNumOscillators; ++i)
    wavetable_creators[i] = synth->getWavetableCreator(i);
}

#if HEADLESS
// In HEADLESS mode, these methods do nothing.
SynthGuiInterface::SynthGuiInterface(SynthBase* synth, bool use_gui) : synth_(synth) { }
SynthGuiInterface::~SynthGuiInterface() { }
void SynthGuiInterface::updateFullGui() { }
void SynthGuiInterface::updateGuiControl(const std::string& name, vital::mono_float value) { }
vital::mono_float SynthGuiInterface::getControlValue(const std::string& name) { return 0.0f; }
void SynthGuiInterface::connectModulation(std::string source, std::string destination) { }
void SynthGuiInterface::connectModulation(vital::ModulationConnection* connection) { }
void SynthGuiInterface::setModulationValues(const std::string& source, const std::string& destination,
                                            vital::mono_float amount, bool bipolar, bool stereo, bool bypass) { }
void SynthGuiInterface::disconnectModulation(std::string source, std::string destination) { }
void SynthGuiInterface::disconnectModulation(vital::ModulationConnection* connection) { }
void SynthGuiInterface::setFocus() { }
void SynthGuiInterface::notifyChange() { }
void SynthGuiInterface::notifyFresh() { }
void SynthGuiInterface::openSaveDialog() { }
void SynthGuiInterface::externalPresetLoaded(File preset) { }
void SynthGuiInterface::setGuiSize(float scale) { }

#else

#include "default_look_and_feel.h"
#include "full_interface.h"

SynthGuiInterface::SynthGuiInterface(SynthBase* synth, bool use_gui) : synth_(synth) {
  if (use_gui) {
    // Create the FullInterface GUI and initialize it with current synth data.
    LineGenerator* lfo_sources[vital::kNumLfos];
    for (int i = 0; i < vital::kNumLfos; ++i)
      lfo_sources[i] = synth->getLfoSource(i);
    SynthGuiData synth_data(synth_);
    gui_ = std::make_unique<FullInterface>(&synth_data);
  }
}

SynthGuiInterface::~SynthGuiInterface() { }

void SynthGuiInterface::updateFullGui() {
  if (gui_ == nullptr)
    return;

  // Refresh all parameter values and GUI components.
  gui_->setAllValues(synth_->getControls());
  gui_->reset();
}

void SynthGuiInterface::updateGuiControl(const std::string& name, vital::mono_float value) {
  if (gui_ == nullptr)
    return;

  // Update a single control in the GUI.
  gui_->setValue(name, value, NotificationType::dontSendNotification);
}

vital::mono_float SynthGuiInterface::getControlValue(const std::string& name) {
  return synth_->getControls()[name]->value();
}

void SynthGuiInterface::notifyModulationsChanged() {
  gui_->modulationChanged();
}

void SynthGuiInterface::notifyModulationValueChanged(int index) {
  gui_->modulationValueChanged(index);
}

void SynthGuiInterface::connectModulation(std::string source, std::string destination) {
  bool created = synth_->connectModulation(source, destination);
  if (created)
    initModulationValues(source, destination);
  notifyModulationsChanged();
}

void SynthGuiInterface::connectModulation(vital::ModulationConnection* connection) {
  synth_->connectModulation(connection);
  notifyModulationsChanged();
}

void SynthGuiInterface::initModulationValues(const std::string& source, const std::string& destination) {
  // Initialize newly created modulations with default line mappings and values.
  int connection_index = synth_->getConnectionIndex(source, destination);
  if (connection_index < 0)
    return;

  vital::ModulationConnection* connection = synth_->getModulationBank().atIndex(connection_index);
  LineGenerator* map_generator = connection->modulation_processor->lineMapGenerator();
  map_generator->initLinear();

  // Reset power to zero and update GUI.
  std::string power_name = "modulation_" + std::to_string(connection_index + 1) + "_power";
  synth_->valueChanged(power_name, 0.0f);
  gui_->setValue(power_name, 0.0f, NotificationType::dontSendNotification);
}

void SynthGuiInterface::setModulationValues(const std::string& source, const std::string& destination,
                                            vital::mono_float amount, bool bipolar, bool stereo, bool bypass) {
  // Update modulation parameters in both the synth and GUI.
  int connection_index = synth_->getConnectionIndex(source, destination);
  if (connection_index < 0)
    return;

  std::string number = std::to_string(connection_index + 1);
  std::string amount_name = "modulation_" + number + "_amount";
  std::string bipolar_name = "modulation_" + number + "_bipolar";
  std::string stereo_name = "modulation_" + number + "_stereo";
  std::string bypass_name = "modulation_" + number + "_bypass";

  float bipolar_amount = bipolar ? 1.0f : 0.0f;
  float stereo_amount = stereo ? 1.0f : 0.0f;
  float bypass_amount = bypass ? 1.0f : 0.0f;

  synth_->valueChanged(amount_name, amount);
  synth_->valueChanged(bipolar_name, bipolar_amount);
  synth_->valueChanged(stereo_name, stereo_amount);
  synth_->valueChanged(bypass_name, bypass_amount);
  gui_->setValue(amount_name, amount, NotificationType::dontSendNotification);
  gui_->setValue(bipolar_name, bipolar_amount, NotificationType::dontSendNotification);
  gui_->setValue(stereo_name, stereo_amount, NotificationType::dontSendNotification);
  gui_->setValue(bypass_name, bypass_amount, NotificationType::dontSendNotification);
}

void SynthGuiInterface::disconnectModulation(std::string source, std::string destination) {
  // Remove a modulation connection and notify GUI.
  synth_->disconnectModulation(source, destination);
  notifyModulationsChanged();
}

void SynthGuiInterface::disconnectModulation(vital::ModulationConnection* connection) {
  // Remove a modulation connection given a ModulationConnection object.
  synth_->disconnectModulation(connection);
  notifyModulationsChanged();
}

void SynthGuiInterface::setFocus() {
  if (gui_ == nullptr)
    return;

  gui_->setFocus();
}

void SynthGuiInterface::notifyChange() {
  if (gui_ == nullptr)
    return;

  gui_->notifyChange();
}

void SynthGuiInterface::notifyFresh() {
  if (gui_ == nullptr)
    return;

  gui_->notifyFresh();
}

void SynthGuiInterface::openSaveDialog() {
  if (gui_ == nullptr)
    return;

  gui_->openSaveDialog();
}

void SynthGuiInterface::externalPresetLoaded(File preset) {
  if (gui_ == nullptr)
    return;

  gui_->externalPresetLoaded(preset);
}

void SynthGuiInterface::setGuiSize(float scale) {
  if (gui_ == nullptr)
    return;

  // Adjust GUI size based on given scale and display constraints.
  Point<int> position = gui_->getScreenBounds().getCentre();
  const Displays::Display& display = Desktop::getInstance().getDisplays().findDisplayForPoint(position);

  Rectangle<int> display_area = Desktop::getInstance().getDisplays().getTotalBounds(true);
  ComponentPeer* peer = gui_->getPeer();
  if (peer)
    peer->getFrameSize().subtractFrom(display_area);

  float window_size = scale / display.scale;
  window_size = std::min(window_size, display_area.getWidth() * 1.0f / vital::kDefaultWindowWidth);
  window_size = std::min(window_size, display_area.getHeight() * 1.0f / vital::kDefaultWindowHeight);
  LoadSave::saveWindowSize(window_size);

  int width = std::round(window_size * vital::kDefaultWindowWidth);
  int height = std::round(window_size * vital::kDefaultWindowHeight);

  Rectangle<int> bounds = gui_->getBounds();
  bounds.setWidth(width);
  bounds.setHeight(height);
  gui_->getParentComponent()->setBounds(bounds);
  gui_->redoBackground();
}
#endif
