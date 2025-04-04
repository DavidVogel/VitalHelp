/**
 * @file interface_test.cpp
 * @brief Implements the InterfaceTest class, providing methods to run stress tests on Vital's interface components.
 */

#include "interface_test.h"
#include "filter_section.h"
#include "default_look_and_feel.h"
#include "full_interface.h"
#include "preset_selector.h"
#include "sound_engine.h"
#include "synth_base.h"
#include "synth_gui_interface.h"
#include "synth_slider.h"
#include "skin.h"

namespace {
    /**
     * @brief Recursively finds all components of type T under the given component.
     * @tparam T The component type to search for.
     * @param component The root component to start the search.
     * @return A vector of pointers to components of type T found in the hierarchy.
     */
    template<class T>
    std::vector<T*> getAllComponentsOfType(Component* component) {
        std::vector<T*> results;
        Array<Component*> children = component->getChildren();
        for (Component* child : children) {
            T* result = dynamic_cast<T*>(child);
            if (result)
                results.push_back(result);
            else {
                std::vector<T*> sub_results = getAllComponentsOfType<T>(child);
                results.insert(results.end(), sub_results.begin(), sub_results.end());
            }
        }
        return results;
    }

    /**
     * @class TestFullInterface
     * @brief A subclass of FullInterface used for testing.
     *
     * Overrides resizing and painting to ensure that the background is redrawn
     * and child components are painted for testing.
     */
    class TestFullInterface : public FullInterface {
    public:
        TestFullInterface() { }

        void resized() override {
            SynthSection::resized();
            redoBackground();
        }

        void paintBackground(Graphics& g) override {
            paintChildrenBackgrounds(g);
        }
    };

    /**
     * @class TestTopComponent
     * @brief A top-level component that manages a FullInterface or a test SynthSection for random stress testing.
     *
     * It periodically changes slider values, toggles buttons, and cycles through presets
     * to stress test the interface components.
     */
    class TestTopComponent : public Component, public Timer {
    public:
        static constexpr float kMsBetweenUpdates = 10.0f;     ///< Interval between UI updates in ms.
        static constexpr float kSliderRatioChangesPerUpdate = 0.2f; ///< Fraction of sliders changed per update.
        static constexpr int kButtonTries = 32;               ///< Frequency factor for random button toggles.

        /**
         * @brief Constructs a TestTopComponent with a given FullInterface or a default one if none provided.
         * @param full_interface An optional FullInterface to manage.
         */
        TestTopComponent(FullInterface* full_interface) : Component("Test Top Component"), test_section_(nullptr) {
            if (full_interface == nullptr)
                full_interface_ = std::make_unique<TestFullInterface>();
            else {
                full_interface->reset();
                full_interface_.reset(full_interface);
            }
            addAndMakeVisible(full_interface_.get());
            startTimer(kMsBetweenUpdates);
        }

        /**
         * @brief Default constructor that uses a default FullInterface.
         */
        TestTopComponent() : TestTopComponent(nullptr) { }

        ~TestTopComponent() {
            stopTimer();
        }

        /**
         * @brief Gets the currently managed FullInterface.
         * @return A pointer to the FullInterface.
         */
        FullInterface* full_interface() {
            return full_interface_.get();
        }

        /**
         * @brief Adds a SynthSection to be tested by random stress interactions.
         * @param section A pointer to the SynthSection component.
         */
        void addTestSection(SynthSection* section) {
            test_section_ = section;

            if (full_interface_.get() != section)
                full_interface_->addSubSection(section);

            full_interface_->redoBackground();
            startTimer(kMsBetweenUpdates);
        }

        void resized() override {
            Component::resized();
            full_interface_->setBounds(getLocalBounds());
            startTimer(kMsBetweenUpdates);
        }

        /**
         * @brief Randomly changes values of some sliders in the test section.
         */
        void doSliderChanges() {
            auto sliders = test_section_->getAllSliders();

            if (sliders.size()) {
                int num_changes = std::ceil(kSliderRatioChangesPerUpdate * sliders.size());
                for (int i = 0; i < num_changes; ++i) {
                    int index = rand() % sliders.size();
                    auto iter = sliders.begin();
                    std::advance(iter, index);
                    SynthSlider* slider = iter->second;
                    if (!slider->isShowing())
                        continue;

                    float min = slider->getMinimum();
                    float max = slider->getMaximum();

                    int decision = rand() % 6;
                    if (decision == 0)
                        slider->setValue(min, NotificationType::sendNotification);
                    else if (decision == 1)
                        slider->setValue(max, NotificationType::sendNotification);
                    else
                        slider->setValue((rand() * (max - min)) / RAND_MAX + min, NotificationType::sendNotification);
                }
            }
        }

