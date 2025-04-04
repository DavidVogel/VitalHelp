#include "formant_filter.h"

#include "digital_svf.h"
#include "formant_manager.h"
#include "operators.h"
#include "synth_constants.h"

namespace vital {

  // Anonymous namespace for internal data structures and helper functions
  namespace {
    /**
     * @struct FormantValues
     * @brief Stores the gain, resonance, and MIDI cutoff for a single vowel/formant configuration.
     */
    struct FormantValues {
      cr::Value gain;        ///< Relative gain for this formant stage
      cr::Value resonance;   ///< Resonance factor for this formant
      cr::Value midi_cutoff; ///< MIDI note for the filter's cutoff
    };

    // Predefined formant values for vowels A, E, I, O, U
    // Each array has 4 sets of values for different "positions" used in interpolation.
    const FormantValues formant_a[kNumFormants] = {
      {cr::Value(-2),  cr::Value(0.66f), cr::Value(75.7552343327f)},
      {cr::Value(-8),  cr::Value(0.75f), cr::Value(84.5454706023f)},
      {cr::Value(-9),  cr::Value(1.0f),  cr::Value(100.08500317f)},
      {cr::Value(-10), cr::Value(1.0f),  cr::Value(101.645729657f)},
    };

    const FormantValues formant_e[kNumFormants] = {
      {cr::Value(0),   cr::Value(0.66f), cr::Value(67.349957715f)},
      {cr::Value(-14), cr::Value(0.75f), cr::Value(92.39951181f)},
      {cr::Value(-4),  cr::Value(1.0f),  cr::Value(99.7552343327f)},
      {cr::Value(-14), cr::Value(1.0f),  cr::Value(103.349957715f)},
    };

    const FormantValues formant_i[kNumFormants] = {
      {cr::Value(0),   cr::Value(0.8f),  cr::Value(61.7825925179f)},
      {cr::Value(-15), cr::Value(0.75f), cr::Value(94.049554095f)},
      {cr::Value(-17), cr::Value(1.0f),  cr::Value(101.03821678f)},
      {cr::Value(-20), cr::Value(1.0f),  cr::Value(103.618371471f)},
    };

    const FormantValues formant_o[kNumFormants] = {
      {cr::Value(-2),  cr::Value(0.7f),  cr::Value(67.349957715f)},
      {cr::Value(-6),  cr::Value(0.75f), cr::Value(79.349957715f)},
      {cr::Value(-14), cr::Value(1.0f),  cr::Value(99.7552343327f)},
      {cr::Value(-14), cr::Value(1.0f),  cr::Value(101.03821678f)},
    };

    const FormantValues formant_u[kNumFormants] = {
      {cr::Value(0),   cr::Value(0.7f),  cr::Value(65.0382167797f)},
      {cr::Value(-20), cr::Value(0.75f), cr::Value(74.3695077237f)},
      {cr::Value(-17), cr::Value(1.0f),  cr::Value(100.408607741f)},
      {cr::Value(-14), cr::Value(1.0f),  cr::Value(101.645729657f)},
    };

    /**
     * @enum FormantPosition
     * @brief Indices representing interpolation corners (bottom-left, bottom-right, top-left, top-right).
     */
    enum FormantPosition {
      kBottomLeft,
      kBottomRight,
      kTopLeft,
      kTopRight,
      kNumFormantPositions
    };

    // Two different style definitions, each referencing a set of vowels
    const FormantValues* formant_style1[kNumFormantPositions] = {
      formant_a, formant_o, formant_i, formant_e
    };

    const FormantValues* formant_style2[kNumFormantPositions] = {
      formant_a, formant_i, formant_u, formant_o
    };

    // Pointers to above style arrays
    const FormantValues** formant_styles[FormantFilter::kNumFormantStyles] = {
      formant_style2, // kAOIE → in code it's swapped, see usage
      formant_style1  // kAIUO
    };

    /**
     * @brief Performs bilinear interpolation on four values according to x and y fractions.
     * @param top_left Value at top-left corner.
     * @param top_right Value at top-right corner.
     * @param bot_left Value at bottom-left corner.
     * @param bot_right Value at bottom-right corner.
     * @param x Horizontal interpolation fraction (0.0 to 1.0).
     * @param y Vertical interpolation fraction (0.0 to 1.0).
     * @return The interpolated result.
     */
    poly_float bilinearInterpolate(poly_float top_left, poly_float top_right,
                                   poly_float bot_left, poly_float bot_right,
                                   poly_float x, poly_float y) {
      poly_float top = vital::utils::interpolate(top_left, top_right, x);
      poly_float bot = vital::utils::interpolate(bot_left, bot_right, x);
      return vital::utils::interpolate(bot, top, y);
    }

