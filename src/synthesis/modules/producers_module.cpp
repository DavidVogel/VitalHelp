#include "producers_module.h"

namespace vital {

    /**
     * @brief Constructs the ProducersModule by creating oscillators and a sample module.
     *
     * Oscillators and sampler are initially disabled until properly initialized and routed.
     */
    ProducersModule::ProducersModule() :
            SynthModule(kNumInputs, kNumOutputs), sample_destination_(nullptr),
            filter1_on_(nullptr), filter2_on_(nullptr) {
        // Create and configure oscillators.
        for (int i = 0; i < kNumOscillators; ++i) {
            std::string number = std::to_string(i + 1);
            oscillators_[i] = new OscillatorModule("osc_" + number);
            addSubmodule(oscillators_[i]);
            addProcessor(oscillators_[i]);
            oscillators_[i]->enable(false);
            oscillator_destinations_[i] = nullptr;
        }

        // Create and configure sampler.
        sampler_ = new SampleModule();
        addSubmodule(sampler_);
        addProcessor(sampler_);
        sampler_->enable(false);
    }

    /**
     * @brief Initializes the ProducersModule by setting up connections for oscillators and sampler.
     *
     * Connects inputs (reset, retrigger, MIDI, etc.) to oscillators and the sampler, and sets up their modulation relationships.
     */
    void ProducersModule::init() {
        for (int i = 0; i < kNumOscillators; ++i) {
            std::string number = std::to_string(i + 1);
            oscillator_destinations_[i] = createBaseControl("osc_" + number + "_destination");

            oscillators_[i]->useInput(input(kReset), OscillatorModule::kReset);
            oscillators_[i]->useInput(input(kRetrigger), OscillatorModule::kRetrigger);
            oscillators_[i]->useInput(input(kMidi), OscillatorModule::kMidi);
            oscillators_[i]->useInput(input(kActiveVoices), OscillatorModule::kActiveVoices);
        }

        sample_destination_ = createBaseControl("sample_destination");
        sampler_->useInput(input(kReset), SampleModule::kReset);
        sampler_->useInput(input(kNoteCount), SampleModule::kNoteCount);
        sampler_->useInput(input(kMidi), SampleModule::kMidi);

        SynthModule::init();

        // Set up modulation references between oscillators and the sampler output.
        for (int i = 0; i < kNumOscillators; ++i) {
            int index1 = getFirstModulationIndex(i);
            int index2 = getSecondModulationIndex(i);
            oscillators_[i]->oscillator()->setFirstOscillatorOutput(oscillators_[index1]->output(OscillatorModule::kRaw));
            oscillators_[i]->oscillator()->setSecondOscillatorOutput(oscillators_[index2]->output(OscillatorModule::kRaw));
            oscillators_[i]->oscillator()->setSampleOutput(sampler_->output(SampleModule::kRaw));
        }
    }

    /**
     * @brief Processes all oscillators and the sampler, mixing their outputs into different destinations.
     *
     * The processing order respects modulation dependencies. After processing, the outputs
     * are summed and routed to filters, direct output, or raw output buffers.
     *
     * @param num_samples The number of samples to process.
     */
    void ProducersModule::process(int num_samples) {
        // Base processing for this module.
        SynthModule::process(num_samples);

        // Process the sampler.
        getLocalProcessor(sampler_)->process(num_samples);

        // Track distortion types and processing states of oscillators to handle modulation dependencies.
        SynthOscillator::DistortionType distortion_types[kNumOscillators];
        bool processed[kNumOscillators];
        for (int i = 0; i < kNumOscillators; ++i) {
            distortion_types[i] = oscillators_[i]->getDistortionType();
            processed[i] = false;
        }

        // Process oscillators in an order that respects their modulation dependencies.
        int num_processed = 0;
        int index = 0;
        for (int i = 0; i < kNumOscillators * kNumOscillators && num_processed < kNumOscillators; ++i) {
            OscillatorModule* module = oscillators_[index];
            int first_source = getFirstModulationIndex(index);
            int second_source = getSecondModulationIndex(index);
            if ((!SynthOscillator::isFirstModulation(distortion_types[index]) || processed[first_source]) &&
                (!SynthOscillator::isSecondModulation(distortion_types[index]) || processed[second_source]) &&
                !processed[index]) {
                num_processed++;
                processed[index] = true;
                getLocalProcessor(module)->process(num_samples);
            }
            index = (index + 1) % kNumOscillators;
        }

        // Prepare the output buffers for mixing.
        poly_float* filter1_output = output(kToFilter1)->buffer;
        poly_float* filter2_output = output(kToFilter2)->buffer;
        poly_float* raw_output = output(kRawOut)->buffer;
        poly_float* direct_output = output(kDirectOut)->buffer;
        utils::zeroBuffer(filter1_output, num_samples);
        utils::zeroBuffer(filter2_output, num_samples);
        utils::zeroBuffer(raw_output, num_samples);
        utils::zeroBuffer(direct_output, num_samples);

        bool filter1_on = isFilter1On();
        bool filter2_on = isFilter2On();

        // Mix the oscillator outputs into their destinations based on their assigned routes.
        for (int i = 0; i < kNumOscillators; ++i) {
            const poly_float* buffer = oscillators_[i]->output(OscillatorModule::kLevelled)->buffer;

            int destination = oscillator_destinations_[i]->value();
            bool raw = destination == constants::kEffects;
            bool filter1 = (destination == constants::kFilter1 || destination == constants::kDualFilters);
            bool filter2 = (destination == constants::kFilter2 || destination == constants::kDualFilters);
            bool direct_out = (destination == constants::kDirectOut);

            // Handle routing logic based on filter states and destinations.
            if (raw || (!filter2 && filter1 && !filter1_on) ||
                (!filter1 && filter2 && !filter2_on) ||
                (filter1 && filter2 && !filter1_on && !filter2_on)) {
                utils::addBuffers(raw_output, raw_output, buffer, num_samples);
            }
            if (filter1)
                utils::addBuffers(filter1_output, filter1_output, buffer, num_samples);
            if (filter2)
                utils::addBuffers(filter2_output, filter2_output, buffer, num_samples);
            if (direct_out)
                utils::addBuffers(direct_output, direct_output, buffer, num_samples);
        }

        // Add the sampler output according to its destination.
        const poly_float* sample = sampler_->output(SampleModule::kLevelled)->buffer;

        int sample_destination = sample_destination_->value();
        bool sample_raw = sample_destination == constants::kEffects;
        bool filter1_sample = (sample_destination == constants::kFilter1 || sample_destination == constants::kDualFilters);
        bool filter2_sample = (sample_destination == constants::kFilter2 || sample_destination == constants::kDualFilters);
        bool sample_direct_out = (sample_destination == constants::kDirectOut);

        // Handle routing logic for the sampler output similar to oscillators.
        if (sample_raw || (!filter2_sample && filter1_sample && !filter1_on) ||
            (!filter1_sample && filter2_sample && !filter2_on) ||
            (filter1_sample && filter2_sample && !filter1_on && !filter2_on)) {
            utils::addBuffers(raw_output, raw_output, sample, num_samples);
        }
        if (filter1_sample)
            utils::addBuffers(filter1_output, filter1_output, sample, num_samples);
        if (filter2_sample)
            utils::addBuffers(filter2_output, filter2_output, sample, num_samples);
        if (sample_direct_out)
            utils::addBuffers(direct_output, direct_output, sample, num_samples);
    }

} // namespace vital
