/**
 * @file processor.h
 * @brief Declares the Processor class and related structures for handling audio processing
 *        in a polyphonic context within the Vital synthesizer.
 *
 * This header defines the Processor class and its related Input and Output classes used to route
 * audio and control signals between various components of the synthesizer. The Processor class
 * forms a node in a graph of audio and control signal processing units. Inputs and outputs
 * provide access to buffers that hold generated or processed signals.
 *
 * Processors can operate at audio rate or control rate, and can be polyphonic, handling
 * multiple voices simultaneously.
 */

#pragma once

#include "common.h"
#include "poly_utils.h"

#include <cstring>
#include <vector>

namespace vital {

  class Processor;
  class ProcessorRouter;

  /**
   * @struct Output
   * @brief Holds and manages a buffer of samples (poly_float) for a Processor's output.
   *
   * This class also tracks trigger information such as trigger mask, value, and offset.
   * It can run at audio or control rates, depending on buffer_size.
   */
  struct Output {
    /**
     * @brief Constructs an Output with a specified buffer size and oversampling factor.
     * @param size The base number of samples in the buffer (e.g., kMaxBufferSize).
     * @param max_oversample Maximum oversample factor to allocate for.
     */
    Output(int size = kMaxBufferSize, int max_oversample = 1) {
      VITAL_ASSERT(size > 0);

      owner = nullptr;
      buffer_size = size * max_oversample;
      owned_buffer = std::make_unique<poly_float[]>(buffer_size);
      buffer = owned_buffer.get();
      clearBuffer();
      clearTrigger();
    }

    /**
     * @brief Virtual destructor.
     */
    virtual ~Output() { }

    /**
     * @brief Sets trigger values (mask, trigger value, and offset).
     * @param mask   The trigger mask that indicates which voices are triggering.
     * @param value  The trigger value for those triggered voices.
     * @param offset The sample offset at which the trigger occurs.
     */
    force_inline void trigger(poly_mask mask, poly_float value, poly_int offset) {
      trigger_mask |= mask;
      trigger_value = utils::maskLoad(trigger_value, value, mask);
      trigger_offset = utils::maskLoad(trigger_offset, offset, mask);
    }

    /**
     * @brief Clears the trigger mask, value, and offset.
     */
    force_inline void clearTrigger() {
      trigger_mask = 0;
      trigger_value = 0.0f;
      trigger_offset = 0;
    }

    /**
     * @brief Zeros out the entire output buffer.
     */
    void clearBuffer() {
      utils::zeroBuffer(owned_buffer.get(), buffer_size);
    }

    /**
     * @brief Checks whether this output runs at control rate (buffer_size == 1).
     * @return True if control rate, false if audio rate.
     */
    force_inline bool isControlRate() const {
      return buffer_size == 1;
    }

    /**
     * @brief Ensures the buffer is large enough to hold @p new_max_buffer_size samples.
     *        This will reallocate if necessary (unless already control rate).
     * @param new_max_buffer_size The requested new buffer size.
     */
    void ensureBufferSize(int new_max_buffer_size) {
      if (buffer_size >= new_max_buffer_size || buffer_size == 1)
        return;

      buffer_size = new_max_buffer_size;
      bool buffer_is_original = (buffer == owned_buffer.get());
      owned_buffer = std::make_unique<poly_float[]>(buffer_size);
      if (buffer_is_original)
        buffer = owned_buffer.get();
      clearBuffer();
    }

    poly_float* buffer;                          ///< Pointer to the output buffer.
    std::unique_ptr<poly_float[]> owned_buffer;  ///< Owned memory for the output buffer.
    Processor* owner;                            ///< Owning processor.

    int buffer_size;           ///< Current buffer size in samples.
    poly_mask trigger_mask;    ///< Mask for triggered voices.
    poly_float trigger_value;  ///< Trigger values for voices.
    poly_int trigger_offset;   ///< Sample offset (per voice) for triggers.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Output)
  };

  /**
   * @struct Input
   * @brief Represents a connection to an Output from another Processor.
   *
   * Provides read-access to the source buffer (the output of another Processor).
   */
  struct Input {
    /**
     * @brief Default constructor, initializes source to nullptr.
     */
    Input() : source(nullptr) { }