        /**
         * @brief Randomly toggles buttons in the test section.
         */
        void doButtonChanges() {
            auto buttons = getAllComponentsOfType<ToggleButton>(test_section_);

            int num_changes = static_cast<int>(buttons.size());
            for (int i = 0; i < num_changes; ++i) {
                Button* button = buttons[i];
                if (rand() % kButtonTries == 0 && button->isShowing())
                    button->setToggleState(!button->getToggleState(), NotificationType::sendNotification);
            }
        }

        /**
         * @brief Randomly changes presets through PresetSelectors in the test section.
         */
        void doPresetChanges() {
            auto preset_selectors = getAllComponentsOfType<PresetSelector>(test_section_);
            int num_changes = static_cast<int>(preset_selectors.size());
            for (int i = 0; i < num_changes; ++i) {
                PresetSelector* preset_selector = preset_selectors[i];
                if (rand() % kButtonTries == 0 && preset_selector->isShowing())
                    preset_selector->clickNext();
            }
        }

        void timerCallback() override {
            if (test_section_ == nullptr)
                return;

            doSliderChanges();
            doButtonChanges();
            doPresetChanges();

            PopupMenu::dismissAllActiveMenus();
        }

    private:
        std::unique_ptr<FullInterface> full_interface_; ///< The full interface under test.
        SynthSection* test_section_;                    ///< The test section component being randomly interacted with.
    };

    /**
     * @class TestAudioComponentBase
     * @brief An AudioAppComponent that uses the TestSynthBase for audio processing, providing a testing environment.
     */
    class TestAudioComponentBase : public AudioAppComponent {
    public:
        /**
         * @brief Constructs a TestAudioComponentBase with a given TestSynthBase and optional FullInterface.
         * @param synth_base A pointer to the TestSynthBase for audio processing.
         * @param full_interface An optional FullInterface to manage within this audio component.
         */
        TestAudioComponentBase(TestSynthBase* synth_base, FullInterface* full_interface = nullptr) :
                synth_base_(synth_base), top_component_(full_interface) {
            addAndMakeVisible(&top_component_);

            setAudioChannels(0, vital::kNumChannels);

            AudioDeviceManager::AudioDeviceSetup setup;
            deviceManager.getAudioDeviceSetup(setup);
            setup.sampleRate = vital::kDefaultSampleRate;
            deviceManager.initialise(0, vital::kNumChannels, nullptr, true, "", &setup);

            if (deviceManager.getCurrentAudioDevice() == nullptr) {
                const OwnedArray<AudioIODeviceType>& device_types = deviceManager.getAvailableDeviceTypes();

                for (AudioIODeviceType* device_type : device_types) {
                    deviceManager.setCurrentAudioDeviceType(device_type->getTypeName(), true);
                    if (deviceManager.getCurrentAudioDevice())
                        break;
                }
            }
        }

        ~TestAudioComponentBase() {
            shutdownAudio();
        }

        /**
         * @brief Sets initial size of the top component.
         */
        void setSizes() {
            top_component_.setSize(vital::kDefaultWindowWidth, vital::kDefaultWindowHeight);
        }

        /**
         * @brief Gets the top-level TestTopComponent.
         * @return A pointer to the TestTopComponent.
         */
        TestTopComponent* top_component() { return &top_component_; }

        void prepareToPlay(int buffer_size, double sample_rate) override {
            synth_base_->getEngine()->setSampleRate(sample_rate);
            synth_base_->getEngine()->updateAllModulationSwitches();
        }

        void getNextAudioBlock(const AudioSourceChannelInfo& buffer) override {
            int num_samples = buffer.buffer->getNumSamples();
            int synth_samples = std::min(num_samples, vital::kMaxBufferSize);

            MidiBuffer midi_messages;

            for (int b = 0; b < num_samples; b += synth_samples) {
                int current_samples = std::min<int>(synth_samples, num_samples - b);

                synth_base_->process(buffer.buffer, vital::kNumChannels, current_samples, b);
            }
        }

        void releaseResources() override {
        }

    private:
        TestSynthBase* synth_base_;      ///< The test synth base used for audio processing.
        TestTopComponent top_component_; ///< The top-level component for UI testing.
    };

