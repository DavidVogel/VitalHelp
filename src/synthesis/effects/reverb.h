#pragma once

#include "processor.h"
#include "one_pole_filter.h"

namespace vital {

  /**
   * @class Reverb
   * @brief A Processor implementing a dense feedback network reverb.
   *
   * The Reverb uses multiple all-pass filters and feedback delay lines to create
   * a spacious, reverberant sound. It can be controlled via decay time, cutoff
   * filters, chorus modulation, stereo width, and more.
   */
  class Reverb : public Processor {
    public:
      /**
       * @brief Amplitude at which we consider the reverb tail to effectively end (T60).
       */
      static constexpr mono_float kT60Amplitude = 0.001f;

      /**
       * @brief Feedback coefficient for all-pass filter sections.
       */
      static constexpr mono_float kAllpassFeedback = 0.6f;

      /**
       * @brief Minimum delay (in samples) used for certain time-domain operations.
       */
      static constexpr float kMinDelay = 3.0f;

      /**
       * @brief Reference sample rate for the base reverb time calculations.
       */
      static constexpr int kBaseSampleRate = 44100;

      /**
       * @brief Default sample rate used for internal buffer initialization.
       */
      static constexpr int kDefaultSampleRate = 88200;

      /**
       * @brief Number of feedback delay lines in the network.
       */
      static constexpr int kNetworkSize = 16;

      /**
       * @brief Base bits used for feedback buffer size calculations.
       */
      static constexpr int kBaseFeedbackBits = 14;

      /**
       * @brief Extra samples in the buffer to allow for interpolation overflow.
       */
      static constexpr int kExtraLookupSample = 4;

      /**
       * @brief Base bits used for the all-pass filters’ buffer size.
       */
      static constexpr int kBaseAllpassBits = 10;

      /**
       * @brief Number of poly_float-sized containers covering the entire network.
       */
      static constexpr int kNetworkContainers = kNetworkSize / poly_float::kSize;

      /**
       * @brief Minimum size exponent for reverb buffer scale.
       */
      static constexpr int kMinSizePower = -3;

      /**
       * @brief Maximum size exponent for reverb buffer scale.
       */
      static constexpr int kMaxSizePower = 1;

      /**
       * @brief The exponent range (max minus min).
       */
      static constexpr float kSizePowerRange = kMaxSizePower - kMinSizePower;

      /**
       * @brief Fixed all-pass filter delays for each container.
       */
      static const poly_int kAllpassDelays[kNetworkContainers];

      /**
       * @brief Fixed feedback delays (in samples) for each container.
       */
      static const poly_float kFeedbackDelays[kNetworkContainers];

      /**
       * @enum InputIndices
       * @brief Enumerates the inputs to the Reverb Processor.
       */
      enum {
        kAudio,          ///< Audio input buffer
        kDecayTime,      ///< Reverb decay time in seconds
        kPreLowCutoff,   ///< Pre-filter low cutoff (MIDI note)
        kPreHighCutoff,  ///< Pre-filter high cutoff (MIDI note)
        kLowCutoff,      ///< Internal feedback low cutoff (MIDI note)
        kLowGain,        ///< Low-frequency attenuation (dB)
        kHighCutoff,     ///< Internal feedback high cutoff (MIDI note)
        kHighGain,       ///< High-frequency attenuation (dB)
        kChorusAmount,   ///< Amount of chorusing applied to feedback lines
        kChorusFrequency,///< Frequency of the chorus LFO (Hz)
        kStereoWidth,    ///< Stereo width parameter (not used in all reverb modes)
        kSize,           ///< Overall size (scales buffer size exponent)
        kDelay,          ///< Additional pre-delay in samples
        kWet,            ///< Dry/wet mix
        kNumInputs       ///< Total number of inputs
      };

      /**
       * @brief Constructs a Reverb processor with default configuration.
       *
       * Allocates internal buffers and sets a default sample rate for initialization.
       */
      Reverb();

      /** @brief Default destructor. */
      virtual ~Reverb() { }

