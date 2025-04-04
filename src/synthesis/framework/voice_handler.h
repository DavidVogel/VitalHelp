#pragma once

/**
 * @file voice_handler.h
 * @brief Declares classes and data structures to handle polyphonic voices in Vital,
 *        including voice assignment, note handling, and parameter routing.
 */

#include "circular_queue.h"
#include "note_handler.h"
#include "processor_router.h"
#include "synth_module.h"
#include "tuning.h"

#include <map>
#include <list>

namespace vital {

  /**
   * @struct VoiceState
   * @brief Holds state data for a single voice, such as MIDI note, velocity, pitch bend, etc.
   */
  struct VoiceState {
    /**
     * @brief Default constructor initializes state to neutral or inactive values.
     */
    VoiceState() : event(kInvalid), midi_note(0), tuned_note(0.0f), last_note(0.0f), velocity(0.0f),
                   lift(0.0f), local_pitch_bend(0.0f), note_pressed(0), note_count(0),
                   channel(0), sostenuto_pressed(false) { }

    VoiceEvent event;         ///< The most recent voice event (on/off/kill).
    int midi_note;            ///< MIDI note (0-127 usually).
    mono_float tuned_note;    ///< Possibly adjusted by a Tuning object.
    poly_float last_note;     ///< Holds the last note played for this voice.
    mono_float velocity;      ///< Velocity of the note-on event.
    mono_float lift;          ///< Velocity of the note-off (a.k.a. release velocity).
    mono_float local_pitch_bend;  ///< Per-voice pitch bend amount for legato-like transitions.
    int note_pressed;         ///< Pressed note count (e.g., for note priority logic).
    int note_count;           ///< A global note counter (incremented with each note-on).
    int channel;              ///< Which MIDI channel this voice is associated with.
    bool sostenuto_pressed;   ///< True if this voice is currently held by sostenuto pedal.
  };

  struct AggregateVoice; // Documented below.

  /**
   * @class Voice
   * @brief Represents a single playing note/voice, including voice-state and event handling.
   *
   * Voices can be grouped into AggregateVoice sets, with each group sharing a Processor.
   * This class stores the note data and manages transitions between states (on/off/sustain).
   */
  class Voice {
    public:
      /// Default lift velocity to use if none is provided.
      static constexpr mono_float kDefaultLiftVelocity = 0.5f;

      /**
       * @enum KeyState
       * @brief Describes the lifecycle stage of a voice:
       *        kTriggering -> kHeld -> kReleased -> kDead, with special states for sustain.
       */
      enum KeyState {
        kTriggering,  ///< Note-on occurred, but hasn't processed yet.
        kHeld,        ///< The note is actively held down.
        kSustained,   ///< The note has ended, but sustain pedal is holding it on.
        kReleased,    ///< The note has ended (off event) and is releasing.
        kDead,        ///< The voice is no longer active.
        kNumStates
      };

      /**
       * @brief Constructs a Voice owned by a given AggregateVoice.
       * @param parent Pointer to the owning AggregateVoice.
       */
      Voice(AggregateVoice* parent);

      /**
       * @brief Disabled default constructor: Voice must belong to an AggregateVoice.
       */
      Voice() = delete;

      /**
       * @brief Virtual destructor.
       */
      virtual ~Voice() { }

      /// Returns the pointer to the parent AggregateVoice.
      force_inline AggregateVoice* parent() { return parent_; }

      /// Returns a const reference to the VoiceState struct that holds all relevant data.
      force_inline const VoiceState& state() { return state_; }

      /// Returns the previous key state (before the most recent update).
      force_inline const KeyState last_key_state() { return last_key_state_; }

      /// Returns the current key state.
      force_inline const KeyState key_state() { return key_state_; }

      /// Returns the sample index at which the latest event (on/off) was triggered.
      force_inline int event_sample() { return event_sample_; }

      /// Returns the index of this voice within an AggregateVoice (also the SIMD lane grouping).
      force_inline int voice_index() { return voice_index_; }

      /// Returns the SIMD mask representing this voice's active lanes.
      force_inline poly_mask voice_mask() { return voice_mask_; }

