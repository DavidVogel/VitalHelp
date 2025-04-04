#include "effects_modulation_handler.h"

#include "envelope.h"
#include "filters_module.h"
#include "line_map.h"
#include "operators.h"
#include "portamento_slope.h"
#include "random_lfo_module.h"
#include "synth_constants.h"
#include "envelope_module.h"
#include "lfo_module.h"
#include "trigger_random.h"
#include "value_switch.h"
#include "modulation_connection_processor.h"

namespace vital {

  /**
   * @brief Constructs an EffectsModulationHandler and sets up the MIDI offset output.
   *
   * @param beats_per_second Pointer to an Output that tracks tempo in BPS.
   */
  EffectsModulationHandler::EffectsModulationHandler(Output* beats_per_second)
      : VoiceHandler(0, 1, true),
        beats_per_second_(beats_per_second),
        note_from_reference_(nullptr),
        midi_offset_output_(nullptr),
        bent_midi_(nullptr),
        current_midi_note_(nullptr),
        filters_module_(nullptr),
        lfos_(),
        envelopes_(),
        lfo_sources_(),
        random_(nullptr),
        random_lfos_(),
        note_mapping_(nullptr),
        velocity_mapping_(nullptr),
        aftertouch_mapping_(nullptr),
        slide_mapping_(nullptr),
        lift_mapping_(nullptr),
        mod_wheel_mapping_(nullptr),
        pitch_wheel_mapping_(nullptr),
        stereo_(nullptr),
        note_percentage_(nullptr) {

    note_from_reference_ = new cr::Add();
    midi_offset_output_ = registerOutput(note_from_reference_->output());
  }

  /**
   * @brief Initializes the entire modulation system by creating articulation, modulators, and filters.
   *
   * Also initializes the modulation connection processors and registers them in the
   * @c modulation_bank_.
   */
  void EffectsModulationHandler::init() {
    createArticulation();
    createModulators();
    createFilters(note_from_reference_->output());

    // Setup macro controls
    Output* macros[kNumMacros];
    for (int i = 0; i < kNumMacros; ++i)
      macros[i] = createMonoModControl("macro_control_" + std::to_string(i + 1));

    // Initialize all possible modulation connections
    for (int i = 0; i < vital::kMaxModulationConnections; ++i) {
      ModulationConnectionProcessor* processor =
          modulation_bank_.atIndex(i)->modulation_processor.get();

      processor->plug(reset(), ModulationConnectionProcessor::kReset);

      // Setup modulation amount controls
      std::string number = std::to_string(i + 1);
      std::string amount_name = "modulation_" + number + "_amount";
      Output* modulation_amount = createPolyModControl(amount_name);
      processor->plug(modulation_amount, ModulationConnectionProcessor::kModulationAmount);
      processor->initializeBaseValue(data_->controls[amount_name]);

      Value* modulation_power = createBaseControl("modulation_" + number + "_power");
      processor->plug(modulation_power, ModulationConnectionProcessor::kModulationPower);

      addProcessor(processor);
      addSubmodule(processor);
      processor->enable(false);
    }

    // Initialize voice handling
    VoiceHandler::init();
    setupPolyModulationReadouts();

    // Register macros as mod sources
    for (int i = 0; i < kNumMacros; ++i) {
      std::string name = "macro_control_" + std::to_string(i + 1);
      data_->mod_sources[name] = macros[i];
      createStatusOutput(name, macros[i]);
    }

    // Register random LFOs as mod sources
    for (int i = 0; i < kNumRandomLfos; ++i) {
      std::string name = "random_" + std::to_string(i + 1);
      data_->mod_sources[name] = random_lfos_[i]->output();
      createStatusOutput(name, random_lfos_[i]->output());
    }

    // Add random and stereo mod sources
    data_->mod_sources["random"] = random_->output();
    data_->mod_sources["stereo"] = stereo_->output();

    createStatusOutput("random", random_->output());
    createStatusOutput("stereo", stereo_->output());

    // Create status outputs for each modulation connection
    std::string modulation_source_prefix = "modulation_source_";
    std::string modulation_amount_prefix = "modulation_amount_";
    for (int i = 0; i < vital::kMaxModulationConnections; ++i) {
      ModulationConnectionProcessor* processor =
          modulation_bank_.atIndex(i)->modulation_processor.get();
      std::string number = std::to_string(i + 1);
      Output* source_output = processor->output(ModulationConnectionProcessor::kModulationSource);
      Output* pre_scale_output = processor->output(ModulationConnectionProcessor::kModulationPreScale);
      createStatusOutput(modulation_source_prefix + number, source_output);
      createStatusOutput(modulation_amount_prefix + number, pre_scale_output);
    }
  }

