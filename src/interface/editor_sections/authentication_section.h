/// @file authentication_section.h
/// @brief Declares the AuthenticationSection class and related components for user authentication.

#pragma once

#if NDEBUG && !NO_AUTH

#include "JuceHeader.h"
#include "authentication.h"
#include "synth_button.h"
#include "overlay.h"
#include "open_gl_image_component.h"
#include "open_gl_multi_quad.h"

/// @class ForgotPasswordLink
/// @brief A clickable text component that redirects the user to a "forgot password" page when clicked.
class ForgotPasswordLink : public PlainTextComponent {
  public:
    /// Constructor.
    ForgotPasswordLink() : PlainTextComponent("Forgot password?", "Forgot password?") {
      setInterceptsMouseClicks(true, false);
    }

    /// Called when the mouse enters the component area. Changes the text color to indicate hover.
    /// @param e The mouse event.
    void mouseEnter(const MouseEvent& e) override {
      setColor(findColour(Skin::kWidgetAccent1, true).brighter(1.0f));
    }

    /// Called when the mouse leaves the component area. Restores the original text color.
    /// @param e The mouse event.
    void mouseExit(const MouseEvent& e) override {
      setColor(findColour(Skin::kWidgetAccent1, true));
    }

    /// Called when the component is clicked. Launches the browser to a "forgot password" page.
    /// @param e The mouse event.
    void mouseDown(const MouseEvent& e) override {
      URL url("");  // TODO: Insert correct URL if available.
      url.launchInDefaultBrowser();
    }
};

class AuthenticationSection; // Forward declaration.

/// @class AuthInitThread
/// @brief A background thread to handle authentication initialization without blocking the GUI.
class AuthInitThread : public Thread {
  public:
    /// Constructor.
    /// @param ref Pointer to the AuthenticationSection that will be initialized.
    AuthInitThread(AuthenticationSection* ref) : Thread("Vial Auth Init Thread"), ref_(ref) { }
    virtual ~AuthInitThread() { }

    /// Thread run method that initializes authentication.
    void run() override;

  private:
    AuthenticationSection* ref_; ///< Reference to the AuthenticationSection.
};

/// @class WorkOffline
/// @brief A clickable text component that allows the user to opt to work offline if authentication fails.
///
/// The user can click "Work offline" to skip authentication.
class WorkOffline : public PlainTextComponent {
  public:
    /// @class Listener
    /// @brief Interface for objects that need to respond when the user chooses to work offline.
    class Listener {
      public:
        virtual ~Listener() = default;

        /// Called when the user selects "Work offline".
        virtual void workOffline() = 0;
    };

    /// Constructor.
    WorkOffline() : PlainTextComponent("Work offline", "Work offline") {
      setInterceptsMouseClicks(true, false);
    }

    /// Called when the mouse enters the component area. Changes text color to indicate hover.
    /// @param e The mouse event.
    void mouseEnter(const MouseEvent& e) override {
      setColor(findColour(Skin::kWidgetAccent1, true).brighter(1.0f));
    }

    /// Called when the mouse leaves the component area. Restores the original text color.
    /// @param e The mouse event.
    void mouseExit(const MouseEvent& e) override {
      setColor(findColour(Skin::kWidgetAccent1, true));
    }

    /// Called when the component is clicked. Notifies listeners that the user wants to work offline.
    /// @param e The mouse event.
    void mouseDown(const MouseEvent& e) override {
      for (Listener* listener: listeners_)
        listener->workOffline();
    }

    /// Adds a listener to be notified when "Work offline" is chosen.
    /// @param listener The listener to add.
    void addListener(Listener* listener) {
      listeners_.push_back(listener);
    }

  private:
    std::vector<Listener*> listeners_; ///< Registered listeners.
};

/// @class AuthenticationSection
/// @brief An overlay component that handles user authentication via email and password.
///
/// This component shows input fields for email and password, a "Sign in" button,
/// and options to recover a forgotten password or work offline. Once authenticated, it can notify listeners.
class AuthenticationSection : public Overlay, public TextEditor::Listener, public WorkOffline::Listener, public Timer {
  public:
    /// Fixed dimensions and layout constants.
    static constexpr int kWidth = 450;
    static constexpr int kHeight = 398;
    static constexpr int kY = 180;
    static constexpr int kPadding = 20;
    static constexpr int kTextHeight = 36;
    static constexpr int kImageWidth = 128;

    /// @class Listener
    /// @brief Interface for objects that need to respond when a user successfully logs in.
    class Listener {
      public:
        virtual ~Listener() = default;