      /// Returns the current aftertouch value for this voice.
      force_inline mono_float aftertouch() { return aftertouch_; }

      /// Returns the sample index at which the latest aftertouch event occurred.
      force_inline mono_float aftertouch_sample() { return aftertouch_sample_; }

      /// Returns the current slide (MPE expression) value for this voice.
      force_inline mono_float slide() { return slide_; }

      /// Returns the sample index at which the latest slide event occurred.
      force_inline mono_float slide_sample() { return slide_sample_; }

      /**
       * @brief Activates (starts) the voice with the given note parameters.
       * @param midi_note      MIDI note number.
       * @param tuned_note     Possibly adjusted note frequency (from Tuning).
       * @param velocity       Velocity of note-on.
       * @param last_note      The previous note value (for legato).
       * @param note_pressed   The count of pressed notes for note-on logic.
       * @param note_count     A global note increment for event ordering.
       * @param sample         The sample index at which note-on occurs.
       * @param channel        The MIDI channel for this note.
       */
      force_inline void activate(int midi_note, mono_float tuned_note, mono_float velocity,
                                 poly_float last_note, int note_pressed, int note_count,
                                 int sample, int channel) {
        event_sample_ = sample;
        state_.event = kVoiceOn;
        state_.midi_note = midi_note;
        state_.tuned_note = tuned_note;
        state_.velocity = velocity;
        state_.lift = kDefaultLiftVelocity;
        state_.local_pitch_bend = 0.0f;
        state_.last_note = last_note;
        state_.note_pressed = note_pressed;
        state_.note_count = note_count;
        state_.channel = channel;
        state_.sostenuto_pressed = false;
        aftertouch_ = 0.0f;
        aftertouch_sample_ = 0;
        slide_ = 0.0f;
        slide_sample_ = 0;
        setKeyState(kTriggering);
      }

      /**
       * @brief Sets the key state of this voice (e.g., from Triggering to Held).
       * @param key_state The new KeyState.
       */
      force_inline void setKeyState(KeyState key_state) {
        last_key_state_ = key_state_;
        key_state_ = key_state;
      }

      /**
       * @brief Switches this voice to the `kSustained` state, typically when a sustain pedal is active.
       */
      force_inline void sustain() {
        last_key_state_ = key_state_;
        key_state_ = kSustained;
      }

      /// Returns true if the voice is in the kSustained state.
      force_inline bool sustained() {
        return key_state_ == kSustained;
      }

      /// Returns true if the voice is in the kHeld state.
      force_inline bool held() {
        return key_state_ == kHeld;
      }

      /// Returns true if the voice is in the kReleased state.
      force_inline bool released() {
        return key_state_ == kReleased;
      }

      /// Returns true if the voice has sostenuto pressed.
      force_inline bool sostenuto() {
        return state_.sostenuto_pressed;
      }

      /// Sets the sostenuto flag on or off.
      force_inline void setSostenuto(bool sostenuto) {
        state_.sostenuto_pressed = sostenuto;
      }

      /// Sets the local pitch bend (used for legato transitions or channel pitch bend).
      force_inline void setLocalPitchBend(mono_float bend) {
        state_.local_pitch_bend = bend;
      }

      /// Adjusts the lift velocity (release velocity) of the note-off.
      force_inline void setLiftVelocity(mono_float lift) {
        state_.lift = lift;
      }

      /**
       * @brief Deactivates (turns off) this voice with a note-off event, transitioning to kReleased.
       * @param sample The sample index at which note-off occurs.
       */
      force_inline void deactivate(int sample = 0) {
        event_sample_ = sample;
        state_.event = kVoiceOff;
        setKeyState(kReleased);
      }

      /**
       * @brief Immediately kills this voice (disregarding release).
       * @param sample The sample index at which kill occurs.
       */
      force_inline void kill(int sample = 0) {
        event_sample_ = sample;
        state_.event = kVoiceKill;
      }

      /// Marks this voice as kDead, meaning it's completely inactive.
      force_inline void markDead() {
        setKeyState(kDead);
      }

      /// Checks if there is a new (non-processed) on/off event for this voice.
      force_inline bool hasNewEvent() {
        return event_sample_ >= 0;
      }