    const Output* source;  ///< The output from which this input reads samples.

    /**
     * @brief Returns the sample at index @p i from the source buffer.
     * @param i The sample index.
     * @return The sample as a poly_float.
     */
    force_inline poly_float at(int i) const {
      return source->buffer[i];
    }

    /**
     * @brief Operator[] overload for read-access to the source buffer.
     * @param i The sample index.
     * @return Reference to the poly_float sample.
     */
    force_inline const poly_float& operator[](std::size_t i) {
      return source->buffer[i];
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Input)
  };

  /**
   * @struct ProcessorState
   * @brief Holds shared state regarding sample rate, oversampling, and other flags.
   *
   * Multiple Processors may reference the same state if they share sample rate, etc.
   */
  struct ProcessorState {
    /**
     * @brief Default constructor, initializes to default sample rate and no oversampling.
     */
    ProcessorState() {
      sample_rate = kDefaultSampleRate;
      oversample_amount = 1;
      control_rate = false;
      enabled = true;
      initialized = false;
    }

    int sample_rate;         ///< The current sample rate (may include oversampling factor).
    int oversample_amount;   ///< Oversampling factor.
    bool control_rate;       ///< True if running at control rate (usually buffer_size == 1).
    bool enabled;            ///< Whether this Processor is currently enabled or not.
    bool initialized;        ///< Whether this Processor has been initialized.
  };

  namespace cr {
    /**
     * @struct Output
     * @brief A specialized Output that always runs at control rate (buffer_size = 1).
     *
     * This is used for control signals rather than audio signals.
     */
    struct Output : public ::vital::Output {
      /**
       * @brief Constructs a control-rate Output (always buffer_size = 1).
       */
      Output() {
        owner = nullptr;
        buffer_size = 1;
        owned_buffer = std::make_unique<poly_float[]>(1);
        buffer = &trigger_value;
        clearBuffer();
        clearTrigger();
      }
    };
  } // namespace cr

  /**
   * @class Processor
   * @brief Base class for all signal-processing units in Vital.
   *
   * A Processor can have multiple inputs and outputs, manages its own state, and
   * can be plugged into other Processors or have other Processors plugged in.
   * In typical use, a `ProcessorRouter` manages connections and calls `process()`.
   */
  class Processor {
    public:
      /**
       * @brief Constructs a Processor with a given number of inputs/outputs and oversampling.
       * @param num_inputs      How many input slots to allocate.
       * @param num_outputs     How many output slots to allocate.
       * @param control_rate    If true, the Processor runs at control rate (1 sample).
       * @param max_oversample  The maximum oversampling factor for its outputs.
       */
      Processor(int num_inputs, int num_outputs, bool control_rate = false, int max_oversample = 1);

      /**
       * @brief Virtual destructor.
       */
      virtual ~Processor() { }

      /**
       * @brief Clones this Processor for polyphonic expansion. Must be overridden by subclasses.
       * @return A pointer to a newly allocated Processor that is a copy of this Processor.
       */
      virtual Processor* clone() const = 0;

      /**
       * @brief Indicates whether this Processor requires per-voice state.
       * @return True if it has state per voice. Defaults to true.
       */
      virtual bool hasState() const { return true; }

      /**
       * @brief Main processing function. Called by the `ProcessorRouter`.
       * @param num_samples Number of samples to process.
       */
      virtual void process(int num_samples) = 0;

      /**
       * @brief An optional processing function taking explicit input buffer.
       *        Fallback is an assertion failure (not supported).
       * @param audio_in    Pointer to input buffer.
       * @param num_samples Number of samples to process.
       */
      virtual void processWithInput(const poly_float* audio_in, int num_samples) { VITAL_ASSERT(false); }

      /**
       * @brief Called after constructor, used for any additional initialization.
       *        Subclasses can override. Sets the initialized flag.
       */
      virtual void init() {
        VITAL_ASSERT(!initialized());
        state_->initialized = true;
      }

      /**
       * @brief Called to reset the Processor's per-voice state (e.g., on note-on).
       * @param reset_mask Mask of voices to reset.
       */
      virtual void reset(poly_mask reset_mask) { }

