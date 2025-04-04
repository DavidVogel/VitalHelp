#pragma once

#include "processor.h"
#include "linkwitz_riley_filter.h"

namespace vital {

  /**
   * @class Compressor
   * @brief A dynamic range compressor Processor that operates on a single band of audio.
   *
   * The Compressor class applies compression/expansion based on two thresholds (upper and lower)
   * and corresponding ratios. It tracks the input and output RMS levels and adjusts audio samples
   * in real time according to configured attack and release times.
   */
  class Compressor : public Processor {
    public:
      /**
       * @enum InputIndices
       * @brief Enumerates the input indices to the Compressor Processor.
       */
      enum {
        kAudio,           ///< Input audio signal
        kUpperThreshold,  ///< Upper threshold in dB
        kLowerThreshold,  ///< Lower threshold in dB
        kUpperRatio,      ///< Upper ratio (compression ratio above upper threshold)
        kLowerRatio,      ///< Lower ratio (expansion ratio below lower threshold)
        kOutputGain,      ///< Output gain (dB)
        kAttack,          ///< Attack time control (0.0 to 1.0 maps to exponential range)
        kRelease,         ///< Release time control (0.0 to 1.0 maps to exponential range)
        kMix,             ///< Dry/Wet mix
        kNumInputs        ///< Total number of inputs
      };

      /**
       * @enum OutputIndices
       * @brief Enumerates the output indices from the Compressor Processor.
       */
      enum {
        kAudioOut,  ///< Compressed audio output
        kNumOutputs ///< Total number of outputs
      };

      /**
       * @brief Constructs a Compressor Processor with given base attack and release times.
       *
       * @param base_attack_ms_first  Base attack time in ms for the first voice.
       * @param base_release_ms_first Base release time in ms for the first voice.
       * @param base_attack_ms_second Base attack time in ms for the second voice (polyphonic).
       * @param base_release_ms_second Base release time in ms for the second voice (polyphonic).
       */
      Compressor(mono_float base_attack_ms_first,
                 mono_float base_release_ms_first,
                 mono_float base_attack_ms_second,
                 mono_float base_release_ms_second);

      /**
       * @brief Default destructor.
       */
      virtual ~Compressor() { }

      /**
       * @brief Creates a clone of this Processor. (Not implemented for Compressor).
       *
       * @return Returns nullptr.
       */
      virtual Processor* clone() const override { VITAL_ASSERT(false); return nullptr; }

      /**
       * @brief Processes audio using the input audio buffer, modifying output buffer in-place.
       *
       * @param num_samples Number of samples to process.
       */
      virtual void process(int num_samples) override;

      /**
       * @brief Processes audio using the provided input buffer and writes to output.
       *
       * @param audio_in    Pointer to the input buffer.
       * @param num_samples Number of samples to process.
       */
      virtual void processWithInput(const poly_float* audio_in, int num_samples) override;

      /**
       * @brief Processes RMS for the input buffer and applies compression gain.
       *
       * @param audio_in    Pointer to the input buffer.
       * @param num_samples Number of samples to process.
       */
      void processRms(const poly_float* audio_in, int num_samples);

      /**
       * @brief Applies the final output scaling and dry/wet mix to the processed audio.
       *
       * @param audio_input Pointer to the unmodified input buffer (used for dry/wet blend).
       * @param num_samples Number of samples to process.
       */
      void scaleOutput(const poly_float* audio_input, int num_samples);

      /**
       * @brief Resets internal states and envelopes.
       *
       * @param reset_mask A poly_mask specifying which voices should be reset.
       */
      void reset(poly_mask reset_mask) override;

      /**
       * @brief Retrieves the current input RMS value (mean squared).
       *
       * @return The input mean squared level.
       */
      force_inline poly_float getInputMeanSquared() { return input_mean_squared_; }

      /**
       * @brief Retrieves the current output RMS value (mean squared).
       *
       * @return The output mean squared level.
       */
      force_inline poly_float getOutputMeanSquared() { return output_mean_squared_; }

