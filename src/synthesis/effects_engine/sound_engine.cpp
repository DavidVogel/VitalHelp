#include "sound_engine.h"

#include "compressor_module.h"
#include "flanger_module.h"
#include "phaser_module.h"
#include "decimator.h"
#include "modulation_connection_processor.h"
#include "synth_constants.h"
#include "effects_modulation_handler.h"
#include "peak_meter.h"
#include "operators.h"
#include "reorderable_effect_chain.h"
#include "value_switch.h"

namespace vital {

  /**
   * @brief Default constructor for SoundEngine, initializes vital controls and reserves space for mod processors.
   */
  SoundEngine::SoundEngine()
    : SynthModule(0, 1),
      modulation_handler_(nullptr),
      last_oversampling_amount_(-1),
      last_sample_rate_(-1),
      peak_meter_(nullptr) {

    init();
    bps_ = data_->controls["beats_per_minute"];
    modulation_processors_.reserve(kMaxModulationConnections);
  }

  /**
   * @brief Destructor. Prepares the modulation handler for deletion by removing processors.
   */
  SoundEngine::~SoundEngine() {
    modulation_handler_->prepareDestroy();
  }

  /**
   * @brief Initializes base controls, sets up the EffectsModulationHandler, effect chain, decimator, stereo handling, etc.
   */
  void SoundEngine::init() {
    createBaseControl("bypass");
    oversampling_ = createBaseControl("oversampling");
    legato_ = createBaseControl("legato");

    // Stereo routing
    Output* stereo_routing = createMonoModControl("stereo_routing");
    Value* stereo_mode = createBaseControl("stereo_mode");
    Output* beats_per_second = createMonoModControl("beats_per_minute");
    cr::LowerBound* beats_per_second_clamped = new cr::LowerBound(0.0f);
    beats_per_second_clamped->plug(beats_per_second);
    addProcessor(beats_per_second_clamped);

    // Polyphony and voice priority
    Output* polyphony = createMonoModControl("polyphony");
    Value* voice_priority = createBaseControl("voice_priority");
    Value* voice_override = createBaseControl("voice_override");

    // Create modulation handler and hook up polyphony settings
    modulation_handler_ = new EffectsModulationHandler(beats_per_second_clamped->output());
    addSubmodule(modulation_handler_);
    modulation_handler_->setPolyphony(vital::kMaxPolyphony);
    modulation_handler_->plug(polyphony, VoiceHandler::kPolyphony);
    modulation_handler_->plug(voice_priority, VoiceHandler::kVoicePriority);
    modulation_handler_->plug(voice_override, VoiceHandler::kVoiceOverride);
    addProcessor(modulation_handler_);

    // Pitch wheel / mod wheel controls
    Value* pitch_wheel = createBaseControl("pitch_wheel");
    modulation_handler_->setPitchWheelControl(pitch_wheel);
    Value* mod_wheel = createBaseControl("mod_wheel");
    modulation_handler_->setModWheelControl(mod_wheel);

    // Upsampler for oversampling
    upsampler_ = new Upsampler();
    addIdleProcessor(upsampler_);

    // Create the ReorderableEffectChain
    Value* effect_chain_order = createBaseControl("effect_chain_order");
    effect_chain_ = new ReorderableEffectChain(beats_per_second, modulation_handler_->midi_offset_output());
    addSubmodule(effect_chain_);
    addProcessor(effect_chain_);
    effect_chain_->plug(upsampler_, ReorderableEffectChain::kAudio);
    effect_chain_->plug(effect_chain_order, ReorderableEffectChain::kOrder);

    // Set up compressor readouts
    SynthModule* compressor = effect_chain_->getEffect(constants::kCompressor);
    createStatusOutput("compressor_low_input", compressor->output(CompressorModule::kLowInputMeanSquared));
    createStatusOutput("compressor_band_input", compressor->output(CompressorModule::kBandInputMeanSquared));
    createStatusOutput("compressor_high_input", compressor->output(CompressorModule::kHighInputMeanSquared));
    createStatusOutput("compressor_low_output", compressor->output(CompressorModule::kLowOutputMeanSquared));
    createStatusOutput("compressor_band_output", compressor->output(CompressorModule::kBandOutputMeanSquared));
    createStatusOutput("compressor_high_output", compressor->output(CompressorModule::kHighOutputMeanSquared));

    // Chorus effect readouts
    SynthModule* chorus = effect_chain_->getEffect(constants::kChorus);
    for (int i = 0; i < ChorusModule::kMaxDelayPairs; ++i)
      createStatusOutput("chorus_delays" + std::to_string(i + 1), chorus->output(i + 1));

    // Phaser readout
    SynthModule* phaser = effect_chain_->getEffect(constants::kPhaser);
    createStatusOutput("phaser_cutoff", phaser->output(PhaserModule::kCutoffOutput));

    // Flanger readout
    SynthModule* flanger = effect_chain_->getEffect(constants::kFlanger);
    createStatusOutput("flanger_delay_frequency", flanger->output(FlangerModule::kFrequencyOutput));

    // Decimator at the end of the chain
    Decimator* decimator = new Decimator(3);
    decimator->plug(effect_chain_);
    addProcessor(decimator);

    // Stereo encoding
    StereoEncoder* decoder = new StereoEncoder(true);
    decoder->plug(decimator, StereoEncoder::kAudio);
    decoder->plug(stereo_routing, StereoEncoder::kEncodingValue);
    decoder->plug(stereo_mode, StereoEncoder::kMode);
    addProcessor(decoder);

    // Final volume and peak meter
    Output* volume = createMonoModControl("volume");
    SmoothVolume* scaled_audio = new SmoothVolume();
    scaled_audio->plug(decoder, SmoothVolume::kAudioRate);
    scaled_audio->plug(volume, SmoothVolume::kDb);

    peak_meter_ = new PeakMeter();
    peak_meter_->plug(scaled_audio, 0);
    createStatusOutput("peak_meter", peak_meter_->output());

    Clamp* clamp = new Clamp(-2.1f, 2.1f);
    clamp->plug(scaled_audio);

    addProcessor(peak_meter_);
    addProcessor(scaled_audio);
    addProcessor(clamp);

    // Use clamp’s output for the SoundEngine final output
    clamp->useOutput(output());

    // Finalize initialization
    SynthModule::init();
    disableUnnecessaryModSources();
    setOversamplingAmount(kDefaultOversamplingAmount, kDefaultSampleRate);
  }