      /**
       * @brief Sets the aftertouch (pressure) value for the voice.
       * @param aftertouch The new aftertouch value.
       * @param sample     The sample index at which the event is received.
       */
      force_inline void setAftertouch(mono_float aftertouch, int sample = 0) {
        aftertouch_ = aftertouch;
        aftertouch_sample_ = sample;
      }

      /**
       * @brief Sets the MPE "slide" value for the voice (often CC#74).
       * @param slide  The new slide value.
       * @param sample The sample index at which the event is received.
       */
      force_inline void setSlide(mono_float slide, int sample = 0) {
        slide_ = slide;
        slide_sample_ = sample;
      }

      /// Returns true if there's a new aftertouch event not yet processed.
      force_inline bool hasNewAftertouch() {
        return aftertouch_sample_ >= 0;
      }

      /// Returns true if there's a new slide event not yet processed.
      force_inline bool hasNewSlide() {
        return slide_sample_ >= 0;
      }

      /**
       * @brief Completes (consumes) the voice event, marking it as processed.
       *        If the voice was kTriggering, transitions it to kHeld.
       */
      force_inline void completeVoiceEvent() {
        event_sample_ = -1;
        if (key_state_ == kTriggering)
          setKeyState(kHeld);
      }

      /**
       * @brief Shifts the event sample index by `num_samples` (e.g., for partial block processing).
       * @param num_samples How many samples to shift.
       */
      force_inline void shiftVoiceEvent(int num_samples) {
        event_sample_ -= num_samples;
        VITAL_ASSERT(event_sample_ >= 0);
      }

      /**
       * @brief Shifts the aftertouch event sample index by `num_samples`.
       */
      force_inline void shiftAftertouchEvent(int num_samples) {
        aftertouch_sample_ -= num_samples;
        VITAL_ASSERT(aftertouch_sample_ >= 0);
      }

      /**
       * @brief Shifts the slide event sample index by `num_samples`.
       */
      force_inline void shiftSlideEvent(int num_samples) {
        slide_sample_ -= num_samples;
        VITAL_ASSERT(slide_sample_ >= 0);
      }

      /// Clears the unprocessed aftertouch event, if any.
      force_inline void clearAftertouchEvent() {
        aftertouch_sample_ = -1;
      }

      /// Clears the unprocessed slide event, if any.
      force_inline void clearSlideEvent() {
        aftertouch_sample_ = -1;
      }

      /**
       * @brief Clears both note-on/off events and aftertouch events, marking them processed.
       */
      force_inline void clearEvents() {
        event_sample_ = -1;
        aftertouch_sample_ = -1;
      }

      /**
       * @brief Stores references to other Voices in the same parallel group for advanced sharing logic.
       * @param shared_voices A list of pointers to parallel voices in the same AggregateVoice.
       */
      force_inline void setSharedVoices(std::vector<Voice*> shared_voices) {
        for (Voice* voice : shared_voices) {
          if (voice != this)
            shared_voices_.push_back(voice);
        }
      }

      /**
       * @brief Sets the voice index within its parallel group and the corresponding SIMD mask.
       * @param voice_index An integer index (0..kParallelVoices-1).
       * @param voice_mask  A poly_mask enabling only the lanes relevant to this voice.
       */
      force_inline void setVoiceInfo(int voice_index, poly_mask voice_mask) {
        voice_index_ = voice_index;
        voice_mask_ = voice_mask;
      }

    private:
      int voice_index_;             ///< Index within the AggregateVoice, used for SIMD grouping.
      poly_mask voice_mask_;        ///< SIMD mask for this voice's lanes.
      std::vector<Voice*> shared_voices_; ///< Optional references to other voices in the same group.

      int event_sample_;            ///< Sample index of the most recent event (on/off).
      VoiceState state_;            ///< The primary state for this voice (MIDI data, pitch, etc.).
      KeyState last_key_state_;     ///< The previous KeyState, used for transitions.
      KeyState key_state_;          ///< Current KeyState (held, sustained, etc.).

      int aftertouch_sample_;       ///< Sample index for aftertouch event.
      mono_float aftertouch_;       ///< Latest aftertouch value.

