#pragma once

#include "processor.h"
#include "futils.h"

namespace vital {

  /**
   * @class Distortion
   * @brief A Processor that applies various types of distortion to an audio signal.
   *
   * The Distortion class provides several distortion algorithms such as soft clipping,
   * hard clipping, bitcrushing, waveshaping (folding), and downsampling. Users can specify
   * the distortion type and drive amount, and the class will transform the audio accordingly.
   */
  class Distortion : public Processor {
    public:
      /**
       * @brief Maximum allowed drive in decibels.
       */
      static constexpr mono_float kMaxDrive = 30.0f;

      /**
       * @brief Minimum allowed drive in decibels.
       */
      static constexpr mono_float kMinDrive = -30.0f;

      /**
       * @brief Factor used to scale downsampling period relative to sample rate.
       */
      static constexpr float kPeriodScale = 1.0f / 88200.0f;

      /**
       * @brief Minimum distortion multiplier used for certain distortion styles (e.g., bitcrush).
       */
      static constexpr mono_float kMinDistortionMult = 32.0f / INT_MAX;

      /**
       * @enum InputIndices
       * @brief Enumerates the Distortion Processor input indices.
       */
      enum {
        kAudio, ///< Audio input buffer
        kType,  ///< Distortion type (see Type enum)
        kDrive, ///< Drive amount in dB
        kNumInputs
      };

      /**
       * @enum OutputIndices
       * @brief Enumerates the Distortion Processor output indices.
       */
      enum {
        kAudioOut, ///< Distorted audio output
        kDriveOut, ///< Drive values used in calculations (for reference or debugging)
        kNumOutputs
      };

      /**
       * @enum Type
       * @brief Distortion algorithms supported by this class.
       */
      enum Type {
        kSoftClip,    ///< Soft clipping (tanh-based waveshaping)
        kHardClip,    ///< Hard clipping
        kLinearFold,  ///< Linear waveform folding
        kSinFold,     ///< Sine-based waveform folding
        kBitCrush,    ///< Bitcrushing (quantizing samples)
        kDownSample,  ///< Downsampling
        kNumTypes
      };

      /**
       * @brief Scales a drive (dB) value into a linear multiplier for standard distortions.
       *
       * @param db The drive in decibels.
       * @return A linear magnitude multiplier based on the provided drive.
       */
      static force_inline poly_float driveDbScale(poly_float db) {
        return futils::dbToMagnitude(utils::clamp(db, kMinDrive, kMaxDrive));
      }

      /**
       * @brief Scales a drive (dB) value for bitcrush distortion (controls quantization level).
       *
       * @param db The drive in decibels.
       * @return A clamped multiplier for bitcrushing, between kMinDistortionMult and 1.0f.
       */
      static force_inline poly_float bitCrushScale(poly_float db) {
        constexpr mono_float kDriveScale = 1.0f / (Distortion::kMaxDrive - Distortion::kMinDrive);

        poly_float drive = utils::max(db - kMinDrive, 0.0f) * kDriveScale;
        return utils::clamp(drive * drive, kMinDistortionMult, 1.0f);
      }

      /**
       * @brief Scales a drive (dB) value for downsampling distortion.
       *
       * Inversely affects how often samples are updated (lower drive => more frequent updates).
       *
       * @param db The drive in decibels.
       * @return A scaled period factor for downsample-based distortion.
       */
      static force_inline poly_float downSampleScale(poly_float db) {
        constexpr mono_float kDriveScale = 1.0f / (Distortion::kMaxDrive - Distortion::kMinDrive);

        // Normalize drive to [0..1], then invert and square it
        poly_float drive = utils::max(db - Distortion::kMinDrive, 0.0f) * kDriveScale;
        drive = -drive + 1.0f;
        drive = poly_float(1.0f) / utils::clamp(drive * drive, Distortion::kMinDistortionMult, 1.0f);
        return utils::max(drive * 0.99f, 1.0f) * Distortion::kPeriodScale;
      }

      /**
       * @brief Converts an input drive in dB to a linear multiplier depending on distortion type.
       *
       * @param type         The distortion type (e.g., bitcrush, downsample, softclip).
       * @param input_drive  The raw drive in dB.
       * @return The scaled drive multiplier.
       */
      static poly_float getDriveValue(int type, poly_float input_drive) {
        if (type == kBitCrush)
          return bitCrushScale(input_drive);
        if (type == kDownSample)
          return downSampleScale(input_drive);
        return driveDbScale(input_drive);
      }

      /**
       * @brief Applies the specified distortion to a single sample given the drive multiplier.
       *
       * @param type  The distortion type.
       * @param value The input sample.
       * @param drive The scaled drive multiplier (or period) as appropriate for @p type.
       * @return The distorted sample.
       */
      static poly_float getDrivenValue(int type, poly_float value, poly_float drive);

      /**
       * @brief Constructs a Distortion object with the default number of inputs/outputs.
       */
      Distortion();

      /** @brief Default destructor. */
      virtual ~Distortion() { }

      /**
       * @brief Creates a clone of this Processor. (Not implemented for Distortion).
       * @return Returns nullptr.
       */
      virtual Processor* clone() const override {
        VITAL_ASSERT(false);
        return nullptr;
      }

      /**
       * @brief Processes a block of audio using the stored input buffer.
       *
       * @param num_samples Number of samples to process.
       */
      virtual void process(int num_samples) override;

      /**
       * @brief Processes a block of audio using a provided input buffer.
       *
       * @param audio_in    Pointer to the input buffer.
       * @param num_samples Number of samples to process.
       */
      virtual void processWithInput(const poly_float* audio_in, int num_samples) override;

      /**
       * @brief Processes samples with a time-invariant distortion function (no dynamic changes).
       *
       * A template that expects two function pointers: one for the distortion (`distort`)
       * and another for scaling the drive (`scale`).
       *
       * @tparam distort The function pointer for the distortion method.
       * @tparam scale   The function pointer for drive scaling.
       * @param num_samples Number of samples to process.
       * @param audio_in    Pointer to the input buffer.
       * @param drive       Pointer to the drive values.
       * @param audio_out   Pointer to the output buffer.
       */
      template<poly_float(*distort)(poly_float, poly_float), poly_float(*scale)(poly_float)>
      void processTimeInvariant(int num_samples, const poly_float* audio_in, const poly_float* drive,
                                poly_float* audio_out);

      /**
       * @brief Processes samples using a downsampling approach for distortion.
       *
       * @param num_samples Number of samples to process.
       * @param audio_in    Pointer to the input buffer.
       * @param drive       Pointer to the drive values (which translate to sample period).
       * @param audio_out   Pointer to the output buffer.
       */
      void processDownSample(int num_samples, const poly_float* audio_in, const poly_float* drive,
                             poly_float* audio_out);

    private:
      /**
       * @brief Stores the last computed output value for certain distortion types (e.g., downsampling).
       */
      poly_float last_distorted_value_;

      /**
       * @brief Accumulator tracking the time between updates in downsample-based distortion.
       */
      poly_float current_samples_;

      /**
       * @brief The currently active distortion type.
       */
      int type_;

      JUCE_LEAK_DETECTOR(Distortion)
  };
} // namespace vital