    /**
     * @brief Builds a FilterState by interpolating between four sets of formant values.
     * @param top_left The formant values at the top-left corner.
     * @param top_right The formant values at the top-right corner.
     * @param bot_left The formant values at the bottom-left corner.
     * @param bot_right The formant values at the bottom-right corner.
     * @param formant_x The horizontal interpolation fraction.
     * @param formant_y The vertical interpolation fraction.
     * @return A SynthFilter::FilterState containing the interpolated gain, resonance, and cutoff.
     */
    SynthFilter::FilterState interpolateFormants(const FormantValues& top_left,
                                                 const FormantValues& top_right,
                                                 const FormantValues& bot_left,
                                                 const FormantValues& bot_right,
                                                 poly_float formant_x, poly_float formant_y) {
      SynthFilter::FilterState filter_state;
      filter_state.midi_cutoff =
        bilinearInterpolate(top_left.midi_cutoff.value(),
                            top_right.midi_cutoff.value(),
                            bot_left.midi_cutoff.value(),
                            bot_right.midi_cutoff.value(),
                            formant_x, formant_y);

      filter_state.resonance_percent =
        bilinearInterpolate(top_left.resonance.value(),
                            top_right.resonance.value(),
                            bot_left.resonance.value(),
                            bot_right.resonance.value(),
                            formant_x, formant_y);

      filter_state.gain =
        bilinearInterpolate(top_left.gain.value(),
                            top_right.gain.value(),
                            bot_left.gain.value(),
                            bot_right.gain.value(),
                            formant_x, formant_y);

      return filter_state;
    }
  } // namespace

  /**
   * @brief FormantFilter constructor. Allocates a FormantManager and configures the chosen style.
   * @param style The initial style index (default is 0).
   */
  FormantFilter::FormantFilter(int style) :
      ProcessorRouter(kNumInputs, 1), formant_manager_(nullptr), style_(style) {
    formant_manager_ = new FormantManager(kNumFormants);
    addProcessor(formant_manager_);
  }