    protected:
      /**
       * @brief Computes the mean squared value over a buffer of samples.
       *
       * @param audio_in    Pointer to the audio buffer.
       * @param num_samples Number of samples to process.
       * @param mean_squared The current rolling mean squared value to update.
       * @return Updated mean squared value.
       */
      poly_float computeMeanSquared(const poly_float* audio_in, int num_samples, poly_float mean_squared);

      /**
       * @brief Rolling mean squared value of the input signal.
       */
      poly_float input_mean_squared_;

      /**
       * @brief Rolling mean squared value of the output signal.
       */
      poly_float output_mean_squared_;

      /**
       * @brief Internal high enveloped mean squared value for upper threshold detection.
       */
      poly_float high_enveloped_mean_squared_;

      /**
       * @brief Internal low enveloped mean squared value for lower threshold detection.
       */
      poly_float low_enveloped_mean_squared_;

      /**
       * @brief The current dry/wet mix (0.0 = fully dry, 1.0 = fully wet).
       */
      poly_float mix_;

      /**
       * @brief Base attack time in ms for the current voice.
       */
      poly_float base_attack_ms_;

      /**
       * @brief Base release time in ms for the current voice.
       */
      poly_float base_release_ms_;

      /**
       * @brief Current multiplier for output gain (converted from dB).
       */
      poly_float output_mult_;

      JUCE_LEAK_DETECTOR(Compressor)
  };

  /**
   * @class MultibandCompressor
   * @brief A Processor implementing multiband compression using multiple Compressor instances.
   *
   * Splits the audio signal into up to three bands (low, band, high) via LinkwitzRiley filters,
   * and applies distinct compressors for each band. Consolidates the outputs for a final signal.
   */
  class MultibandCompressor : public Processor {
    public:
      /**
       * @enum InputIndices
       * @brief Enumerates the input indices to the MultibandCompressor Processor.
       */
      enum {
        kAudio,               ///< Input audio signal
        kLowUpperRatio,       ///< Upper ratio for low band
        kBandUpperRatio,      ///< Upper ratio for band
        kHighUpperRatio,      ///< Upper ratio for high band
        kLowLowerRatio,       ///< Lower ratio for low band
        kBandLowerRatio,      ///< Lower ratio for band
        kHighLowerRatio,      ///< Lower ratio for high band
        kLowUpperThreshold,   ///< Upper threshold (dB) for low band
        kBandUpperThreshold,  ///< Upper threshold (dB) for band
        kHighUpperThreshold,  ///< Upper threshold (dB) for high band
        kLowLowerThreshold,   ///< Lower threshold (dB) for low band
        kBandLowerThreshold,  ///< Lower threshold (dB) for band
        kHighLowerThreshold,  ///< Lower threshold (dB) for high band
        kLowOutputGain,       ///< Output gain (dB) for low band
        kBandOutputGain,      ///< Output gain (dB) for band
        kHighOutputGain,      ///< Output gain (dB) for high band
        kAttack,              ///< Global attack control
        kRelease,             ///< Global release control
        kEnabledBands,        ///< Enabled bands (see BandOptions)
        kMix,                 ///< Dry/wet mix for all bands
        kNumInputs            ///< Total number of inputs
      };

      /**
       * @enum BandOptions
       * @brief Identifies which subset of bands are active in the MultibandCompressor.
       */
      enum BandOptions {
        kMultiband,   ///< All three bands active
        kLowBand,     ///< Only low band active
        kHighBand,    ///< Only high band active
        kSingleBand,  ///< Single band (compressor passes audio directly)
        kNumBandOptions
      };

      /**
       * @enum OutputIndices
       * @brief Enumerates the output indices from the MultibandCompressor Processor.
       */
      enum OutputType {
        kAudioOut,               ///< Combined compressed output
        kLowInputMeanSquared,    ///< Low band input mean squared
        kBandInputMeanSquared,   ///< Band input mean squared
        kHighInputMeanSquared,   ///< High band input mean squared
        kLowOutputMeanSquared,   ///< Low band output mean squared
        kBandOutputMeanSquared,  ///< Band output mean squared
        kHighOutputMeanSquared,  ///< High band output mean squared
        kNumOutputs              ///< Total number of outputs
      };

      /**
       * @brief Constructs a MultibandCompressor, creating internal compressors and filters.
       */
      MultibandCompressor();

