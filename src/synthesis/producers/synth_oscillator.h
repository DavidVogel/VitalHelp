#pragma once

#include "spectral_morph.h"
#include "synth_constants.h"
#include "utils.h"
#include "wave_frame.h"
#include "wavetable.h"

namespace vital {

  class FourierTransform;
  class Wavetable;

  /**
   * @struct PhaseBuffer
   * @brief Holds a buffer of poly_int values for phase information.
   */
  struct PhaseBuffer {
    /// Phase buffer array
    poly_int buffer[kMaxBufferSize * kMaxOversample];
  };

  /**
   * @class RandomValues
   * @brief A singleton class that generates and stores random poly_float values.
   *
   * These values are used for randomized amplitude morphing, etc.
   */
  class RandomValues {
    public:
      /// Seed value for internal random generator
      static constexpr int kSeed = 0x4;

      /**
       * @brief Retrieve the singleton instance of RandomValues.
       * @return A pointer to the unique RandomValues instance.
       */
      static RandomValues* instance() {
        int size = (kRandomAmplitudeStages + 1) * (Wavetable::kNumHarmonics + 1) / poly_float::kSize;
        static RandomValues instance(size);
        return &instance;
      }

      /**
       * @brief Get the internal random data buffer.
       * @return A pointer to the random poly_float array.
       */
      const poly_float* buffer() { return data_.get(); }

    private:
      /**
       * @brief Constructs RandomValues by filling an internal buffer with random poly_float values.
       * @param num_poly_floats The number of poly_float items to create in the buffer.
       */
      RandomValues(int num_poly_floats) {
        data_ = std::make_unique<poly_float[]>(num_poly_floats);
        utils::RandomGenerator generator(-1.0f, 1.0f);
        generator.seed(kSeed);
        for (int i = 0; i < num_poly_floats; ++i)
          data_[i] = generator.polyNext();
      }

      /// Unique pointer holding the array of random poly_float values.
      std::unique_ptr<poly_float[]> data_;
  };

  /**
   * @class SynthOscillator
   * @brief A core oscillator processor that generates audio by reading wavetable data with various effects.
   *
   * This class handles unison, spectral morphing, distortion, FM/RM modulation, and more.
   * It inherits from Processor, allowing it to be part of Vital's modular audio graph.
   */
  class SynthOscillator : public Processor {
    public:

      /** @enum InputIndices
       *  @brief Enumeration of input indices used by SynthOscillator.
       */
      enum {
        kWaveFrame,                 ///< Waveform frame selection
        kMidiNote,                  ///< MIDI note (for pitch)
        kMidiTrack,                 ///< MIDI tracking toggle
        kSmoothlyInterpolate,       ///< Not used directly in this file, but part of the Vital architecture
        kTranspose,                 ///< Transpose amount
        kTransposeQuantize,         ///< Transpose quantize setting
        kTune,                      ///< Fine tune
        kAmplitude,                 ///< Output amplitude
        kPan,                       ///< Stereo panning
        kUnisonVoices,             ///< Number of unison voices
        kUnisonDetune,             ///< Unison detune amount
        kPhase,                     ///< Phase offset
        kDistortionPhase,           ///< Phase distortion offset
        kRandomPhase,               ///< Random phase amount
        kBlend,                     ///< Blend between center voice and detuned voices
        kStereoSpread,              ///< Stereo spread amount
        kStackStyle,                ///< Unison stack style
        kDetunePower,               ///< Detune power exponent
        kDetuneRange,               ///< Detune range
        kUnisonFrameSpread,         ///< Unison wavetable frame spread
        kUnisonDistortionSpread,    ///< Unison distortion spread
        kUnisonSpectralMorphSpread, ///< Unison spectral morph spread
        kSpectralMorphType,         ///< Spectral morph type
        kSpectralMorphAmount,       ///< Spectral morph amount
        kSpectralUnison,            ///< Toggle for spectral unison
        kDistortionType,            ///< Distortion type
        kDistortionAmount,          ///< Distortion amount
        kActiveVoices,             ///< Active voice mask
        kReset,                     ///< Manual reset (phase reset)
        kRetrigger,                 ///< Retrigger mask
        kNumInputs                  ///< Number of inputs
      };