      /**
       * @brief Called to perform a "hard" reset for all voices.
       */
      virtual void hardReset() { reset(poly_mask(-1)); }

      /**
       * @brief Returns whether this Processor has been initialized.
       */
      bool initialized() {
        return state_->initialized;
      }

      /**
       * @brief Updates the sample rate of this Processor (scaled by oversampling).
       * @param sample_rate The new base sample rate (pre-oversampling).
       */
      virtual void setSampleRate(int sample_rate) {
        state_->sample_rate = sample_rate * state_->oversample_amount;
      }

      /**
       * @brief Sets the oversampling amount and updates the effective sample rate.
       * @param oversample The new oversampling factor.
       */
      virtual void setOversampleAmount(int oversample) {
        // Remove old oversampling factor, set new, then re-add.
        state_->sample_rate /= state_->oversample_amount;
        state_->oversample_amount = oversample;
        state_->sample_rate *= state_->oversample_amount;

        // Ensure our outputs match the new oversample buffer size.
        for (int i = 0; i < numOwnedOutputs(); ++i)
          ownedOutput(i)->ensureBufferSize(kMaxBufferSize * oversample);
        for (int i = 0; i < numOutputs(); ++i)
          output(i)->ensureBufferSize(kMaxBufferSize * oversample);
      }

      /**
       * @brief Checks if this Processor is enabled.
       * @return True if enabled, false otherwise.
       */
      force_inline bool enabled() const {
        return state_->enabled;
      }

      /**
       * @brief Enables or disables this Processor.
       * @param enable If true, sets the Processor to enabled; else disabled.
       */
      virtual void enable(bool enable) {
        state_->enabled = enable;
      }

      /**
       * @brief Retrieves the current (effective) sample rate.
       * @return The sample rate (includes oversampling).
       */
      force_inline int getSampleRate() const {
        return state_->sample_rate;
      }

      /**
       * @brief Retrieves the current oversampling factor.
       * @return The oversample amount.
       */
      force_inline int getOversampleAmount() const {
        return state_->oversample_amount;
      }

      /**
       * @brief Checks if this Processor is running at control rate (buffer_size == 1).
       * @return True if control rate, false otherwise.
       */
      force_inline bool isControlRate() const {
        return state_->control_rate;
      }

      /**
       * @brief Sets whether this Processor runs at control rate.
       * @param control_rate True to set control rate, false otherwise.
       */
      virtual void setControlRate(bool control_rate) {
        state_->control_rate = control_rate;
      }

      /**
       * @brief Retrieves a mask indicating which voices triggered a note-on event.
       *        Compares the input's trigger_value to kVoiceOn.
       * @param input_index Which input index to check.
       * @return A poly_mask of voices that match note-on.
       */
      force_inline poly_mask getResetMask(int input_index) const {
        poly_float trigger_val = inputs_->at(input_index)->source->trigger_value;
        return poly_float::equal(trigger_val, kVoiceOn);
      }

      /**
       * @brief Clears output samples for voices that are about to be reset, based on the trigger offset.
       * @param reset_mask   Mask of voices to reset.
       * @param input_index  Index of the input to read the trigger_offset from.
       * @param output_index Index of the output buffer to clear.
       *
       * This is a specialized method for multi-voice switching or gating.
       */
      force_inline void clearOutputBufferForReset(poly_mask reset_mask, int input_index, int output_index) const {
        poly_float* audio_out = output(output_index)->buffer;
        poly_int trigger_offset = input(input_index)->source->trigger_offset & reset_mask;

        // Example logic for partial clearing (based on trigger offsets).
        int num_samples_first = trigger_offset[0];
        poly_int mask(0, 0, -1, -1);
        for (int i = 0; i < num_samples_first; ++i)
          audio_out[i] = audio_out[i] & mask;

        mask = poly_int(-1, -1, 0, 0);
        int num_samples_second = trigger_offset[2];
        for (int i = 0; i < num_samples_second; ++i)
          audio_out[i] = audio_out[i] & mask;
      }

      /**
       * @brief Checks whether the buffer size of a particular input matches the size needed by this Processor.
       * @param input The input index to check.
       * @return True if it matches, false otherwise.
       */
      bool inputMatchesBufferSize(int input = 0);