      /**
       * @brief Processes audio by pulling from the kAudio input buffer.
       *
       * @param num_samples Number of samples to process.
       */
      void process(int num_samples) override;

      /**
       * @brief Processes a block of audio using a provided input buffer.
       *
       * @param audio_in    Pointer to the input buffer.
       * @param num_samples Number of samples to process.
       */
      void processWithInput(const poly_float* audio_in, int num_samples) override;

      /**
       * @brief Returns the ratio of the current sample rate to kBaseSampleRate.
       *
       * @param sample_rate The current sample rate.
       * @return The ratio between current sample rate and the base sample rate.
       */
      force_inline float getSampleRateRatio(int sample_rate) { return sample_rate / (1.0f * kBaseSampleRate); }

      /**
       * @brief Computes a buffer scaling factor based on current sample rate.
       *
       * Doubling from kBaseSampleRate to keep the buffer sizes a power of two.
       *
       * @param sample_rate The current sample rate.
       * @return The integer scaling factor.
       */
      force_inline int getBufferScale(int sample_rate) {
        int scale = 1;
        float ratio = getSampleRateRatio(sample_rate);
        while (scale < ratio)
          scale *= 2;
        return scale;
      }

      /**
       * @brief Overrides base class to update reverb internal buffers at a new sample rate.
       *
       * @param sample_rate The new sample rate in Hz.
       */
      void setSampleRate(int sample_rate) override;

      /**
       * @brief Overrides base class to handle changes in oversampling factor.
       *
       * @param oversample_amount The new oversampling factor.
       */
      void setOversampleAmount(int oversample_amount) override;

      /**
       * @brief Adjusts internal buffer sizes and states for the given sample rate.
       *
       * @param sample_rate The current sample rate in Hz.
       */
      void setupBuffersForSampleRate(int sample_rate);

      /**
       * @brief Resets the reverb, clearing buffer contents and resetting filters.
       */
      void hardReset() override;

      /**
       * @brief Reads from the feedback delay line with polynomial interpolation.
       *
       * @param lookups Pointer array of pointers to delay line memory blocks.
       * @param offset  The fractional read offset (in samples).
       * @return Interpolated sample from the feedback delay line.
       */
      force_inline poly_float readFeedback(const mono_float* const* lookups, poly_float offset) {
        poly_float write_offset = poly_float(write_index_) - offset;
        poly_float floored_offset = utils::floor(write_offset);
        poly_float t = write_offset - floored_offset;
        matrix interpolation_matrix = utils::getPolynomialInterpolationMatrix(t);
        poly_int indices = utils::toInt(floored_offset) & feedback_mask_;
        matrix value_matrix = utils::getValueMatrix(lookups, indices);
        value_matrix.transpose();
        return interpolation_matrix.multiplyAndSumRows(value_matrix);
      }

      /**
       * @brief Reads from the all-pass filters using integer offsets.
       *
       * @param lookup Pointer to the all-pass delay line memory.
       * @param offset The integer offset into the delay line.
       * @return Sample from the all-pass buffer at that offset.
       */
      force_inline poly_float readAllpass(const mono_float* lookup, poly_int offset) {
        poly_int indices = (poly_int(write_index_ * poly_float::kSize) - offset) & allpass_mask_;
        return poly_float(lookup[indices[0]], lookup[indices[1]], lookup[indices[2]], lookup[indices[3]]);
      }

      /**
       * @brief Wraps the feedback buffer to preserve continuity for polynomial interpolation.
       *
       * Copies the first few samples to the end so wrapping indices read correct data.
       *
       * @param buffer Pointer to the feedback delay memory.
       */
      force_inline void wrapFeedbackBuffer(mono_float* buffer) {
        buffer[0] = buffer[max_feedback_size_];
        buffer[max_feedback_size_ + 1] = buffer[1];
        buffer[max_feedback_size_ + 2] = buffer[2];
        buffer[max_feedback_size_ + 3] = buffer[3];
      }