      /** @enum OutputIndices
       *  @brief Enumeration of output indices used by SynthOscillator.
       */
      enum {
        kRaw,       ///< Unleveled or "raw" output
        kLevelled,  ///< Output after amplitude leveling/panning
        kNumOutputs ///< Number of outputs
      };

      /** @enum SpectralMorph
       *  @brief Types of spectral morph effects that can be applied to the wavetable.
       */
      enum SpectralMorph {
        kNoSpectralMorph, ///< No spectral morph effect
        kVocode,          ///< Vocode morph
        kFormScale,       ///< Formant scaling
        kHarmonicScale,   ///< Harmonic scaling
        kInharmonicScale, ///< Inharmonic scaling
        kSmear,           ///< Smear morph
        kRandomAmplitudes,///< Random amplitudes morph
        kLowPass,         ///< Lowpass morph
        kHighPass,        ///< Highpass morph
        kPhaseDisperse,   ///< Phase dispersion
        kShepardTone,     ///< Shepard tone morph
        kSkew,            ///< Skew morph
        kNumSpectralMorphTypes
      };

      /** @enum DistortionType
       *  @brief Types of distortion/waveshaping used by the oscillator.
       */
      enum DistortionType {
        kNone,            ///< No distortion
        kSync,            ///< Sync distortion
        kFormant,         ///< Formant shifting
        kQuantize,        ///< Quantization
        kBend,            ///< Bend distortion
        kSqueeze,         ///< Squeeze distortion
        kPulseWidth,      ///< Pulse width distortion
        kFmOscillatorA,   ///< FM using oscillator A
        kFmOscillatorB,   ///< FM using oscillator B
        kFmSample,        ///< FM using sample
        kRmOscillatorA,   ///< RM using oscillator A
        kRmOscillatorB,   ///< RM using oscillator B
        kRmSample,        ///< RM using sample
        kNumDistortionTypes
      };

      /** @enum UnisonStackType
       *  @brief Ways to stack unison voices for chord or harmonic effects.
       */
      enum UnisonStackType {
        kNormal,              ///< Standard unison
        kCenterDropOctave,    ///< Shift center voice down an octave
        kCenterDropOctave2,   ///< Shift center voice down two octaves
        kOctave,              ///< Alternate between the base pitch and +1 octave
        kOctave2,             ///< Alternate among the base pitch, +1 octave, +2 octaves
        kPowerChord,          ///< Power chord intervals
        kPowerChord2,         ///< Extended power chord intervals
        kMajorChord,          ///< Major chord intervals
        kMinorChord,          ///< Minor chord intervals
        kHarmonicSeries,      ///< Harmonic series
        kOddHarmonicSeries,   ///< Odd harmonic series
        kNumUnisonStackTypes
      };

      /**
       * @brief Checks if distortion type uses the first modulation oscillator.
       * @param type The distortion type.
       * @return True if distortion type uses the first oscillator for FM/RM.
       */
      static bool isFirstModulation(int type) {
        return type == kFmOscillatorA || type == kRmOscillatorA;
      }

      /**
       * @brief Checks if distortion type uses the second modulation oscillator.
       * @param type The distortion type.
       * @return True if distortion type uses the second oscillator for FM/RM.
       */
      static bool isSecondModulation(int type) {
        return type == kFmOscillatorB || type == kRmOscillatorB;
      }