    /**
     * @class TestWindow
     * @brief A DocumentWindow that hosts the full testing environment: an audio component and a UI, for a limited time.
     *
     * Runs tests for a specified duration, then requests a quit event.
     */
    class TestWindow : public DocumentWindow, public SynthGuiInterface, public Timer {
    public:
        static constexpr int kTestMs = 8000; ///< Duration of the test in milliseconds before closing.

        /**
         * @brief Constructs a TestWindow hosting UI and audio for testing.
         * @param synth_base The underlying TestSynthBase for the sound engine and GUI.
         * @param full_interface Optional FullInterface to be managed.
         */
        TestWindow(TestSynthBase* synth_base, FullInterface* full_interface = nullptr) :
                DocumentWindow("Interface Test", Colours::lightgrey, DocumentWindow::allButtons, true),
                SynthGuiInterface(synth_base, false) {
            synth_base->setGuiInterface(this);
            top_audio_component_ = std::make_unique<TestAudioComponentBase>(synth_base, full_interface);
            setUsingNativeTitleBar(true);
            setResizable(true, true);
            top_audio_component_->setSize(vital::kDefaultWindowWidth, vital::kDefaultWindowHeight);
            setContentOwned(top_audio_component_.get(), true);
            top_audio_component_->setSizes();
            setLookAndFeel(DefaultLookAndFeel::instance());
            startTimer(kTestMs);
        }

        /**
         * @brief Gets the top-level testing component.
         * @return A pointer to the TestTopComponent.
         */
        TestTopComponent* top_component() { return top_audio_component_->top_component(); }

        void closeButtonPressed() override {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        void timerCallback() override {
            closeButtonPressed();
        }

    private:
        std::unique_ptr<TestAudioComponentBase> top_audio_component_; ///< The audio component that also hosts the UI.
    };

    /**
     * @class TestApp
     * @brief A JUCEApplication instance created for the duration of the test.
     *
     * Manages the TestWindow's lifecycle and requests the message manager loop to stop after tests complete.
     */
    class TestApp : public JUCEApplication {
    public:
        TestApp(TestSynthBase* synth_base, FullInterface* full_interface = nullptr) {
            main_window_ = std::make_unique<TestWindow>(synth_base, full_interface);
            main_window_->centreWithSize(vital::kDefaultWindowWidth, vital::kDefaultWindowHeight);
            main_window_->setVisible(true);
        }

        const String getApplicationName() override { return ProjectInfo::projectName; }
        const String getApplicationVersion() override { return ProjectInfo::versionString; }
        bool moreThanOneInstanceAllowed() override { return true; }

        void initialise(const String& command_line) override {
        }

        void systemRequestedQuit() override {
            MessageManager::getInstance()->stopDispatchLoop();
            main_window_ = nullptr;
        }

        void shutdown() override {
            main_window_ = nullptr;
        }

        TestWindow* window() { return main_window_.get(); }

    private:
        std::unique_ptr<TestWindow> main_window_; ///< The main window hosting tests.
    };

    JUCEApplicationBase* createNullApplication() { return nullptr; }
}

void InterfaceTest::runStressRandomTest(SynthSection* component, FullInterface* full_interface) {
    beginTest("Stress Random Controls");
    MessageManager::getInstance();

    ScopedJuceInitialiser_GUI library_initializer;
    juce::JUCEApplicationBase::createInstance = &createNullApplication;

    if (synth_base_ == nullptr)
        createSynthEngine();

    vital::SoundEngine* engine = getSynthEngine();
    engine->noteOn(30, 0, 0, 0);
    engine->noteOn(37, 0, 0, 0);
    engine->noteOn(42, 0, 0, 0);

    // Create and run the test application which manages the test window and UI.
    TestApp test_app(synth_base_.get(), full_interface);
    test_app.window()->top_component()->addTestSection(component);
    component->setSize(vital::kDefaultWindowWidth, vital::kDefaultWindowHeight);
    test_app.window()->resized();
    vital::control_map controls = engine->getControls();
    test_app.window()->top_component()->full_interface()->setAllValues(controls);
    test_app.window()->top_component()->full_interface()->reset();

    JUCE_TRY {
            // Enter the message loop and run the test for the specified duration.
            MessageManager::getInstance()->runDispatchLoop();
    }
    JUCE_CATCH_EXCEPTION

    // Turn off the notes after the test.
    engine->noteOff(30, 0, 0, 0);
    engine->noteOff(37, 0, 0, 0);
    engine->noteOff(42, 0, 0, 0);
}