      int slide_sample_;            ///< Sample index for MPE "slide" event.
      mono_float slide_;            ///< Latest slide value.

      AggregateVoice* parent_;      ///< The parent aggregate voice set that groups multiple voices.
  };

  /**
   * @struct AggregateVoice
   * @brief An aggregate grouping that pairs multiple (parallel) voices with a shared Processor instance.
   *
   * Vital uses parallel voices within an AggregateVoice to handle SIMD lanes efficiently.
   */
  struct AggregateVoice {
    CircularQueue<Voice*> voices;     ///< Collection of active Voice pointers.
    std::unique_ptr<Processor> processor; ///< A single processor instance shared by these voices.
  };

  /**
   * @class VoiceHandler
   * @brief A SynthModule and NoteHandler that manages a pool of polyphonic voices,
   *        handles note-on/off logic, and routes the data to multiple processors.
   *
   * The VoiceHandler class takes in note events (MIDI or otherwise) and dispatches
   * them to available or stolen voices, tracks voice states (e.g., sustaining), and
   * provides outputs that other modules can use to render audio or handle modulation
   * based on active voices.
   */
  class VoiceHandler : public SynthModule, public NoteHandler {
    public:
      /// Range of local pitch bend in semitones for each voice.
      static constexpr mono_float kLocalPitchBendRange = 48.0f;

      /**
       * @enum
       * @brief Input indexes for VoiceHandler control parameters.
       */
      enum {
        kPolyphony,       ///< Desired polyphony setting (1..kMaxActivePolyphony).
        kVoicePriority,   ///< Priority scheme for stealing or reassigning voices.
        kVoiceOverride,   ///< Behavior when exceeding polyphony: kill or steal.
        kNumInputs
      };

      /**
       * @enum VoiceOverride
       * @brief Behavior for assigning a new note when at max polyphony.
       */
      enum VoiceOverride {
        kKill,  ///< Immediately kill an existing voice to free one.
        kSteal, ///< Steal an existing voice that is in a certain state (released/sustained).
        kNumVoiceOverrides
      };

      /**
       * @enum VoicePriority
       * @brief Determines the voice stealing strategy (oldest, newest, highest, etc.).
       */
      enum VoicePriority {
        kNewest,
        kOldest,
        kHighest,
        kLowest,
        kRoundRobin,
        kNumVoicePriorities
      };

      /**
       * @brief Constructs a VoiceHandler with a given polyphony and outputs.
       * @param num_outputs How many output channels to allocate (e.g., for various mod outputs).
       * @param polyphony   Number of voices to allow.
       * @param control_rate True if running at control rate (buffer_size == 1).
       */
      VoiceHandler(int num_outputs, int polyphony, bool control_rate = false);

      /// Disabled default constructor; must specify polyphony and output count.
      VoiceHandler() = delete;

      /**
       * @brief Virtual destructor.
       */
      virtual ~VoiceHandler();

      /**
       * @brief Clones this VoiceHandler. Not supported for this class.
       * @return Always returns nullptr after asserting false.
       */
      virtual Processor* clone() const override {
        VITAL_ASSERT(false);
        return nullptr;
      }

      /**
       * @brief Processes audio for a block of samples. For each active voice,
       *        triggers events, updates parameters, and runs its Processor.
       * @param num_samples The number of samples in this processing block.
       */
      virtual void process(int num_samples) override;

      /**
       * @brief Initializes the voice and global routers, then calls SynthModule::init().
       */
      virtual void init() override;

      /**
       * @brief Sets the sample rate for both mono/global and voice (poly) routers.
       * @param sample_rate The base sample rate (pre-oversampling).
       */
      virtual void setSampleRate(int sample_rate) override;

      /**
       * @brief Sets the custom Tuning object (if any) for note->frequency conversion.
       * @param tuning Pointer to a Tuning object.
       */
      void setTuning(const Tuning* tuning) { tuning_ = tuning; }

      /**
       * @brief Returns the number of currently active voices (not dead).
       */
      int getNumActiveVoices();

      /**
       * @brief Returns how many notes are pressed (including partial states).
       */
      force_inline int getNumPressedNotes() { return pressed_notes_.size(); }