  /**
   * @brief Connects a modulation source to a destination using the provided modulation_change details.
   *
   * Sets control rate based on whether either the source or destination is audio-rate.
   */
  void SoundEngine::connectModulation(const modulation_change& change) {
    change.modulation_processor->plug(change.source, ModulationConnectionProcessor::kModulationInput);
    change.modulation_processor->setDestinationScale(change.destination_scale);
    VITAL_ASSERT(vital::utils::isFinite(change.destination_scale));

    Processor* destination = change.mono_destination;
    bool polyphonic = change.source->owner->isPolyphonic() && change.poly_destination;
    change.modulation_processor->setPolyphonicModulation(polyphonic);
    if (polyphonic)
      destination = change.poly_destination;

    // If source and destination are both audio-rate, so is the processor
    if (!destination->isControlRate() && !change.source->isControlRate()) {
      change.source->owner->setControlRate(false);
      change.modulation_processor->setControlRate(false);
    }

    change.source->owner->enable(true);
    change.modulation_processor->enable(true);
    destination->plugNext(change.modulation_processor);
    change.modulation_processor->process(1);
    destination->process(1);

    // Mark modulation switchers as connected
    change.mono_modulation_switch->set(1);
    if (change.poly_modulation_switch)
      change.poly_modulation_switch->set(1);

    modulation_processors_.push_back(change.modulation_processor);
  }

  /**
   * @brief Returns the number of pressed notes from the modulation handler.
   *
   * @return The count of pressed notes.
   */
  int SoundEngine::getNumPressedNotes() {
    return modulation_handler_->getNumPressedNotes();
  }

  /**
   * @brief Removes a previously connected modulation, disabling the processor and resetting rate modes.
   */
  void SoundEngine::disconnectModulation(const modulation_change& change) {
    Processor* destination = change.mono_destination;
    if (change.source->owner->isPolyphonic() && change.poly_destination)
      destination = change.poly_destination;

    destination->unplug(change.modulation_processor);

    // If no other connections remain, disable the modulation
    if (change.mono_destination->connectedInputs() == 1 &&
        (change.poly_destination == nullptr || change.poly_destination->connectedInputs() == 0)) {
      change.mono_modulation_switch->set(0);
      if (change.poly_modulation_switch)
        change.poly_modulation_switch->set(0);
    }

    change.modulation_processor->enable(false);
    change.modulation_processor->setControlRate(true);
    if (change.num_audio_rate == 0)
      change.source->owner->setControlRate(true);
    modulation_processors_.remove(change.modulation_processor);
  }