      /**
       * @brief Checks if all inputs and outputs have buffers big enough for @p num_samples.
       * @param num_samples The requested number of samples to process.
       * @return True if valid sizes, false otherwise.
       */
      bool checkInputAndOutputSize(int num_samples);

      /**
       * @brief Checks if this Processor is polyphonic by querying its ProcessorRouter.
       * @return True if polyphonic, false if monophonic or no router.
       */
      virtual bool isPolyphonic() const;

      /**
       * @brief Connects an external Output to this Processor's first input.
       * @param source The output to connect.
       */
      void plug(const Output* source);

      /**
       * @brief Connects an external Output to a specified input index.
       * @param source      The output to connect.
       * @param input_index Which input slot to connect to.
       */
      void plug(const Output* source, unsigned int input_index);

      /**
       * @brief Connects the first output of a Processor to this Processor's first input.
       * @param source The Processor providing the output.
       */
      void plug(const Processor* source);

      /**
       * @brief Connects the first output of a Processor to a specified input index.
       * @param source      The Processor providing the output.
       * @param input_index Which input slot to connect to.
       */
      void plug(const Processor* source, unsigned int input_index);

      /**
       * @brief Connects an external Output to the first available (unplugged) input.
       * @param source The output to connect.
       */
      void plugNext(const Output* source);

      /**
       * @brief Connects the first output of a Processor to the first available (unplugged) input.
       * @param source The Processor providing the output.
       */
      void plugNext(const Processor* source);

      /**
       * @brief Uses an existing Input object as this Processor's first input.
       * @param input The existing Input object to use.
       */
      void useInput(Input* input);

      /**
       * @brief Uses an existing Input object at a specified input index.
       * @param input The existing Input object to use.
       * @param index The input slot index in this Processor.
       */
      void useInput(Input* input, int index);

      /**
       * @brief Uses an existing Output object as this Processor's first output.
       * @param output The existing Output object to use.
       */
      void useOutput(Output* output);

      /**
       * @brief Uses an existing Output object at a specified output index.
       * @param output The existing Output object to use.
       * @param index  The output slot index in this Processor.
       */
      void useOutput(Output* output, int index);

      /**
       * @brief Counts how many inputs are connected to a real source (not null_source_).
       * @return The number of connected inputs.
       */
      int connectedInputs();

      /**
       * @brief Removes the connection at a specified input index, if any.
       * @param input_index The input slot index to unplug.
       */
      virtual void unplugIndex(unsigned int input_index);

      /**
       * @brief Removes a connection to a given Output from all inputs.
       * @param source The output to unplug.
       */
      virtual void unplug(const Output* source);

      /**
       * @brief Removes connections to all outputs from a given Processor.
       * @param source The Processor to unplug.
       */
      virtual void unplug(const Processor* source);

      /**
       * @brief Called when the number of inputs changes (e.g., new connections).
       *        Subclasses may override for dynamic behavior.
       */
      virtual void numInputsChanged() { }

      /**
       * @brief Sets the ProcessorRouter that owns or manages this Processor.
       * @param router A pointer to the ProcessorRouter.
       */
      force_inline void router(ProcessorRouter* router) {
        router_ = router;
        VITAL_ASSERT((Processor*)router != this);
      }

      /**
       * @brief Returns the ProcessorRouter that currently owns this Processor.
       * @return The ProcessorRouter pointer.
       */
      force_inline ProcessorRouter* router() const {
        return router_;
      }

      /**
       * @brief Gets the topmost (root) ProcessorRouter by traversing parent routers.
       * @return A pointer to the top-level ProcessorRouter, or nullptr if none.
       */
      ProcessorRouter* getTopLevelRouter() const;

      /**
       * @brief Registers a new input, appending it to the input list.
       * @param input The Input pointer.
       * @param index Optional index to place the input at (overloaded).
       */
      virtual void registerInput(Input* input, int index);

      /**
       * @brief Registers a new Output in the output list at a specified index.
       * @param output The Output pointer.
       * @param index  The index to place it at.
       * @return The same Output pointer.
       */
      virtual Output* registerOutput(Output* output, int index);