      /**
       * @brief Checks if a given MIDI note is playing.
       * @param note The MIDI note to check.
       * @return True if any voice is on with that note, false otherwise.
       */
      bool isNotePlaying(int note);

      /**
       * @brief Checks if a given MIDI note is playing on a particular channel.
       * @param note    The MIDI note to check.
       * @param channel The MIDI channel to check.
       * @return True if a matching voice is found, false otherwise.
       */
      bool isNotePlaying(int note, int channel);

      // NoteHandler overrides:
      void allSoundsOff() override;
      void allNotesOff(int sample) override;
      void allNotesOff(int sample, int channel) override;
      void allNotesOffRange(int sample, int from_channel, int to_channel);

      virtual void noteOn(int note, mono_float velocity, int sample, int channel) override;
      virtual void noteOff(int note, mono_float velocity, int sample, int channel) override;

      /**
       * @brief Handles per-note aftertouch for a specific note and channel.
       * @param note        The MIDI note.
       * @param aftertouch  The aftertouch value.
       * @param sample      The sample index for the event.
       * @param channel     MIDI channel.
       */
      void setAftertouch(int note, mono_float aftertouch, int sample, int channel);

      /**
       * @brief Sets channel-wide aftertouch (applies to all voices on that channel).
       * @param channel     MIDI channel.
       * @param aftertouch  The aftertouch value.
       * @param sample      The sample index for the event.
       */
      void setChannelAftertouch(int channel, mono_float aftertouch, int sample);

      /**
       * @brief Sets channel-wide aftertouch for a range of channels.
       */
      void setChannelRangeAftertouch(int from_channel, int to_channel, mono_float aftertouch, int sample);

      /**
       * @brief Sets channel-wide MPE "slide" for a single channel.
       */
      void setChannelSlide(int channel, mono_float aftertouch, int sample);

      /**
       * @brief Sets channel-wide MPE "slide" for a range of channels.
       */
      void setChannelRangeSlide(int from_channel, int to_channel, mono_float aftertouch, int sample);

      /// Turns on sustain for a single channel.
      void sustainOn(int channel);
      /// Turns off sustain for a single channel, prompting voices to release.
      void sustainOff(int sample, int channel);

      /// Turns on sostenuto for a single channel.
      void sostenutoOn(int channel);
      /// Turns off sostenuto for a single channel, prompting release if not sustained.
      void sostenutoOff(int sample, int channel);

      /// Turns on sustain for a range of channels.
      void sustainOnRange(int from_channel, int to_channel);
      /// Turns off sustain for a range of channels, prompting voices to release.
      void sustainOffRange(int sample, int from_channel, int to_channel);

      /// Turns on sostenuto for a range of channels.
      void sostenutoOnRange(int from_channel, int to_channel);
      /// Turns off sostenuto for a range of channels, prompting release if not sustained.
      void sostenutoOffRange(int sample, int from_channel, int to_channel);

      /**
       * @brief Returns a mask for the last active voice, used for writing to output buffers.
       */
      poly_mask getCurrentVoiceMask();

      /**
       * @brief Sets the mod wheel value for a single channel.
       * @param value   The new mod wheel value [0..1].
       * @param channel The MIDI channel.
       */
      force_inline void setModWheel(mono_float value, int channel = 0) {
        VITAL_ASSERT(channel < kNumMidiChannels && channel >= 0);
        mod_wheel_values_[channel] = value;
      }

      /**
       * @brief Sets the mod wheel value for all channels at once.
       * @param value The new mod wheel value [0..1].
       */
      force_inline void setModWheelAllChannels(mono_float value) {
        for (int i = 0; i < kNumMidiChannels; ++i)
          mod_wheel_values_[i] = value;
      }

      /**
       * @brief Sets the pitch wheel value for a single channel, applying to all held voices on that channel.
       * @param value   The new pitch wheel value [0..1].
       * @param channel The MIDI channel.
       */
      force_inline void setPitchWheel(mono_float value, int channel = 0) {
        VITAL_ASSERT(channel < kNumMidiChannels && channel >= 0);
        pitch_wheel_values_[channel] = value;
        for (Voice* voice : active_voices_) {
          if (voice->state().channel == channel && voice->held())
            voice->setLocalPitchBend(value);
        }
      }

