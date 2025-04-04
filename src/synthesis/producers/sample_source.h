#pragma once

/**
 * @file sample_source.h
 * @brief Declares the Sample and SampleSource classes for loading, managing,
 *        and playing back audio samples in Vital.
 */

#include "processor.h"
#include "json/json.h"
#include "utils.h"

using json = nlohmann::json;

namespace vital {
  /**
   * @class Sample
   * @brief Holds and manages a single sampled waveform, including stereo or mono data
   *        and multiple band-limited versions for different playback rates.
   *
   * The Sample class can load samples from raw float buffers or from a stereo pair.
   * It also supports generating default content (e.g., noise) and can produce a JSON
   * representation for saving/loading states.
   */
  class Sample {
    public:
      /// Default length for a newly created (noise) sample if none is provided.
      static constexpr int kDefaultSampleLength = 44100;
      /// Upsampling factor exponent (i.e., 1 << kUpsampleTimes).
      static constexpr int kUpsampleTimes = 1;
      /// Extra buffer samples at start and end to avoid interpolation issues.
      static constexpr int kBufferSamples = 4;
      /// Minimum sample size for further downsampling.
      static constexpr int kMinSize = 4;

      /**
       * @struct SampleData
       * @brief Holds the actual audio buffers (left/right) for multiple band-limited versions
       *        (both looped and non-looped), and associated metadata like length and sample rate.
       */
      struct SampleData {
        /**
         * @brief Constructs a SampleData with basic metadata.
         * @param l  The sample length in frames.
         * @param sr The sample rate.
         * @param s  Whether this sample is stereo (true) or mono (false).
         */
        SampleData(int l, int sr, bool s) : length(l), sample_rate(sr), stereo(s) { }

        int length;      ///< Number of samples in the base (original) buffer.
        int sample_rate; ///< Original sample rate of the data.
        bool stereo;     ///< True if the sample is stereo, false if mono.

        /// Collection of band-limited upsample/downsample buffers for the left channel.
        std::vector<std::unique_ptr<mono_float[]>> left_buffers;
        /// Collection of band-limited upsample/downsample loop buffers for the left channel.
        std::vector<std::unique_ptr<mono_float[]>> left_loop_buffers;
        /// Collection of band-limited upsample/downsample buffers for the right channel (stereo only).
        std::vector<std::unique_ptr<mono_float[]>> right_buffers;
        /// Collection of band-limited upsample/downsample loop buffers for the right channel (stereo only).
        std::vector<std::unique_ptr<mono_float[]>> right_loop_buffers;

        JUCE_LEAK_DETECTOR(SampleData)
      };

      /**
       * @brief Default constructor. Initializes the sample with default noise data.
       */
      Sample();

      /**
       * @brief Loads a mono sample from raw float data.
       * @param buffer      Pointer to the float array containing sample data.
       * @param size        Number of frames in the buffer.
       * @param sample_rate The sample rate of the data.
       */
      void loadSample(const mono_float* buffer, int size, int sample_rate);

      /**
       * @brief Loads a stereo sample from two float arrays (left/right).
       * @param left_buffer  Pointer to float array for the left channel.
       * @param right_buffer Pointer to float array for the right channel.
       * @param size         Number of frames in each buffer.
       * @param sample_rate  The sample rate of the data.
       */
      void loadSample(const mono_float* left_buffer, const mono_float* right_buffer, int size, int sample_rate);

      /// Sets the user-facing name of the sample.
      void setName(const std::string& name) { name_ = name; }
      /// Returns the user-facing name of the sample.
      std::string getName() const { return name_; }

      /// Sets the last browsed file path (if applicable).
      void setLastBrowsedFile(const std::string& path) { last_browsed_file_ = path; }
      /// Returns the last browsed file path.
      std::string getLastBrowsedFile() const { return last_browsed_file_; }

      /// Returns the length of the originally loaded sample in frames.
      force_inline int originalLength() const { return current_data_->length; }

      /// Returns the length of the upsampled data (1 << kUpsampleTimes).
      force_inline int upsampleLength() { return originalLength() * (1 << kUpsampleTimes); }

      /// Returns the sample rate of the originally loaded data.
      force_inline int sampleRate() const { return current_data_->sample_rate; }