      /// Maximum number of unison voices
      static constexpr int kMaxUnison = 16;
      /// Number of poly phases used per voice
      static constexpr int kPolyPhasePerVoice = kMaxUnison / poly_float::kSize;
      /// Number of poly phases total
      static constexpr int kNumPolyPhase = kMaxUnison / 2;
      /// Number of buffers to store waveforms, based on the number of poly phases
      static constexpr int kNumBuffers = kNumPolyPhase * poly_float::kSize;
      /// Size of spectral buffer for Fourier transforms
      static constexpr int kSpectralBufferSize = Wavetable::kWaveformSize * 2 / poly_float::kSize + poly_float::kSize;

      /// Precomputed multipliers used for stacking unison voices into intervals.
      static const mono_float kStackMultipliers[kNumUnisonStackTypes][kNumPolyPhase];

      /**
       * @struct VoiceBlock
       * @brief A helper struct for loading per-voice data during audio processing.
       */
      struct VoiceBlock {
        /**
         * @brief Default constructor. Initializes all values to a safe default.
         */
        VoiceBlock();

        /**
         * @brief Checks if the from_buffers and to_buffers are identical (i.e., no crossfade needed).
         * @return True if the memory contents are the same for from_buffers and to_buffers.
         */
        bool isStatic() const {
          return memcmp(from_buffers, to_buffers, poly_float::kSize * sizeof(mono_float*)) == 0;
        }

        int start_sample;        ///< Sample offset to start processing for this voice
        int end_sample;          ///< Sample offset to end processing for this voice
        int total_samples;       ///< Total samples processed for this chunk

        poly_int phase;          ///< Current oscillator phase
        poly_float phase_inc_mult;       ///< Current phase increment multiplier
        poly_float from_phase_inc_mult;  ///< Previous chunk's phase increment multiplier

        poly_mask shepard_double_mask; ///< Mask indicating voices that should shift by *2
        poly_mask shepard_half_mask;   ///< Mask indicating voices that should shift by /2

        poly_int distortion_phase;       ///< Current distortion phase
        poly_int last_distortion_phase;  ///< Previous chunk's distortion phase
        poly_float distortion;           ///< Current distortion amount
        poly_float last_distortion;      ///< Previous chunk's distortion amount

        int num_buffer_samples;           ///< Number of samples to fade between wave buffers
        poly_int current_buffer_sample;   ///< Keeps track of fade progress

        bool smoothing_enabled;           ///< Whether wave blending is smoothly interpolated
        SpectralMorph spectral_morph;     ///< The current spectral morph type
        const poly_float* modulation_buffer; ///< Buffer for FM/RM modulation values
        const poly_float* phase_inc_buffer;  ///< Phase increment buffer
        const poly_int* phase_buffer;        ///< Phase buffer to add to 'phase' each sample

        /// Buffers used for crossfading from one wave to another
        const mono_float* from_buffers[poly_float::kSize];
        const mono_float* to_buffers[poly_float::kSize];
      };

      /**
       * @brief Sets distortion values for an array of poly_float, handling unison spread if necessary.
       * @param distortion_type The type of distortion used.
       * @param values Pointer to the poly_float array to modify.
       * @param num_values The number of items in the array.
       * @param spread True if a unison spread is applied to these values.
       */
      static void setDistortionValues(DistortionType distortion_type,
                                      poly_float* values, int num_values, bool spread);

      /**
       * @brief Sets spectral morph values for an array of poly_float, handling unison spread if necessary.
       * @param spectral_morph The type of spectral morph to apply.
       * @param values Pointer to the poly_float array to modify.
       * @param num_values The number of items in the array.
       * @param spread True if a unison spread is applied to these values.
       */
      static void setSpectralMorphValues(SpectralMorph spectral_morph,
                                         poly_float* values, int num_values, bool spread);

      /**
       * @brief Applies a spectral morph operation (e.g., vocode, smear) directly on a buffer.
       * @param morph_type The spectral morph type.
       * @param morph_amount The amount of morphing to apply (0.0 - 1.0).
       * @param wavetable_data Pointer to the original wavetable data.
       * @param wavetable_index Which frame of the wavetable to process.
       * @param dest Pointer to the destination buffer for the result.
       * @param transform Pointer to the Fourier transform helper.
       */
      static void runSpectralMorph(SpectralMorph morph_type, float morph_amount,
                                   const Wavetable::WavetableData* wavetable_data,
                                   int wavetable_index, poly_float* dest, FourierTransform* transform);