  /**
   * @brief Removes modulation processors from the audio graph in preparation for destruction.
   */
  void EffectsModulationHandler::prepareDestroy() {
    for (int i = 0; i < vital::kMaxModulationConnections; ++i) {
      ModulationConnectionProcessor* processor = modulation_bank_.atIndex(i)->modulation_processor.get();
      removeProcessor(processor);
    }
  }

  /**
   * @brief Creates the LFO, envelope, and random modulation modules, hooking them into the mod system.
   */
  void EffectsModulationHandler::createModulators() {
    // Create LFO modules
    for (int i = 0; i < kNumLfos; ++i) {
      lfo_sources_[i].setLoop(false);
      lfo_sources_[i].initTriangle();
      std::string prefix = "lfo_" + std::to_string(i + 1);
      LfoModule* lfo = new LfoModule(prefix, &lfo_sources_[i], beats_per_second_);
      addSubmodule(lfo);
      addProcessor(lfo);
      lfos_[i] = lfo;

      lfo->plug(retrigger(), LfoModule::kNoteTrigger);
      lfo->plug(note_count(), LfoModule::kNoteCount);
      lfo->plug(bent_midi_, LfoModule::kMidi);

      data_->mod_sources[prefix] = lfo->output(LfoModule::kValue);
      createStatusOutput(prefix, lfo->output(LfoModule::kValue));
      createStatusOutput(prefix + "_phase", lfo->output(LfoModule::kOscPhase));
      createStatusOutput(prefix + "_frequency", lfo->output(LfoModule::kOscFrequency));
    }

    // Create envelope modules
    for (int i = 0; i < kNumEnvelopes; ++i) {
      std::string prefix = "env_" + std::to_string(i + 1);
      EnvelopeModule* envelope = new EnvelopeModule(prefix);
      envelope->plug(retrigger(), EnvelopeModule::kTrigger);
      addSubmodule(envelope);
      addProcessor(envelope);
      envelopes_[i] = envelope;

      data_->mod_sources[prefix] = envelope->output();
      createStatusOutput(prefix, envelope->output(EnvelopeModule::kValue));
      createStatusOutput(prefix + "_phase", envelope->output(EnvelopeModule::kPhase));
    }

    // Create a random trigger generator
    random_ = new TriggerRandom();
    random_->plug(retrigger());
    addProcessor(random_);

    // Create random LFO modules
    for (int i = 0; i < kNumRandomLfos; ++i) {
      std::string name = "random_" + std::to_string(i + 1);
      random_lfos_[i] = new RandomLfoModule(name, beats_per_second_);
      random_lfos_[i]->plug(retrigger(), RandomLfoModule::kNoteTrigger);
      random_lfos_[i]->plug(bent_midi_, RandomLfoModule::kMidi);
      addSubmodule(random_lfos_[i]);
      addProcessor(random_lfos_[i]);
    }

    // Stereo factor
    stereo_ = new cr::Value(constants::kLeftOne);
    addIdleProcessor(stereo_);

    // Add standard mod sources
    data_->mod_sources["note"] = note_percentage_->output();
    data_->mod_sources["note_in_octave"] = note_in_octave();
    data_->mod_sources["aftertouch"] = aftertouch();
    data_->mod_sources["velocity"] = velocity();
    data_->mod_sources["slide"] = slide();
    data_->mod_sources["lift"] = lift();
    data_->mod_sources["mod_wheel"] = mod_wheel();
    data_->mod_sources["pitch_wheel"] = pitch_wheel_percent();

    // Create status outputs for these mod sources
    createStatusOutput("note", note_percentage_->output());
    createStatusOutput("note_in_octave", note_in_octave());
    createStatusOutput("aftertouch", aftertouch());
    createStatusOutput("velocity", velocity());
    createStatusOutput("slide", slide());
    createStatusOutput("lift", lift());
    createStatusOutput("mod_wheel", mod_wheel());
    createStatusOutput("pitch_wheel", pitch_wheel_percent());
  }