      /**
       * @brief Registers a new Input by appending it to the end of the input list.
       * @param input The Input pointer.
       */
      virtual void registerInput(Input* input);

      /**
       * @brief Registers a new Output by appending it to the end of the output list.
       * @param output The Output pointer.
       * @return The same Output pointer.
       */
      virtual Output* registerOutput(Output* output);

      /**
       * @brief Returns the total number of Input pointers (owned or otherwise).
       */
      force_inline int numInputs() const {
        return static_cast<int>(inputs_->size());
      }

      /**
       * @brief Returns the total number of Output pointers (owned or otherwise).
       */
      force_inline int numOutputs() const {
        return static_cast<int>(outputs_->size());
      }

      /**
       * @brief Returns how many Input objects this Processor owns.
       */
      force_inline int numOwnedInputs() const {
        return static_cast<int>(owned_inputs_.size());
      }

      /**
       * @brief Returns how many Output objects this Processor owns.
       */
      force_inline int numOwnedOutputs() const {
        return static_cast<int>(owned_outputs_.size());
      }

      /**
       * @brief Retrieves the Input pointer at a given index.
       * @param index The index of the input.
       * @return A pointer to the Input at that index.
       */
      force_inline Input* input(unsigned int index = 0) const {
        VITAL_ASSERT(index < inputs_->size());
        return (*inputs_)[index];
      }

      /**
       * @brief Checks if the input source at a given index is polyphonic.
       * @param index The input index.
       * @return True if the source is owned by a polyphonic Processor.
       */
      force_inline bool isInputSourcePolyphonic(int index = 0) {
        return input(index)->source->owner && input(index)->source->owner->isPolyphonic();
      }

      /**
       * @brief Retrieves an owned Input pointer at a given index.
       * @param index The index of the owned input.
       * @return The owned Input pointer.
       */
      force_inline Input* ownedInput(unsigned int index = 0) const {
        VITAL_ASSERT(index < owned_inputs_.size());
        return owned_inputs_[index].get();
      }

      /**
       * @brief Retrieves the Output pointer at a given index.
       * @param index The index of the output.
       * @return A pointer to the Output at that index.
       */
      force_inline Output* output(unsigned int index = 0) const {
        VITAL_ASSERT(index < outputs_->size());
        return (*outputs_)[index];
      }

      /**
       * @brief Retrieves an owned Output pointer at a given index.
       * @param index The index of the owned output.
       * @return The owned Output pointer.
       */
      force_inline Output* ownedOutput(unsigned int index = 0) const {
        VITAL_ASSERT(index < owned_outputs_.size());
        return owned_outputs_[index].get();
      }

      /**
       * @brief Sets the position at which `plugNext` starts searching for an open input.
       * @param start The new starting index for plugNext.
       */
      void setPluggingStart(int start) {
        plugging_start_ = start;
      }

    protected:
      /**
       * @brief Creates and registers a new Output. Handles control rate vs. audio rate.
       * @param oversample Oversample factor for audio-rate outputs.
       * @return The newly created Output pointer.
       */
      Output* addOutput(int oversample = 1);

      /**
       * @brief Creates and registers a new Input, initially connected to null_source_.
       * @return The newly created Input pointer.
       */
      Input* addInput();

      std::shared_ptr<ProcessorState> state_;  ///< Shared state (sample rate, oversample, etc.)

      int plugging_start_; ///< The index at which `plugNext` starts searching for an unplugged input.

      std::vector<std::shared_ptr<Input>> owned_inputs_;   ///< Inputs owned by this Processor.
      std::vector<std::shared_ptr<Output>> owned_outputs_; ///< Outputs owned by this Processor.

      std::shared_ptr<std::vector<Input*>> inputs_;   ///< All inputs, owned or external.
      std::shared_ptr<std::vector<Output*>> outputs_; ///< All outputs, owned or external.

      ProcessorRouter* router_; ///< The ProcessorRouter that manages this Processor.

      static const Output null_source_; ///< A null (dummy) source used for unconnected inputs.

      JUCE_LEAK_DETECTOR(Processor)
  };
} // namespace vital