      /**
       * @brief Adjusts phase for sync, formant, quantize, etc.
       * @param distortion_type Type of distortion.
       * @param phase Current phase to be distorted.
       * @param distortion_amount Amount of distortion to apply.
       * @param distortion_phase Additional phase offset used by certain distortion types.
       * @return Distorted phase value.
       */
      static vital::poly_int adjustPhase(DistortionType distortion_type, poly_int phase,
                                         poly_float distortion_amount, poly_int distortion_phase);

      /**
       * @brief Retrieves a window multiplier (for example, half-sin window in formant mode).
       * @param distortion_type The distortion type.
       * @param phase The original oscillator phase.
       * @param distorted_phase The phase after distortion.
       * @return The multiplier to apply to the final audio sample.
       */
      static vital::poly_float getPhaseWindow(DistortionType distortion_type, poly_int phase,
                                              poly_int distorted_phase);

      /**
       * @brief Performs linear interpolation on a single wave buffer.
       * @param buffer The buffer of mono_float samples.
       * @param indices The poly_int of indices to read from.
       * @return Interpolated poly_float sample.
       */
      static poly_float interpolate(const mono_float* buffer, const poly_int indices);

      /**
       * @brief Checks if a given distortion type uses a separate distortion_phase (kSync, kQuantize, etc).
       * @param distortion_type The distortion type.
       * @return True if uses distortion phase, otherwise false.
       */
      static bool usesDistortionPhase(DistortionType distortion_type);

      /**
       * @brief Constructs a SynthOscillator with the specified Wavetable.
       * @param wavetable A pointer to the wavetable used for generating wave data.
       */
      SynthOscillator(Wavetable* wavetable);

      /**
       * @brief Resets oscillator state with an offset sample count.
       * @param reset_mask The mask indicating which voices need resetting.
       * @param sample A poly_int sample offset for retrigger.
       */
      void reset(poly_mask reset_mask, poly_int sample);

      /**
       * @brief Resets oscillator phase and other variables for specified voices.
       * @param reset_mask The mask indicating which voices to reset.
       */
      void reset(poly_mask reset_mask) override;

      /**
       * @brief Sets the spectral morph values for internal morphing.
       * @param spectral_morph The type of spectral morph.
       */
      void setSpectralMorphValues(SpectralMorph spectral_morph);

      /**
       * @brief Sets the distortion values for internal waveshaping/fm/rm.
       * @param distortion_type The type of distortion to apply.
       */
      void setDistortionValues(DistortionType distortion_type);

      /**
       * @brief Processes the oscillator for a given number of samples, writing to the output buffers.
       * @param num_samples The number of audio samples to generate.
       */
      void process(int num_samples) override;

      /**
       * @brief Clones this oscillator, returning a new instance with the same parameters.
       * @return Pointer to a new SynthOscillator instance.
       */
      Processor* clone() const override { return new SynthOscillator(*this); }

      /**
       * @brief Assigns an oscillator Output pointer for FM/RM modulation (first mod oscillator).
       * @param oscillator A pointer to the Output that provides modulation data.
       */
      void setFirstOscillatorOutput(Output* oscillator) { first_mod_oscillator_ = oscillator; }

      /**
       * @brief Assigns an oscillator Output pointer for FM/RM modulation (second mod oscillator).
       * @param oscillator A pointer to the Output that provides modulation data.
       */
      void setSecondOscillatorOutput(Output* oscillator) { second_mod_oscillator_ = oscillator; }

      /**
       * @brief Assigns a sample Output pointer for FM/RM modulation using a sample.
       * @param sample A pointer to the Output that provides the sample data.
       */
      void setSampleOutput(Output* sample) { sample_ = sample; }