  /**
   * @brief Creates a FiltersModule and plugs in reset, keytrack, and MIDI references.
   *
   * @param keytrack Pointer to the output used for MIDI keytracking data.
   */
  void EffectsModulationHandler::createFilters(Output* keytrack) {
    filters_module_ = new FiltersModule();
    addSubmodule(filters_module_);
    addProcessor(filters_module_);

    filters_module_->plug(reset(), FiltersModule::kReset);
    filters_module_->plug(keytrack, FiltersModule::kKeytrack);
    filters_module_->plug(bent_midi_, FiltersModule::kMidi);
  }

  /**
   * @brief Creates processors for articulating note pitch (portamento, pitch bend, etc.).
   */
  void EffectsModulationHandler::createArticulation() {
    // Velocity tracking
    Output* velocity_track_amount = createPolyModControl("velocity_track");
    cr::Interpolate* velocity_track_mult = new cr::Interpolate();
    velocity_track_mult->plug(&constants::kValueOne, Interpolate::kFrom);
    velocity_track_mult->plug(velocity(), Interpolate::kTo);
    velocity_track_mult->plug(velocity_track_amount, Interpolate::kFractional);
    addProcessor(velocity_track_mult);

    // Portamento controls
    Output* portamento = createPolyModControl("portamento_time");
    Output* portamento_slope = createPolyModControl("portamento_slope");
    Value* portamento_force = createBaseControl("portamento_force");
    Value* portamento_scale = createBaseControl("portamento_scale");

    current_midi_note_ = new PortamentoSlope();
    current_midi_note_->plug(last_note(), PortamentoSlope::kSource);
    current_midi_note_->plug(note(), PortamentoSlope::kTarget);
    current_midi_note_->plug(portamento_force, PortamentoSlope::kPortamentoForce);
    current_midi_note_->plug(portamento_scale, PortamentoSlope::kPortamentoScale);
    current_midi_note_->plug(portamento, PortamentoSlope::kRunSeconds);
    current_midi_note_->plug(portamento_slope, PortamentoSlope::kSlopePower);
    current_midi_note_->plug(voice_event(), PortamentoSlope::kReset);
    current_midi_note_->plug(note_pressed(), PortamentoSlope::kNumNotesPressed);
    setVoiceMidi(current_midi_note_->output());
    addProcessor(current_midi_note_);

    // Pitch bend
    Output* pitch_bend_range = createPolyModControl("pitch_bend_range");
    Output* voice_tune = createPolyModControl("voice_tune");
    cr::Multiply* pitch_bend = new cr::Multiply();
    pitch_bend->plug(pitch_wheel(), 0);
    pitch_bend->plug(pitch_bend_range, 1);

    bent_midi_ = new cr::VariableAdd();
    bent_midi_->plugNext(current_midi_note_);
    bent_midi_->plugNext(pitch_bend);
    bent_midi_->plugNext(local_pitch_bend());
    bent_midi_->plugNext(voice_tune);

    static const cr::Value max_midi_invert(1.0f / (kMidiSize - 1));
    note_percentage_ = new cr::Multiply();
    note_percentage_->plug(&max_midi_invert, 0);
    note_percentage_->plug(bent_midi_, 1);
    addProcessor(note_percentage_);

    static const cr::Value reference_adjust(-kMidiTrackCenter);
    note_from_reference_->plug(&reference_adjust, 0);
    note_from_reference_->plug(bent_midi_, 1);
    addProcessor(note_from_reference_);

    addProcessor(pitch_bend);
    addProcessor(bent_midi_);
  }