  /**
   * @brief Retrieves the number of active voices managed by the modulation handler.
   *
   * @return Active voice count.
   */
  int SoundEngine::getNumActiveVoices() {
    return modulation_handler_->getNumActiveVoices();
  }

  /**
   * @brief Provides access to the internal modulation bank for connecting sources and destinations.
   *
   * @return A reference to the ModulationConnectionBank from the modulation handler.
   */
  ModulationConnectionBank& SoundEngine::getModulationBank() {
    return modulation_handler_->getModulationBank();
  }

  /**
   * @brief Retrieves the last note that was active in the engine.
   *
   * @return The most recently pressed note value.
   */
  mono_float SoundEngine::getLastActiveNote() const {
    return modulation_handler_->getLastActiveNote();
  }

  /**
   * @brief Sets a custom tuning table for note pitch mapping.
   *
   * @param tuning Pointer to a Tuning object that maps MIDI notes to frequencies.
   */
  void SoundEngine::setTuning(const Tuning* tuning) {
    modulation_handler_->setTuning(tuning);
  }

  /**
   * @brief Checks if the oversampling setting or sample rate has changed and re-applies if needed.
   */
  void SoundEngine::checkOversampling() {
    int oversampling_amount = oversampling_->value();
    int sample_rate = getSampleRate();
    if (last_oversampling_amount_ != oversampling_amount || last_sample_rate_ != sample_rate) {
      setOversamplingAmount(1 << oversampling_amount, sample_rate);
      last_oversampling_amount_ = oversampling_amount;
      last_sample_rate_ = sample_rate;
    }
  }

  /**
   * @brief Configures oversampling for the engine, upsampler, modulation handler, and effect chain.
   *
   * @param oversampling_amount The integer oversampling factor.
   * @param sample_rate         The current audio sample rate in Hz.
   */
  void SoundEngine::setOversamplingAmount(int oversampling_amount, int sample_rate) {
    static constexpr int kBaseSampleRate = 44100;
    int oversample = oversampling_amount;
    int sample_rate_mult = sample_rate / kBaseSampleRate;

    // Attempt to reduce oversample if the sample rate is already higher
    while (sample_rate_mult > 1 && oversample > 1) {
      sample_rate_mult >>= 1;
      oversample >>= 1;
    }

    upsampler_->setOversampleAmount(oversample);
    modulation_handler_->setOversampleAmount(oversample);
    effect_chain_->setOversampleAmount(oversample);
  }

  /**
   * @brief Processes audio through the entire chain of the SoundEngine.
   *
   * @param audio_in    Buffer of samples (poly_float), though many effects do not use direct audio_in.
   * @param num_samples The block size to process.
   */
  void SoundEngine::processWithInput(const poly_float* audio_in, int num_samples) {
    VITAL_ASSERT(num_samples <= output()->buffer_size);

    FloatVectorOperations::disableDenormalisedNumberSupport();
    modulation_handler_->setLegato(legato_->value());

    // Oversampler runs first
    upsampler_->processWithInput(audio_in, num_samples);
    ProcessorRouter::process(num_samples);

    // Update status outputs
    for (auto& status_source : data_->status_outputs)
      status_source.second->update();
  }

  /**
   * @brief Synchronizes time-based modules (effects chain, mod handler) to an absolute time.
   *
   * @param seconds Absolute time in seconds.
   */
  void SoundEngine::correctToTime(double seconds) {
    modulation_handler_->correctToTime(seconds);
    effect_chain_->correctToTime(seconds);
  }

  /**
   * @brief Clears the effect chain states, stopping sound or lingering effects.
   */
  void SoundEngine::allSoundsOff() {
    effect_chain_->hardReset();
  }

  /**
   * @brief Disables all notes across all channels at a given sample index.
   *
   * @param sample The sample index to stop.
   */
  void SoundEngine::allNotesOff(int sample) {
    modulation_handler_->allNotesOff(sample);
  }

  /**
   * @brief Disables all notes on a specific channel at a given sample.
   *
   * @param sample  The sample index.
   * @param channel The MIDI channel.
   */
  void SoundEngine::allNotesOff(int sample, int channel) {
    modulation_handler_->allNotesOff(channel);
  }

  /**
   * @brief Disables all notes across a range of channels.
   *
   * @param sample       The sample index.
   * @param from_channel The first channel in the range.
   * @param to_channel   The last channel in the range (inclusive).
   */
  void SoundEngine::allNotesOffRange(int sample, int from_channel, int to_channel) {
    modulation_handler_->allNotesOffRange(sample, from_channel, to_channel);
  }