      /**
       * @brief Returns the active (currently used) sample length in frames,
       *        taking into account upsample factor and possibly another data set.
       */
      force_inline int activeLength() const { return active_audio_data_.load()->length * (1 << kUpsampleTimes); }

      /// Returns the sample rate of the currently active sample data.
      force_inline int activeSampleRate() const { return active_audio_data_.load()->sample_rate; }

      /**
       * @brief Returns a pointer to the (current) left channel buffer at the base upsample level.
       *
       * This is primarily for quick access to the default buffer, offset by kBufferSamples.
       */
      force_inline const mono_float* buffer() const { return current_data_->left_buffers[kUpsampleTimes].get() + 1; }

      /**
       * @brief Generates default data for the sample (e.g., random noise).
       */
      void init();

      /**
       * @brief Determines which band-limited buffer index should be used for a given pitch delta.
       * @param delta Frequency ratio or pitch factor to find appropriate band-limited buffer.
       * @return The index within the active_audio_data_ arrays.
       */
      int getActiveIndex(mono_float delta) {
        int octaves = utils::ilog2(std::max<int>(delta, 1));
        return std::min(octaves, (int)active_audio_data_.load()->left_buffers.size() - 1);
      }

      /**
       * @brief Retrieves a pointer to the active left channel buffer at a specific band-limited index.
       * @param index The band-limited index, computed from getActiveIndex().
       * @return A pointer to the float array for that band-limited buffer.
       */
      force_inline const mono_float* getActiveLeftBuffer(int index) {
        VITAL_ASSERT(index >= 0 && index < active_audio_data_.load()->left_buffers.size());

        return active_audio_data_.load()->left_buffers[index].get();
      }

      /**
       * @brief Retrieves a pointer to the active left loop buffer at a specific band-limited index.
       */
      force_inline const mono_float* getActiveLeftLoopBuffer(int index) {
        VITAL_ASSERT(index >= 0 && index < active_audio_data_.load()->left_loop_buffers.size());

        return active_audio_data_.load()->left_loop_buffers[index].get();
      }

      /**
       * @brief Retrieves a pointer to the active right channel buffer at a band-limited index.
       *        If the sample is mono, it returns the left buffer.
       */
      force_inline const mono_float* getActiveRightBuffer(int index) {
        if (active_audio_data_.load()->stereo) {
          VITAL_ASSERT(index >= 0 && index < active_audio_data_.load()->right_buffers.size());
          return active_audio_data_.load()->right_buffers[index].get();
        }
        return getActiveLeftBuffer(index);
      }

      /**
       * @brief Retrieves a pointer to the active right loop buffer at a band-limited index.
       *        If the sample is mono, returns the left loop buffer.
       */
      force_inline const mono_float* getActiveRightLoopBuffer(int index) {
        if (active_audio_data_.load()->stereo) {
          VITAL_ASSERT(index >= 0 && index < active_audio_data_.load()->right_loop_buffers.size());
          return active_audio_data_.load()->right_loop_buffers[index].get();
        }
        return getActiveLeftLoopBuffer(index);
      }

      /// Marks this sample as "in use" by updating the active_audio_data_ pointer.
      force_inline void markUsed() { active_audio_data_ = current_data_; }
      /// Marks this sample as "not in use," clearing the active_audio_data_ pointer.
      force_inline void markUnused() { active_audio_data_ = nullptr; }

      /**
       * @brief Exports the sample state (metadata and sample data) to a JSON object.
       * @return A JSON object containing the sample's state.
       */
      json stateToJson();

      /**
       * @brief Restores the sample's state from a JSON object (including audio data).
       * @param data A JSON object containing sample data.
       */
      void jsonToState(json data);

    protected:
      std::string name_;              ///< The user-facing name of the sample.
      std::string last_browsed_file_; ///< The last browsed file path for this sample (if any).
      SampleData* current_data_;      ///< Pointer to the currently loaded data.
      std::atomic<SampleData*> active_audio_data_; ///< Atomic pointer to data in active use.
      std::unique_ptr<SampleData> data_; ///< Owned sample data for this sample.

      JUCE_LEAK_DETECTOR(Sample)
  };