      /**
       * @brief Creates a clone of this Processor. (Not implemented for Reverb).
       * @return Returns nullptr.
       */
      virtual Processor* clone() const override { VITAL_ASSERT(false); return nullptr; }

    private:
      /**
       * @brief A memory buffer used for final reverb read/write (e.g., for cross-channel summing).
       */
      std::unique_ptr<StereoMemory> memory_;

      /**
       * @brief Pointers to memory blocks for the all-pass filters, sized by @c max_allpass_size_.
       */
      std::unique_ptr<poly_float[]> allpass_lookups_[kNetworkContainers];

      /**
       * @brief Buffers for the feedback comb filters, one per line in the reverb network.
       */
      std::unique_ptr<mono_float[]> feedback_memories_[kNetworkSize];

      /**
       * @brief Convenience pointers offset into @c feedback_memories_ for polynomial interpolation.
       */
      mono_float* feedback_lookups_[kNetworkSize];

      /**
       * @brief Cached decay multipliers for each container (computed from decay time, size, etc.).
       */
      poly_float decays_[kNetworkContainers];

      /**
       * @brief Low-shelf filters inside the feedback loop, one per container.
       */
      OnePoleFilter<> low_shelf_filters_[kNetworkContainers];

      /**
       * @brief High-shelf filters inside the feedback loop, one per container.
       */
      OnePoleFilter<> high_shelf_filters_[kNetworkContainers];

      /**
       * @brief One-pole filter for pre-delay low attenuation.
       */
      OnePoleFilter<> low_pre_filter_;

      /**
       * @brief One-pole filter for pre-delay high attenuation.
       */
      OnePoleFilter<> high_pre_filter_;

      /**
       * @brief Coefficient for the pre-delay low filter.
       */
      poly_float low_pre_coefficient_;

      /**
       * @brief Coefficient for the pre-delay high filter.
       */
      poly_float high_pre_coefficient_;

      /**
       * @brief Low-shelf filter coefficient in the feedback path.
       */
      poly_float low_coefficient_;

      /**
       * @brief Low-shelf attenuation factor (from low_gain).
       */
      poly_float low_amplitude_;

      /**
       * @brief High-shelf filter coefficient in the feedback path.
       */
      poly_float high_coefficient_;

      /**
       * @brief High-shelf attenuation factor (from high_gain).
       */
      poly_float high_amplitude_;

      /**
       * @brief Phase accumulator for chorus modulation (0..1).
       */
      mono_float chorus_phase_;

      /**
       * @brief The current chorus drift amount (in samples) added to certain feedback lines.
       */
      poly_float chorus_amount_;

      /**
       * @brief Current feedback multiplier for the entire network (unused in some designs).
       */
      poly_float feedback_;

      /**
       * @brief Current damping factor for controlling reverb tail decay (unused in some designs).
       */
      poly_float damping_;

      /**
       * @brief Current pre-delay in samples for the main memory buffer.
       */
      poly_float sample_delay_;

      /**
       * @brief Rate of change of the pre-delay for smoothing transitions.
       */
      poly_float sample_delay_increment_;

      /**
       * @brief Current dry signal multiplier (after equal-power fade).
       */
      poly_float dry_;

      /**
       * @brief Current wet signal multiplier (after equal-power fade).
       */
      poly_float wet_;

      /**
       * @brief Write index into the feedback buffers.
       */
      int write_index_;

      /**
       * @brief Maximum size of all-pass buffer.
       */
      int max_allpass_size_;

      /**
       * @brief Maximum size of feedback buffer.
       */
      int max_feedback_size_;

      /**
       * @brief Bitwise mask for feedback buffer indexing.
       */
      int feedback_mask_;

      /**
       * @brief Bitwise mask for all-pass buffer indexing in poly_float increments.
       */
      poly_mask allpass_mask_;

      /**
       * @brief Integer version of @c allpass_mask_ for indexing the all-pass buffers per voice.
       */
      int poly_allpass_mask_;

      JUCE_LEAK_DETECTOR(Reverb)
  };
} // namespace vital