  /**
   * @brief Processes a block of samples, updating modulations and voice states.
   *
   * @param num_samples Number of samples to process.
   */
  void EffectsModulationHandler::process(int num_samples) {
    poly_mask reset_mask = reset()->trigger_mask;
    if (reset_mask.anyMask())
      resetFeedbacks(reset_mask);

    VoiceHandler::process(num_samples);
    note_retriggered_.clearTrigger();

    // If no notes are active, clear status outputs
    if (getNumActiveVoices() == 0) {
      for (auto& status_source : data_->status_outputs)
        status_source.second->clear();
    }
    // Otherwise, update active connections
    else {
      poly_mask voice_mask = getCurrentVoiceMask();
      for (int i = 0; i < vital::kMaxModulationConnections; ++i) {
        ModulationConnectionProcessor* processor = modulation_bank_.atIndex(i)->modulation_processor.get();
        if (processor->enabled()) {
          poly_float* buffer = processor->output()->buffer;
          poly_float masked_value = buffer[0] & voice_mask;
          buffer[0] = masked_value + utils::swapVoices(masked_value);
        }
      }
      for (auto& status_source : data_->status_outputs)
        status_source.second->update(voice_mask);
    }
  }

  /**
   * @brief Called when a note-on event occurs.
   *
   * Triggers the note retrigger output if legato is disabled or if the polyphony
   * permits a new note. Then delegates to the VoiceHandler noteOn().
   *
   * @param note     The MIDI note number.
   * @param velocity The note-on velocity.
   * @param sample   The sample index for the note-on.
   * @param channel  The MIDI channel.
   */
  void EffectsModulationHandler::noteOn(int note, mono_float velocity, int sample, int channel) {
    if (getNumPressedNotes() < polyphony() || !legato())
      note_retriggered_.trigger(constants::kFullMask, note, sample);
    VoiceHandler::noteOn(note, velocity, sample, channel);
  }

  /**
   * @brief Called when a note-off event occurs.
   *
   * Retriggers if more pressed notes remain beyond the polyphony limit (and not legato).
   * Delegates to the VoiceHandler noteOff().
   *
   * @param note    The MIDI note number.
   * @param lift    The note-off velocity.
   * @param sample  The sample index for the note-off.
   * @param channel The MIDI channel.
   */
  void EffectsModulationHandler::noteOff(int note, mono_float lift, int sample, int channel) {
    if (getNumPressedNotes() > polyphony() && isNotePlaying(note) && !legato())
      note_retriggered_.trigger(constants::kFullMask, note, sample);

    VoiceHandler::noteOff(note, lift, sample, channel);
  }

  /**
   * @brief Synchronizes internal LFO phases and other time-based modulators to a specific time.
   *
   * @param seconds The absolute time in seconds.
   */
  void EffectsModulationHandler::correctToTime(double seconds) {
    for (int i = 0; i < kNumLfos; ++i)
      lfos_[i]->correctToTime(seconds);
  }

  /**
   * @brief Disables LFOs, envelopes (except env_1), and random mod sources for CPU saving.
   */
  void EffectsModulationHandler::disableUnnecessaryModSources() {
    for (int i = 0; i < kNumLfos; ++i)
      lfos_[i]->enable(false);

    for (int i = 1; i < kNumEnvelopes; ++i)
      envelopes_[i]->enable(false);

    random_->enable(false);
  }

  /**
   * @brief Disables a specific mod source by its name string (e.g., "env_1", "lfo_2", etc.).
   *
   * @param source The name of the modulation source to disable.
   */
  void EffectsModulationHandler::disableModSource(const std::string& source) {
    if (source != "env_1")
      getModulationSource(source)->owner->enable(false);
  }

  /**
   * @brief Sets up polyphonic modulation readouts from the synthesizer's global poly_modulations map.
   */
  void EffectsModulationHandler::setupPolyModulationReadouts() {
    output_map& poly_mods = SynthModule::getPolyModulations();
    for (auto& mod : poly_mods)
      poly_readouts_[mod.first] = registerOutput(mod.second);
  }

  /**
   * @brief Returns the internal map of polyphonic modulation outputs.
   *
   * @return A reference to poly_readouts_.
   */
  output_map& EffectsModulationHandler::getPolyModulations() {
    return poly_readouts_;
  }
} // namespace vital