  /**
   * @class SampleSource
   * @brief A Processor that reads from a Sample object, providing audio output
   *        with controls for looping, pitch transposition, and panning.
   */
  class SampleSource : public Processor {
    public:
      /// Maximum positive transposition in semitones.
      static constexpr mono_float kMaxTranspose = 96.0f;
      /// Minimum negative transposition in semitones.
      static constexpr mono_float kMinTranspose = -96.0f;
      /// Maximum amplitude scale (usually sqrt(2) for stereo).
      static constexpr mono_float kMaxAmplitude = 1.41421356237f;

      /// Number of taps used in the downsampling filter.
      static constexpr int kNumDownsampleTaps = 55;
      /// Number of taps used in the upsampling filter.
      static constexpr int kNumUpsampleTaps = 52;

      /**
       * @enum
       * @brief Indices for input parameters in this Processor.
       */
      enum {
        kReset,             ///< Reset signal (trigger) to re-initialize playback.
        kMidi,              ///< MIDI note input.
        kKeytrack,          ///< Boolean-like input indicating if MIDI note should track pitch.
        kLevel,             ///< Overall amplitude scale.
        kRandomPhase,       ///< If true, randomize phase on note start.
        kTranspose,         ///< Transposition in semitones from the current note.
        kTransposeQuantize, ///< Quantize transposition to scale or semitones.
        kTune,              ///< Fine-tune in cents.
        kLoop,              ///< If non-zero, the sample loops.
        kBounce,            ///< If non-zero, sample playback bounces (back/forth).
        kPan,               ///< Stereo panning control.
        kNoteCount,         ///< Tracks how many notes have been pressed.
        kNumInputs
      };

      /**
       * @enum
       * @brief Indices for output signals from this Processor.
       */
      enum {
        kRaw,       ///< The raw sample output (before final amplitude).
        kLevelled,  ///< The amplitude-scaled output.
        kNumOutputs
      };

      /**
       * @brief Default constructor initializes internal state and random generator.
       */
      SampleSource();

      /**
       * @brief Processes audio for `num_samples`, reading from the Sample and applying pitch, loop, etc.
       * @param num_samples Number of samples to process.
       */
      virtual void process(int num_samples) override;

      /**
       * @brief Clones the SampleSource (copy constructor).
       * @return A new allocated SampleSource pointer.
       */
      virtual Processor* clone() const override { return new SampleSource(*this); }

      /**
       * @brief Provides access to the owned Sample object.
       * @return A pointer to the Sample owned by this SampleSource.
       */
      Sample* getSample() { return sample_.get(); }

      /**
       * @brief Retrieves an additional output that reflects the playback phase (in [0..1]) for debugging or usage.
       * @return A pointer to a control-rate Output that stores the current playback phase.
       */
      force_inline Output* getPhaseOutput() const { return phase_output_.get(); }

    private:
      /**
       * @brief Applies optional snapping (quantization) to the transpose based on input parameters.
       * @param input_midi   The MIDI note, if keytracking is enabled.
       * @param transpose    The raw transpose input in semitones.
       * @param quantize     Non-zero if transposition should be quantized.
       * @return The adjusted transpose value (possibly quantized).
       */
      poly_float snapTranspose(poly_float input_midi, poly_float transpose, int quantize);

      poly_float pan_amplitude_;       ///< Current panning amplitude factor (left/right).
      int transpose_quantize_;         ///< Cached quantize mode to detect changes.
      poly_float last_quantized_transpose_; ///< Remembers last snapped transposition for continuity.

      poly_float sample_index_;   ///< Integer part of the sample playback position (poly_float).
      poly_float sample_fraction_;///< Fractional part of the sample playback position (poly_float).
      poly_float phase_inc_;      ///< The increment (per-sample) for the playback position.

      poly_mask bounce_mask_;     ///< Mask to indicate which voices are in bounce (reverse) mode.

      std::shared_ptr<cr::Output> phase_output_; ///< Outputs the normalized playback phase in [0..1].
      utils::RandomGenerator random_generator_;  ///< For randomizing initial phase if requested.

      std::shared_ptr<Sample> sample_;           ///< The Sample object containing audio data.

      JUCE_LEAK_DETECTOR(SampleSource)
  };
} // namespace vital