  /**
   * @brief Handles note-on events, triggering voices in the modulation handler.
   *
   * @param note     MIDI note number.
   * @param velocity Note-on velocity.
   * @param sample   Sample index for the event.
   * @param channel  MIDI channel.
   */
  void SoundEngine::noteOn(int note, mono_float velocity, int sample, int channel) {
    modulation_handler_->noteOn(note, velocity, sample, channel);
  }

  /**
   * @brief Handles note-off events, releasing the voice in the modulation handler.
   *
   * @param note    MIDI note number.
   * @param lift    Note-off velocity.
   * @param sample  Sample index for the event.
   * @param channel MIDI channel.
   */
  void SoundEngine::noteOff(int note, mono_float lift, int sample, int channel) {
    modulation_handler_->noteOff(note, lift, sample, channel);
  }

  /**
   * @brief Sets the mod wheel value for a specific channel.
   *
   * @param value   The mod wheel value [0..1].
   * @param channel The MIDI channel.
   */
  void SoundEngine::setModWheel(mono_float value, int channel) {
    modulation_handler_->setModWheel(value, channel);
  }

  /**
   * @brief Sets the mod wheel value for all channels.
   *
   * @param value The mod wheel value [0..1].
   */
  void SoundEngine::setModWheelAllChannels(mono_float value) {
    modulation_handler_->setModWheelAllChannels(value);
  }

  /**
   * @brief Sets the pitch wheel value for a specific channel.
   *
   * @param value   The pitch bend amount in [-1..1].
   * @param channel The MIDI channel.
   */
  void SoundEngine::setPitchWheel(mono_float value, int channel) {
    modulation_handler_->setPitchWheel(value, channel);
  }

  /**
   * @brief Sets a pitch wheel value for a range of channels.
   *
   * @param value        The pitch bend in [-1..1].
   * @param from_channel The first channel in the range.
   * @param to_channel   The last channel in the range (inclusive).
   */
  void SoundEngine::setZonedPitchWheel(mono_float value, int from_channel, int to_channel) {
    modulation_handler_->setZonedPitchWheel(value, from_channel, to_channel);
  }

  /**
   * @brief Disables any unnecessary mod sources in the modulation handler for CPU efficiency.
   */
  void SoundEngine::disableUnnecessaryModSources() {
    modulation_handler_->disableUnnecessaryModSources();
  }

  /**
   * @brief Enables a named modulation source by enabling its owner module.
   *
   * @param source The name of the modulation source, e.g. "lfo_1".
   */
  void SoundEngine::enableModSource(const std::string& source) {
    getModulationSource(source)->owner->enable(true);
  }

  /**
   * @brief Disables a named modulation source, e.g. "env_2".
   *
   * @param source The name of the source.
   */
  void SoundEngine::disableModSource(const std::string& source) {
    modulation_handler_->disableModSource(source);
  }

  /**
   * @brief Checks if a named modulation source is currently enabled.
   *
   * @param source The name of the source to check.
   * @return True if enabled, false otherwise.
   */
  bool SoundEngine::isModSourceEnabled(const std::string& source) {
    return getModulationSource(source)->owner->enabled();
  }

  /**
   * @brief Retrieves the stereo memory used by an equalizer effect in the chain (if any).
   *
   * @return A pointer to the StereoMemory or nullptr if not present.
   */
  const StereoMemory* SoundEngine::getEqualizerMemory() {
    return effect_chain_->getEqualizerMemory();
  }

  /**
   * @brief Updates the engine’s internal beats-per-minute, stored as beats_per_second.
   *
   * @param bpm The new BPM value.
   */
  void SoundEngine::setBpm(mono_float bpm) {
    mono_float bps = bpm / 60.0f;
    if (bps_->value() != bps)
      bps_->set(bps);
  }

  /**
   * @brief Sets polyphonic aftertouch for a specific note.
   *
   * @param note    MIDI note number.
   * @param value   Aftertouch in [0..1].
   * @param sample  Sample index.
   * @param channel MIDI channel.
   */
  void SoundEngine::setAftertouch(mono_float note, mono_float value, int sample, int channel) {
    modulation_handler_->setAftertouch(note, value, sample, channel);
  }

  /**
   * @brief Sets channel-wide aftertouch for a single channel.
   *
   * @param channel MIDI channel.
   * @param value   Aftertouch in [0..1].
   * @param sample  Sample index.
   */
  void SoundEngine::setChannelAftertouch(int channel, mono_float value, int sample) {
    modulation_handler_->setChannelAftertouch(channel, value, sample);
  }