      /**
       * @brief Default destructor.
       */
      virtual ~MultibandCompressor() { }

      /**
       * @brief Creates a clone of this Processor. (Not implemented for MultibandCompressor).
       *
       * @return Returns nullptr.
       */
      virtual Processor* clone() const override { VITAL_ASSERT(false); return nullptr; }

      /**
       * @brief Processes audio using the input audio buffer.
       *
       * @param num_samples Number of samples to process.
       */
      virtual void process(int num_samples) override;

      /**
       * @brief Sets the amount of oversampling for the internal filters and compressors.
       *
       * @param oversample The oversampling factor.
       */
      void setOversampleAmount(int oversample) override;

      /**
       * @brief Processes audio using the given input buffer and writes to output.
       *
       * @param audio_in    Pointer to the input buffer.
       * @param num_samples Number of samples to process.
       */
      virtual void processWithInput(const poly_float* audio_in, int num_samples) override;

      /**
       * @brief Sets the current sample rate for the internal filters and compressors.
       *
       * @param sample_rate The new sample rate (in Hz).
       */
      void setSampleRate(int sample_rate) override;

      /**
       * @brief Resets internal states and filters.
       *
       * @param reset_mask A poly_mask specifying which voices should be reset.
       */
      void reset(poly_mask reset_mask) override;

    protected:
      /**
       * @brief Extracts the LinkwitzRileyFilter’s output into a combined buffer for further processing.
       *
       * @param filter      Pointer to a LinkwitzRileyFilter.
       * @param num_samples Number of samples to process.
       * @param dest        Buffer to which the filter output is written.
       */
      void packFilterOutput(LinkwitzRileyFilter* filter, int num_samples, poly_float* dest);

      /**
       * @brief Combines band filter outputs into a single buffer for the low band compressor.
       *
       * @param num_samples Number of samples to process.
       * @param dest        Buffer to write combined data.
       */
      void packLowBandCompressor(int num_samples, poly_float* dest);

      /**
       * @brief Writes the combined output of both compressors to a buffer.
       *
       * Used when both low and high bands are enabled.
       *
       * @param num_samples Number of samples to process.
       * @param dest        Buffer to write the combined data.
       */
      void writeAllCompressorOutputs(int num_samples, poly_float* dest);

      /**
       * @brief Writes a single compressor’s output to a buffer when only one band is active.
       *
       * @param compressor  Pointer to the compressor from which to retrieve data.
       * @param num_samples Number of samples to process.
       * @param dest        Buffer to write the data.
       */
      void writeCompressorOutputs(Compressor* compressor, int num_samples, poly_float* dest);

      /**
       * @brief Whether the low band was enabled on the previous process() call.
       */
      bool was_low_enabled_;

      /**
       * @brief Whether the high band was enabled on the previous process() call.
       */
      bool was_high_enabled_;

      /**
       * @brief Outputs for the low/band compressor thresholds and ratios.
       *
       * These are used to pass threshold/ratio values to the low band compressor
       * via the plug() system.
       */
      cr::Output low_band_upper_ratio_;
      cr::Output band_high_upper_ratio_;
      cr::Output low_band_lower_ratio_;
      cr::Output band_high_lower_ratio_;
      cr::Output low_band_upper_threshold_;
      cr::Output band_high_upper_threshold_;
      cr::Output low_band_lower_threshold_;
      cr::Output band_high_lower_threshold_;

      /**
       * @brief Gain controls for low and band/high compressors.
       */
      cr::Output low_band_output_gain_;
      cr::Output band_high_output_gain_;

      /**
       * @brief A Linkwitz-Riley filter splitting audio into low band and the rest (band + high).
       */
      LinkwitzRileyFilter low_band_filter_;

      /**
       * @brief A Linkwitz-Riley filter splitting audio into band and high bands.
       */
      LinkwitzRileyFilter band_high_filter_;

      /**
       * @brief Compressor handling the low band.
       */
      Compressor low_band_compressor_;

      /**
       * @brief Compressor handling band + high, or high only if configured.
       */
      Compressor band_high_compressor_;

      JUCE_LEAK_DETECTOR(MultibandCompressor)
  };
} // namespace vital