      /**
       * @brief Sets pitch wheel in a zoned manner for a range of MIDI channels.
       * @param value        The new pitch wheel value.
       * @param from_channel Starting channel.
       * @param to_channel   Ending channel (inclusive).
       */
      force_inline void setZonedPitchWheel(mono_float value, int from_channel, int to_channel) {
        VITAL_ASSERT(from_channel < kNumMidiChannels && from_channel >= 0);
        VITAL_ASSERT(to_channel < kNumMidiChannels && to_channel >= 0);
        VITAL_ASSERT(to_channel >= from_channel);
        for (int i = from_channel; i <= to_channel; ++i)
          zoned_pitch_wheel_values_[i] = value;
      }

      /// Returns a pointer to the voice_event Output, used to track voice On/Off/Kill events.
      force_inline Output* voice_event() { return &voice_event_; }

      /// Returns a pointer to the retrigger Output, used for controlling certain envelope triggers.
      force_inline Output* retrigger() { return &retrigger_; }

      /// Returns a pointer to the reset Output, indicating a full voice reset (On from Dead).
      force_inline Output* reset() { return &reset_; }

      /// Returns a pointer to the note Output, giving the current tuned note frequency or pitch.
      force_inline Output* note() { return &note_; }

      /// Returns a pointer to the last_note Output, giving the previous note (for legato transitions).
      force_inline Output* last_note() { return &last_note_; }

      /// Returns a pointer to the note_pressed Output, a count of how many times a note was pressed.
      force_inline Output* note_pressed() { return &note_pressed_; }

      /// Returns a pointer to note_count, a global note counter.
      force_inline Output* note_count() { return &note_count_; }

      /// Returns a pointer to note_in_octave, a fractional note position in [0..1).
      force_inline Output* note_in_octave() { return &note_in_octave_; }

      /// Returns a pointer to channel, indicating the MIDI channel of the voice.
      force_inline Output* channel() { return &channel_; }

      /// Returns a pointer to velocity, the note-on velocity.
      force_inline Output* velocity() { return &velocity_; }

      /// Returns a pointer to lift, the note-off velocity or release velocity.
      force_inline Output* lift() { return &lift_; }

      /// Returns a pointer to aftertouch, storing per-voice or channel-based aftertouch.
      force_inline Output* aftertouch() { return &aftertouch_; }

      /// Returns a pointer to slide, the MPE "slide" expression value.
      force_inline Output* slide() { return &slide_; }

      /// Returns a pointer to active_mask, a mask that indicates which voices are active.
      force_inline Output* active_mask() { return &active_mask_; }

      /// Returns a pointer to pitch_wheel, storing the pitch-bend range for each channel.
      force_inline Output* pitch_wheel() { return &pitch_wheel_; }

      /// Returns a pointer to pitch_wheel_percent, a normalized [0..1] version of pitch_wheel.
      force_inline Output* pitch_wheel_percent() { return &pitch_wheel_percent_; }

      /// Returns a pointer to local_pitch_bend, the per-voice pitch bend output.
      force_inline Output* local_pitch_bend() { return &local_pitch_bend_; }

      /// Returns a pointer to mod_wheel, storing the mod wheel value for each channel.
      force_inline Output* mod_wheel() { return &mod_wheel_; }

      /**
       * @brief Retrieves the accumulated Output associated with a given output pointer.
       * @param output The original Output pointer.
       * @return A pointer to the internally managed Output that accumulates across voices.
       */
      force_inline Output* getAccumulatedOutput(Output* output) { return accumulated_outputs_[output].get(); }

      /**
       * @brief Returns the current maximum polyphony (number of active voices allowed).
       */
      force_inline int polyphony() { return polyphony_; }

      /**
       * @brief Gets the last active note's tuned frequency (or 0 if none).
       */
      mono_float getLastActiveNote() const;

      /**
       * @brief Returns the monophonic (global) ProcessorRouter for this VoiceHandler.
       */
      virtual ProcessorRouter* getMonoRouter() override { return &global_router_; }