  /**
   * @brief Initializes the internal processing graph for formant filtering.
   *
   * Creates BilinearInterpolate, Interpolate, and other operator nodes (Add, Multiply) to
   * control each DigitalSvf that corresponds to a specific formant.
   */
  void FormantFilter::init() {
    static const cr::Value k12Db(DigitalSvf::k12Db);

    // Center value for Spread interpolation around kCenterMidi
    Value* center = new Value(kCenterMidi);
    addIdleProcessor(center);

    for (int i = 0; i < kNumFormants; ++i) {
      // Create interpolation nodes for gain, Q, and MIDI cutoff
      cr::BilinearInterpolate* formant_gain = new cr::BilinearInterpolate();
      cr::BilinearInterpolate* formant_q = new cr::BilinearInterpolate();
      BilinearInterpolate* formant_midi = new BilinearInterpolate();

      // Plug in the corner (top-left, top-right, bottom-left, bottom-right) values
      for (int p = 0; p < kNumFormantPositions; ++p) {
        formant_gain->plug(&formant_styles[style_][p][i].gain,
                           cr::BilinearInterpolate::kPositionStart + p);
        formant_q->plug(&formant_styles[style_][p][i].resonance,
                        cr::BilinearInterpolate::kPositionStart + p);
        formant_midi->plug(&formant_styles[style_][p][i].midi_cutoff,
                           BilinearInterpolate::kPositionStart + p);
      }

      // Connect X and Y interpolation inputs
      formant_gain->useInput(input(kInterpolateX), cr::BilinearInterpolate::kXPosition);
      formant_q->useInput(input(kInterpolateX), cr::BilinearInterpolate::kXPosition);
      formant_midi->useInput(input(kInterpolateX), BilinearInterpolate::kXPosition);

      formant_gain->useInput(input(kInterpolateY), cr::BilinearInterpolate::kYPosition);
      formant_q->useInput(input(kInterpolateY), cr::BilinearInterpolate::kYPosition);
      formant_midi->useInput(input(kInterpolateY), BilinearInterpolate::kYPosition);

      // Interpolate node for spreading the formant MIDI cutoff around a center
      Interpolate* formant_midi_spread = new Interpolate();
      formant_midi_spread->useInput(input(kSpread), Interpolate::kFractional);
      formant_midi_spread->useInput(input(kReset), Interpolate::kReset);
      formant_midi_spread->plug(center, Interpolate::kTo);
      formant_midi_spread->plug(formant_midi, Interpolate::kFrom);

      // Offset (transpose) the MIDI cutoff
      Add* formant_midi_adjust = new Add();
      formant_midi_adjust->useInput(input(kTranspose), 0);
      formant_midi_adjust->plug(formant_midi_spread, 1);

      // Multiply the resonance by an additional user-supplied resonance factor
      cr::Multiply* formant_q_adjust = new cr::Multiply();
      formant_q_adjust->useInput(input(kResonance), 0);
      formant_q_adjust->plug(formant_q, 1);

      // Connect the formant manager's DigitalSvf for this formant
      formant_manager_->getFormant(i)->useInput(input(kAudio), DigitalSvf::kAudio);
      formant_manager_->getFormant(i)->useInput(input(kReset), DigitalSvf::kReset);
      formant_manager_->getFormant(i)->plug(&k12Db, DigitalSvf::kStyle);
      formant_manager_->getFormant(i)->plug(&constants::kValueOne, DigitalSvf::kPassBlend);
      formant_manager_->getFormant(i)->plug(formant_gain, DigitalSvf::kGain);
      formant_manager_->getFormant(i)->plug(formant_q_adjust, DigitalSvf::kResonance);
      formant_manager_->getFormant(i)->plug(formant_midi_adjust, DigitalSvf::kMidiCutoff);

      // Add these processors to the chain
      addProcessor(formant_gain);
      addProcessor(formant_q);
      addProcessor(formant_q_adjust);
      addProcessor(formant_midi);
      addProcessor(formant_midi_spread);
      addProcessor(formant_midi_adjust);
    }

    // Finish initialization of the ProcessorRouter
    ProcessorRouter::init();
  }

  /**
   * @brief Configures the FormantFilter from a given FilterState, adjusting each DigitalSvf formant.
   * @param filter_state A FilterState with interpolation coordinates, style, resonance scaling, etc.
   */
  void FormantFilter::setupFilter(const FilterState& filter_state) {
    // Clamp the style to a valid index
    int style = std::min(filter_state.style, FormantFilter::kNumFormantStyles - 1);

    for (int i = 0; i < vital::kNumFormants; ++i) {
      // Interpolate the formant parameters from the style's corner values
      FilterState formant_setting =
        interpolateFormants(formant_styles[style][kTopLeft][i],
                            formant_styles[style][kTopRight][i],
                            formant_styles[style][kBottomLeft][i],
                            formant_styles[style][kBottomRight][i],
                            filter_state.interpolate_x,
                            filter_state.interpolate_y);

      // Blend between the formant's MIDI cutoff and the global center
      formant_setting.midi_cutoff = utils::interpolate(formant_setting.midi_cutoff,
                                                       kCenterMidi,
                                                       filter_state.pass_blend);

      // Add global transpose
      formant_setting.midi_cutoff += filter_state.transpose;

      // Scale resonance by global resonance_percent
      formant_setting.resonance_percent *= filter_state.resonance_percent;

      // Use 12dB style for each formant
      formant_setting.style = DigitalSvf::k12Db;
      formant_setting.pass_blend = 1.0f;

      // Apply to the underlying DigitalSvf
      vital::DigitalSvf* formant = formant_manager_->getFormant(i);
      formant->setupFilter(formant_setting);
    }
  }

  /**
   * @brief Resets the formant filter by calling reset on the FormantManager.
   * @param reset_mask A mask indicating which voices to reset.
   */
  void FormantFilter::reset(poly_mask reset_mask) {
    getLocalProcessor(formant_manager_)->reset(reset_mask);
  }

  /**
   * @brief Hard-resets the formant filter, resetting all internal states at once.
   */
  void FormantFilter::hardReset() {
    getLocalProcessor(formant_manager_)->hardReset();
  }

} // namespace vital