        /// Called when a user is successfully logged in.
        virtual void loggedIn() = 0;
    };

    /// Constructor.
    /// @param auth Pointer to the Authentication manager.
    AuthenticationSection(Authentication* auth);

    /// Destructor.
    virtual ~AuthenticationSection();

    /// Initializes the authentication process.
    void init();

    /// Creates the authentication object if not already created.
    void createAuth();

    /// Sets up the UI components and starts the authentication process.
    void create();

    /// Checks the current authentication state, logging in if already authenticated.
    void checkAuth();

    /// Gets the Authentication object.
    /// @return Pointer to the Authentication object.
    Authentication* auth() { return auth_; }

    /// Timer callback that periodically checks authentication status when needed.
    void timerCallback() override;

    /// Called when the mouse is released. Hides this overlay if clicked outside its bounds.
    /// @param e The mouse event.
    void mouseUp(const MouseEvent& e) override;

    /// Paints the background of the overlay. Overridden to do nothing here.
    /// @param g The graphics context.
    void paintBackground(Graphics& g) override { }

    /// Resizes and lays out child components.
    void resized() override;

    /// Sets visibility and, if visible, triggers background redraws.
    /// @param should_be_visible True to show, false to hide.
    void setVisible(bool should_be_visible) override;

    /// Called when visibility changes, used to refocus if needed.
    void visibilityChanged() override;

    /// Notifies registered listeners that the user is logged in.
    void notifyLoggedIn();

    /// TextEditor listener callback when return is pressed. Attempts login.
    /// @param editor The TextEditor.
    void textEditorReturnKeyPressed(TextEditor& editor) override {
      tryLogin();
    }

    /// Button listener callback. Also attempts login.
    /// @param clicked_button The button clicked.
    void buttonClicked(Button* clicked_button) override {
      tryLogin();
    }

    /// Gets the signed-in user's email.
    /// @return The signed-in email as a std::string.
    std::string getSignedInName() {
      return signed_in_email_;
    }

    /// Gets the email used for signing in.
    /// @return The email as a std::string.
    std::string getEmail() {
      return signed_in_email_;
    }

    /// Called if the user chooses to work offline.
    void workOffline() override;

    /// Signs the user out.
    void signOut();

    /// Sets keyboard focus on the email field if it's empty.
    void setFocus();

    /// Sets an error message to be displayed.
    /// @param error The error message.
    void setError(const std::string& error) { error_text_->setText(error); }

    /// Sets the sign-in button's enabled state and text.
    /// @param enabled True if enabled, false if disabled.
    /// @param text The button text.
    void setButtonSettings(bool enabled, const std::string& text) {
      sign_in_button_->setEnabled(enabled);
      sign_in_button_->setText(text);
    }

    /// Adds a listener for login events.
    /// @param listener The listener to add.
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /// Finishes the login process, hides the overlay, and notifies listeners.
    void finishLogin();

  private:
    /// Attempts to log in with the entered credentials.
    void tryLogin();

    Authentication* auth_;                         ///< The Authentication manager.
    std::vector<Listener*> listeners_;             ///< Registered listeners.

    std::string signed_in_email_;                  ///< The user's email after login.

    OpenGlQuad body_;                             ///< Background quad for the overlay.

    std::unique_ptr<AppLogo> logo_;               ///< Logo component.
    std::unique_ptr<PlainTextComponent> sign_in_text_;  ///< "Sign in" text label.
    std::unique_ptr<PlainTextComponent> error_text_;    ///< Error text display.
    std::unique_ptr<OpenGlTextEditor> email_;          ///< Email input field.
    std::unique_ptr<OpenGlTextEditor> password_;       ///< Password input field.
    std::unique_ptr<OpenGlToggleButton> sign_in_button_; ///< Sign-in button.
    std::unique_ptr<ForgotPasswordLink> forgot_password_; ///< Forgot password link.
    std::unique_ptr<WorkOffline> work_offline_;           ///< "Work offline" link.
    std::unique_ptr<AuthInitThread> auth_init_thread_;    ///< Background thread for auth initialization.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AuthenticationSection)
};

#else

/// A stubbed-out AuthenticationSection for builds without authentication.
class AuthenticationSection : public Component {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void loggedIn() = 0;
    };

    AuthenticationSection(Authentication* auth) { }

    std::string getSignedInName() { return ""; }
    void signOut() { }
    void create() { }
    void setFocus() { }
    void addListener(Listener* listener) { }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AuthenticationSection)
};

#endif