      /**
       * @brief Returns the polyphonic (voice) ProcessorRouter for this VoiceHandler.
       */
      virtual ProcessorRouter* getPolyRouter() override { return &voice_router_; }

      // Overriding processor management from SynthModule:
      void addProcessor(Processor* processor) override;
      void addIdleProcessor(Processor* processor) override;
      void removeProcessor(Processor* processor) override;

      /**
       * @brief Adds a Processor to the "global" (monophonic) router, e.g. for final mixing or master effects.
       */
      void addGlobalProcessor(Processor* processor);

      /**
       * @brief Removes a Processor from the global router.
       */
      void removeGlobalProcessor(Processor* processor);

      /**
       * @brief Resets any feedback paths in the poly router, applying the given mask.
       * @param reset_mask The mask of voices to reset.
       */
      void resetFeedbacks(poly_mask reset_mask) override;

      /**
       * @brief Registers an Output with this VoiceHandler, returning a pointer to a new accumulated or single-lane Output.
       * @param output The original Output pointer from a Processor.
       * @return The new Output pointer (owned by VoiceHandler).
       */
      Output* registerOutput(Output* output) override;

      /**
       * @brief Registers a control-rate Output with the VoiceHandler.
       * @param output The original Output pointer.
       * @param active Whether this output is "active" (if false, it won't get updates).
       * @return The new control-rate Output pointer owned by VoiceHandler.
       */
      Output* registerControlRateOutput(Output* output, bool active);

      /**
       * @brief Not implemented index-based registerOutput override (asserts false).
       */
      Output* registerOutput(Output* output, int index) override;

      /**
       * @brief Sets the polyphony to a new value, allocating or freeing voices as needed.
       * @param polyphony The desired number of voices (1..kMaxActivePolyphony).
       */
      void setPolyphony(int polyphony);

      /**
       * @brief Specifies an Output from a Processor used to detect silence or inactivity for voice killing.
       * @param killer A pointer to the "voice killer" Output.
       */
      force_inline void setVoiceKiller(const Output* killer) {
        voice_killer_ = killer;
      }

      /// Overload for setting the voice killer from a Processor directly.
      force_inline void setVoiceKiller(const Processor* killer) {
        setVoiceKiller(killer->output());
      }

      /**
       * @brief Sets the Output that provides the current MIDI note for the voice.
       * @param midi Output pointer (e.g., note_).
       */
      force_inline void setVoiceMidi(const Output* midi) {
        voice_midi_ = midi;
      }

      /**
       * @brief Enables or disables legato mode (disables retriggers if still in Held state).
       * @param legato True to enable legato mode, false otherwise.
       */
      force_inline void setLegato(bool legato) {
        legato_ = legato;
      }

      /// Returns true if legato mode is enabled.
      force_inline bool legato() {
        return legato_;
      }

      /**
       * @brief Checks if a given Processor is handled by the poly (voice) router.
       * @param processor The Processor to check.
       * @return True if `processor` belongs to the voice_router_, false otherwise.
       */
      bool isPolyphonic(const Processor* processor) const override;

      /**
       * @brief Sets the oversampling factor for both SynthModule and the internal poly/mono routers.
       * @param oversample The new oversampling factor.
       */
      virtual void setOversampleAmount(int oversample) override {
        SynthModule::setOversampleAmount(oversample);
        voice_router_.setOversampleAmount(oversample);
        global_router_.setOversampleAmount(oversample);
      }

      /**
       * @brief Marks an Output as "active" for non-accumulated usage (e.g., for the last active voice only).
       * @param output The original Output pointer.
       */
      void setActiveNonaccumulatedOutput(Output* output);

      /**
       * @brief Marks an Output as "inactive" for non-accumulated usage, effectively disabling it.
       * @param output The original Output pointer.
       */
      void setInactiveNonaccumulatedOutput(Output* output);

    protected:
      /**
       * @brief Determines whether an Output should be summed across voices (accumulated) or handled individually.
       * @param output The Output pointer.
       * @return True if it should be accumulated, false if not (e.g., control-rate).
       */
      virtual bool shouldAccumulate(Output* output);