      /**
       * @brief Overrides the base Processor method to set oversampling. Ensures internal buffers are large enough.
       * @param oversample The new oversampling factor.
       */
      void setOversampleAmount(int oversample) override {
        Processor::setOversampleAmount(oversample);
        phase_inc_buffer_->ensureBufferSize(oversample * kMaxBufferSize);
      }

    private:
      /**
       * @brief Processes all oscillator voices using the specified phaseDistort and window function objects.
       *
       * This template is specialized for different types of distortions and windowing (e.g., syncPhase, fmPhase).
       * @param num_samples The total number of samples to process.
       * @param distortion_type The type of distortion in use.
       */
      template<poly_int(*phaseDistort)(poly_int, poly_float, poly_int, const poly_float*, int),
               poly_float(*window)(poly_int, poly_int, poly_float, const poly_float*, int)>
      void processOscillators(int num_samples, DistortionType distortion_type);

      /**
       * @brief Processes a chunk (subset) of audio samples once per wave buffer crossfade iteration.
       *
       * @tparam phaseDistort Function pointer to apply the chosen phase distortion.
       * @tparam window Function pointer to apply any amplitude windowing.
       * @param current_center_amplitude The amplitude for the 'center' or non-detuned voice.
       * @param current_detuned_amplitude The amplitude for detuned voices.
       */
      template<poly_int(*phaseDistort)(poly_int, poly_float, poly_int, const poly_float*, int),
               poly_float(*window)(poly_int, poly_int, poly_float, const poly_float*, int)>
      void processChunk(poly_float current_center_amplitude, poly_float current_detuned_amplitude);

      /**
       * @brief Final step of processing, blending stereo and leveling output amplitude.
       * @param num_samples Number of samples in this audio block.
       * @param reset_mask Mask indicating which voices were reset.
       */
      void processBlend(int num_samples, poly_mask reset_mask);

      /**
       * @brief Loads a VoiceBlock with the correct buffers and parameters for the specified index.
       * @param voice_block Reference to the VoiceBlock struct to fill.
       * @param index The index of the voice chunk to load.
       * @param active_mask Mask of active voices.
       */
      void loadVoiceBlock(VoiceBlock& voice_block, int index, poly_mask active_mask);

      /**
       * @brief Resets all wavetable-related buffers to a safe default (e.g., null waveforms).
       */
      void resetWavetableBuffers();

      /**
       * @brief Sets the active number of oscillators (2 * unison size). Initializes buffers if needed.
       * @param new_active_oscillators The new count of active oscillators (always even).
       */
      void setActiveOscillators(int new_active_oscillators);

      /**
       * @brief Internal helper to set the phase increment buffer, applying optional transpose snapping.
       * @tparam snapTranspose A function pointer controlling how transpose quantization is handled.
       * @param num_samples The number of samples to fill in the phase increment buffer.
       * @param reset_mask Mask indicating which voices are to be reset.
       * @param trigger_sample A poly_int offset for manual retriggering.
       * @param active_mask Mask of active voices.
       * @param snap_buffer A float array used for storing snap values if quantize is enabled.
       */
      template<poly_float(*snapTranspose)(poly_float, poly_float, float*)>
      void setPhaseIncBufferSnap(int num_samples, poly_mask reset_mask,
                                 poly_int trigger_sample, poly_mask active_mask, float* snap_buffer);

      /**
       * @brief Dispatch function that chooses the correct snapTranspose function for phase increment buffer.
       * @param num_samples The number of samples to process.
       * @param reset_mask Mask of voices that were reset.
       * @param trigger_sample A poly_int offset for manual retrigger.
       * @param active_mask Mask of active voices.
       */
      void setPhaseIncBuffer(int num_samples, poly_mask reset_mask,
                             poly_int trigger_sample, poly_mask active_mask);