  /**
   * @brief Sets channel-wide aftertouch for all channels in [from_channel..to_channel].
   *
   * @param from_channel The first channel in the range.
   * @param to_channel   The last channel in the range (inclusive).
   * @param value        Aftertouch in [0..1].
   * @param sample       Sample index.
   */
  void SoundEngine::setChannelRangeAftertouch(int from_channel, int to_channel, mono_float value, int sample) {
    modulation_handler_->setChannelRangeAftertouch(from_channel, to_channel, value, sample);
  }

  /**
   * @brief Sets channel-wide slide on a single channel.
   *
   * @param channel MIDI channel.
   * @param value   Slide in [0..1].
   * @param sample  Sample index.
   */
  void SoundEngine::setChannelSlide(int channel, mono_float value, int sample) {
    modulation_handler_->setChannelSlide(channel, value, sample);
  }

  /**
   * @brief Sets channel-wide slide for all channels in the specified range.
   *
   * @param from_channel The first channel in the range.
   * @param to_channel   The last channel in the range.
   * @param value        Slide in [0..1].
   * @param sample       Sample index.
   */
  void SoundEngine::setChannelRangeSlide(int from_channel, int to_channel, mono_float value, int sample) {
    modulation_handler_->setChannelRangeSlide(from_channel, to_channel, value, sample);
  }

  /**
   * @brief Returns nullptr since wavetable retrieval is unimplemented here.
   */
  Wavetable* SoundEngine::getWavetable(int index) {
    return nullptr;
  }

  /**
   * @brief Returns nullptr since sample retrieval is unimplemented here.
   */
  Sample* SoundEngine::getSample() {
    return nullptr;
  }

  /**
   * @brief Retrieves the LFO source line generator by index from the modulation handler.
   *
   * @param index LFO index.
   * @return Pointer to the corresponding LineGenerator.
   */
  LineGenerator* SoundEngine::getLfoSource(int index) {
    return modulation_handler_->getLfoSource(index);
  }

  /**
   * @brief Turns sustain on for a given MIDI channel, holding all pressed notes.
   *
   * @param channel MIDI channel.
   */
  void SoundEngine::sustainOn(int channel) {
    modulation_handler_->sustainOn(channel);
  }

  /**
   * @brief Turns sustain off for a given channel, releasing notes if keys are up.
   *
   * @param sample  Sample index at which sustain is released.
   * @param channel MIDI channel.
   */
  void SoundEngine::sustainOff(int sample, int channel) {
    modulation_handler_->sustainOff(sample, channel);
  }

  /**
   * @brief Engages sostenuto on a single channel, holding only currently active notes.
   *
   * @param channel MIDI channel.
   */
  void SoundEngine::sostenutoOn(int channel) {
    modulation_handler_->sostenutoOn(channel);
  }

  /**
   * @brief Disengages sostenuto for a single channel.
   *
   * @param sample  Sample index at which sostenuto is released.
   * @param channel MIDI channel.
   */
  void SoundEngine::sostenutoOff(int sample, int channel) {
    modulation_handler_->sostenutoOff(sample, channel);
  }

  /**
   * @brief Engages sustain for all channels in a specified range.
   *
   * @param from_channel First channel in the range.
   * @param to_channel   Last channel in the range (inclusive).
   */
  void SoundEngine::sustainOnRange(int from_channel, int to_channel) {
    modulation_handler_->sustainOnRange(from_channel, to_channel);
  }

  /**
   * @brief Disengages sustain for all channels in the given range.
   *
   * @param sample       Sample index at which sustain is released.
   * @param from_channel First channel in the range.
   * @param to_channel   Last channel in the range (inclusive).
   */
  void SoundEngine::sustainOffRange(int sample, int from_channel, int to_channel) {
    modulation_handler_->sustainOffRange(sample, from_channel, to_channel);
  }

  /**
   * @brief Engages sostenuto for all channels in a specified range.
   *
   * @param from_channel First channel in the range.
   * @param to_channel   Last channel in the range (inclusive).
   */
  void SoundEngine::sostenutoOnRange(int from_channel, int to_channel) {
    modulation_handler_->sostenutoOnRange(from_channel, to_channel);
  }

  /**
   * @brief Disengages sostenuto for all channels in the given range.
   *
   * @param sample       Sample index at which sostenuto is released.
   * @param from_channel First channel in the range.
   * @param to_channel   Last channel in the range (inclusive).
   */
  void SoundEngine::sostenutoOffRange(int sample, int from_channel, int to_channel) {
    modulation_handler_->sostenutoOffRange(sample, from_channel, to_channel);
  }
} // namespace vital