    private:
      // Internal utility methods for grabbing, assigning, and processing voices.
      Voice* grabVoice();
      Voice* grabFreeVoice();
      Voice* grabFreeParallelVoice();
      Voice* grabVoiceOfType(Voice::KeyState key_state);
      Voice* getVoiceToKill(int max_voices);
      int grabNextUnplayedPressedNote();
      void sortVoicePriority();
      void addParallelVoices();
      void prepareVoiceTriggers(AggregateVoice* aggregate_voice, int num_samples);
      void prepareVoiceValues(AggregateVoice* aggregate_voice);
      void processVoice(AggregateVoice* aggregate_voice, int num_samples);
      void clearAccumulatedOutputs();
      void clearNonaccumulatedOutputs();
      void accumulateOutputs(int num_samples);
      void combineAccumulatedOutputs(int num_samples);
      void writeNonaccumulatedOutputs(poly_mask voice_mask, int num_samples);

      int polyphony_;                        ///< Current max polyphony (voice count).
      bool legato_;                          ///< If true, we don't retrigger envelope if a note is still held.

      std::map<Output*, std::unique_ptr<Output>> last_voice_outputs_; ///< Outputs used for single-voice data (non-accumulated).
      CircularQueue<std::pair<Output*, Output*>> nonaccumulated_outputs_; ///< Pairs mapping from a source output to the single-lane output.
      std::map<Output*, std::unique_ptr<Output>> accumulated_outputs_;   ///< For outputs that sum signals from multiple voices.

      const Output* voice_killer_; ///< Points to an output used to detect silent or dying voices.
      const Output* voice_midi_;   ///< Points to the note Output in typical usage.

      int last_num_voices_;        ///< Number of active voices in the last process() call.
      poly_float last_played_note_;///< The last note triggered (for legato).

      // Outputs published by VoiceHandler for other processors:
      cr::Output voice_event_;
      cr::Output retrigger_;
      cr::Output reset_;
      cr::Output note_;
      cr::Output last_note_;
      cr::Output note_pressed_;
      cr::Output note_count_;
      cr::Output note_in_octave_;
      cr::Output channel_;
      cr::Output velocity_;
      cr::Output lift_;
      cr::Output aftertouch_;
      cr::Output slide_;
      cr::Output active_mask_;
      cr::Output mod_wheel_;
      cr::Output pitch_wheel_;
      cr::Output pitch_wheel_percent_;
      cr::Output local_pitch_bend_;

      bool sustain_[kNumMidiChannels];      ///< Sustain pedal states per channel.
      bool sostenuto_[kNumMidiChannels];    ///< Sostenuto pedal states per channel.
      mono_float mod_wheel_values_[kNumMidiChannels];        ///< Mod wheel values per channel.
      mono_float pitch_wheel_values_[kNumMidiChannels];      ///< Pitch wheel values per channel.
      mono_float zoned_pitch_wheel_values_[kNumMidiChannels];///< Pitch wheel values for channel zones.
      mono_float pressure_values_[kNumMidiChannels];         ///< Channel aftertouch or pressure.
      mono_float slide_values_[kNumMidiChannels];            ///< Channel-based MPE "slide" values.

      const Tuning* tuning_;               ///< Pointer to the current Tuning table (can be nullptr).
      VoicePriority voice_priority_;       ///< Strategy for voice stealing or selection.
      VoiceOverride voice_override_;       ///< Behavior when at max polyphony (kill vs. steal).

      int total_notes_;                    ///< A global note count incremented with every note-on.
      CircularQueue<int> pressed_notes_;   ///< A queue of pressed note/channel combos for note priority logic.

      // Pools of Voice and AggregateVoice objects:
      CircularQueue<std::unique_ptr<Voice>> all_voices_;
      CircularQueue<Voice*> free_voices_;
      CircularQueue<Voice*> active_voices_;


      CircularQueue<std::unique_ptr<AggregateVoice>> all_aggregate_voices_;
      CircularQueue<AggregateVoice*> active_aggregate_voices_;

      // Two routers: one for each voice (poly), one for global (mono).
      ProcessorRouter voice_router_;
      ProcessorRouter global_router_;

      JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoiceHandler)
  };
} // namespace vital

