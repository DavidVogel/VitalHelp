#include "synth_filter.h"

#include "comb_filter.h"
#include "digital_svf.h"
#include "diode_filter.h"
#include "dirty_filter.h"
#include "formant_filter.h"
#include "ladder_filter.h"
#include "phaser_filter.h"
#include "sallen_key_filter.h"

namespace vital {

  namespace {
    /**
     * @brief Maximum and minimum allowed drive gain (in dB).
     * @details Used to clamp the incoming drive values in loadSettings().
     */
    constexpr mono_float kMaxDriveGain = 36.0f;
    constexpr mono_float kMinDriveGain = 0.0f;
  } // namespace

  // Static initialization of the CoefficientLookup
  const SynthFilter::CoefficientLookup SynthFilter::coefficient_lookup_;

  /**
   * @brief Loads filter parameters from the Processor’s inputs into this FilterState.
   * @param processor The Processor to query for filter input values.
   */
  void SynthFilter::FilterState::loadSettings(Processor* processor) {
    // MIDI note-based cutoff
    midi_cutoff = processor->input(kMidiCutoff)->at(0);
    midi_cutoff_buffer = processor->input(kMidiCutoff)->source->buffer;

    // Resonance (0..1)
    resonance_percent = processor->input(kResonance)->at(0);

    // Drive gain in dB, clamped between 0 and 36
    poly_float input_drive = utils::clamp(processor->input(kDriveGain)->at(0), kMinDriveGain, kMaxDriveGain);
    drive_percent = (input_drive - kMinDriveGain) * (1.0f / (kMaxDriveGain - kMinDriveGain));
    drive = futils::dbToMagnitude(input_drive);

    // Additional overall gain
    gain = processor->input(kGain)->at(0);

    // Filter style enumerator (cast to int)
    style = processor->input(kStyle)->at(0)[0];

    // Pass blend in range [0..2]
    pass_blend = utils::clamp(processor->input(kPassBlend)->at(0), 0.0f, 2.0f);

    // XY interpolation parameters (for formants, morphing, etc.)
    interpolate_x = processor->input(kInterpolateX)->at(0);
    interpolate_y = processor->input(kInterpolateY)->at(0);

    // Transpose parameter in semitones
    transpose = processor->input(kTranspose)->at(0);
  }

  /**
   * @brief Factory method to create a SynthFilter object based on the given model enum.
   * @param model Filter model (e.g., kAnalog, kDiode, kFormant, etc.).
   * @return A pointer to the newly allocated filter instance, or nullptr for an invalid model.
   */
  SynthFilter* SynthFilter::createFilter(constants::FilterModel model) {
    switch (model) {
      case constants::kAnalog:
        return new SallenKeyFilter();
      case constants::kComb:
        return new CombFilter(1);
      case constants::kDigital:
        return new DigitalSvf();
      case constants::kDirty:
        return new DirtyFilter();
      case constants::kLadder:
        return new LadderFilter();
      case constants::kDiode:
        return new DiodeFilter();
      case constants::kFormant:
        return new FormantFilter(0);
      case constants::kPhase:
        return new PhaserFilter(false);
      default:
        return nullptr;
    }
  }

} // namespace vital