      /**
       * @brief Calculates and sets unison pitch offsets (phase_inc_mult) based on detune, range, and stack style.
       */
      void setPhaseIncMults();

      /**
       * @brief If a Shepard tone morph is active, sets up masks for doubling/halving oscillator frequency.
       */
      void setupShepardWrap();

      /**
       * @brief Clears any internal masks for Shepard tone wrapping.
       */
      void clearShepardWrap();

      /**
       * @brief Performs actual doubling or halving of oscillator frequency if appropriate for Shepard tone transitions.
       * @param new_buffer_mask Mask of voices that need a wave buffer refresh.
       * @param quantize If non-zero, phase is quantized differently to avoid pitch discontinuities.
       */
      void doShepardWrap(poly_mask new_buffer_mask, int quantize);

      /**
       * @brief Computes overall amplitude of center vs. detuned voices to keep consistent volume with unison.
       */
      void setAmplitude();

      /**
       * @brief Prepares the wave buffers for crossfading or switching to a new set of wave data.
       * @param phase_inc The current phase increment for this voice pair.
       * @param index The offset index in the voice_block.
       */
      void setWaveBuffers(poly_float phase_inc, int index);

      /**
       * @brief Builds and transforms wave frames for Fourier-based spectral morphing.
       * @tparam spectralMorph Function pointer to the morph routine (e.g., smearMorph).
       * @param phase_update Current unison voice index.
       * @param index The offset index in the voice_block.
       * @param formant_shift True if additional formant shifting is required.
       * @param phase_adjustment A multiplier based on sample rate or other factors.
       * @param wave_index The frame index in the wavetable to retrieve data from.
       * @param voice_increment Additional frequency ratio per unison voice.
       * @param morph_amount The amount of spectral morph to apply.
       */
      template<void(*spectralMorph)(const Wavetable::WavetableData*, int, poly_float*,
                                    FourierTransform*, float, int, const poly_float*)>
      void computeSpectralWaveBufferPair(int phase_update, int index, bool formant_shift,
                                         float phase_adjustment, poly_int wave_index,
                                         poly_float voice_increment, poly_float morph_amount);

      /**
       * @brief Sets up wave_buffers_ for every unison voice, applying spectral morph.
       * @tparam spectralMorph A function pointer to the actual morph routine.
       * @param phase_inc The current phase increment for all voices.
       * @param index The offset index in the voice_block (0 for left, 2 for right, etc.).
       * @param formant_shift Whether we need formant frequency shifting.
       */
      template<void(*spectralMorph)(const Wavetable::WavetableData*, int, poly_float*,
                                    FourierTransform*, float, int, const poly_float*)>
      void setFourierWaveBuffers(poly_float phase_inc, int index, bool formant_shift);

      /**
       * @brief Blends stereo channels based on the current stereo spread parameter.
       * @param audio_out The array to apply stereo blending to.
       * @param num_samples The total samples in this block.
       * @param reset_mask Mask of voices that were reset.
       */
      void stereoBlend(poly_float* audio_out, int num_samples, poly_mask reset_mask);

      /**
       * @brief Applies final level scaling and panning to the raw audio output.
       * @param audio_out Output buffer (L+R) to be scaled.
       * @param raw_out Input buffer holding the pre-leveled audio.
       * @param num_samples Number of samples to process.
       * @param reset_mask Mask of voices that were reset.
       */
      void levelOutput(poly_float* audio_out, const poly_float* raw_out, int num_samples, poly_mask reset_mask);

      /**
       * @brief Converts from a single voice channel to stereo if fewer than 2 active voices exist.
       * @param num_samples Number of samples to process.
       * @param audio_out The buffer to transform in-place.
       * @param active_mask Mask indicating which voices are active.
       */
      void convertVoiceChannels(int num_samples, poly_float* audio_out, poly_mask active_mask);

      /**
       * @brief Computes a factor to scale phase increment for different sample rates.
       *        This ensures consistent pitch across sample rates.
       * @return A float multiplier to apply to the phase increment.
       */
      force_inline float getPhaseIncAdjustment() {
        static constexpr int kBaseSampleRate = 44100;

        float adjustment = 1.0f;
        int sample_rate_mult = getSampleRate() / kBaseSampleRate;
        while (sample_rate_mult > 1) {
          sample_rate_mult >>= 1;
          adjustment *= 2.0f;
        }
        return adjustment;
      }

      /// Storage of phases for all unison voices (kNumPolyPhase entries)
      poly_int phases_[kNumPolyPhase];
      /// Detuning multipliers for each unison voice
      poly_float detunings_[kNumPolyPhase];
      /// Phase increment multipliers for each unison voice
      poly_float phase_inc_mults_[kNumPolyPhase];
      /// Phase increment multipliers from previous chunk
      poly_float from_phase_inc_mults_[kNumPolyPhase];
      /// Shepard masks to indicate doubling oscillator frequency
      poly_int shepard_double_masks_[kNumPolyPhase];
      /// Shepard masks to indicate halving oscillator frequency
      poly_int shepard_half_masks_[kNumPolyPhase];
      /// Shepard masks waiting to be applied (double)
      poly_int waiting_shepard_double_masks_[kNumPolyPhase];
      /// Shepard masks waiting to be applied (half)
      poly_int waiting_shepard_half_masks_[kNumPolyPhase];

      /// Internal amplitude computations for center and detuned voices
      poly_float pan_amplitude_;
      poly_float center_amplitude_;
      poly_float detuned_amplitude_;
      poly_float midi_note_;
      poly_float distortion_phase_;
      poly_float blend_stereo_multiply_;
      poly_float blend_center_multiply_;

      /// Wavetable buffers for crossfading
      const mono_float* next_buffers_[kNumBuffers];
      const mono_float* wave_buffers_[kNumBuffers];
      const mono_float* last_buffers_[kNumBuffers];

      /// Storage for spectral morph and distortion amounts (per unison voice)
      poly_float spectral_morph_values_[kNumPolyPhase];
      poly_float last_spectral_morph_values_[kNumPolyPhase];
      poly_float distortion_values_[kNumPolyPhase];
      poly_float last_distortion_values_[kNumPolyPhase];

      /// Reusable VoiceBlock struct to avoid allocations in the audio thread
      VoiceBlock voice_block_;

      /// Random generator for initializing random phase, etc.
      utils::RandomGenerator random_generator_;

      /// Index for transpose quantization
      int transpose_quantize_;
      /// Last stored quantized transpose
      poly_float last_quantized_transpose_;
      /// Last quantize ratio
      poly_float last_quantize_ratio_;
      /// Number of unison voices
      int unison_;
      /// Number of currently active oscillators (2 * unison_).
      int active_oscillators_;

      /// Pointer to the main wavetable storing WaveFrame data
      Wavetable* wavetable_;
      /// Tracks changes in the wavetable to re-initialize buffers if needed
      int wavetable_version_;

      /// Modulator outputs for FM/RM
      Output* first_mod_oscillator_;
      Output* second_mod_oscillator_;
      Output* sample_;

      /// Internal buffers for Fourier transform frames
      poly_float fourier_frames1_[kNumBuffers + 1][kSpectralBufferSize];
      poly_float fourier_frames2_[kNumBuffers + 1][kSpectralBufferSize];

      /// Shared FourierTransform resource for spectral morph operations
      std::shared_ptr<FourierTransform> fourier_transform_;
      /// Shared pointer to an Output that holds the phase increment buffer
      std::shared_ptr<Output> phase_inc_buffer_;
      /// Shared pointer to a PhaseBuffer that stores phase offsets
      std::shared_ptr<PhaseBuffer> phase_buffer_;

      JUCE_LEAK_DETECTOR(SynthOscillator)
  };
} // namespace vital
