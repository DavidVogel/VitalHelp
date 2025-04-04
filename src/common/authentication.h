#pragma once

#if NDEBUG && !NO_AUTH

#include "JuceHeader.h"
#include "load_save.h"

#if defined(__APPLE__)
#include <firebase/app.h>
#include <firebase/auth.h>
#else
#include "firebase/app.h"
#include "firebase/auth.h"
#endif

/**
 * @class Authentication
 * @brief Manages Firebase authentication for the Vital application.
 *
 * This class provides functionality to initialize Firebase authentication,
 * handle token retrieval, and check login status. It integrates with
 * Vital’s logging system to record errors and diagnostic information.
 *
 * Usage:
 * - Call Authentication::create() early to ensure that the Firebase app
 *   is initialized.
 * - Construct an Authentication instance and call init() to acquire an
 *   Auth handle.
 * - Once a user is logged in, refreshToken() can be called to asynchronously
 *   request a new authentication token.
 */
class Authentication {
public:
    /**
     * @brief Callback invoked when a token refresh request completes.
     *
     * This callback is triggered asynchronously after calling refreshToken(),
     * when the Firebase Future for token retrieval completes.
     *
     * @param completed_future A completed firebase::Future<std::string> holding the token or an error.
     * @param ref_data A pointer to an Authentication instance passed to OnCompletion().
     */
    static void onTokenRefreshResult(const firebase::Future<std::string>& completed_future, void* ref_data) {
      const MessageManagerLock lock(Thread::getCurrentThread());
      if (!lock.lockWasGained())
        return;

      if (completed_future.status() != firebase::kFutureStatusComplete) {
        LoadSave::writeErrorLog("Firebase getting token error: not complete");
        return;
      }

      if (completed_future.error()) {
        std::string error = "Firebase getting token error: error code ";
        LoadSave::writeErrorLog(error + std::to_string(completed_future.error()));
        return;
      }

      Authentication* reference = (Authentication*)ref_data;
      reference->setToken(*completed_future.result());
    }

    /**
     * @brief Creates a Firebase App instance if one does not already exist.
     *
     * This method must be called before interacting with Firebase authentication.
     * Subsequent calls have no effect if the App is already initialized.
     */
    static void create() {
      if (firebase::App::GetInstance() != nullptr)
        return;

      firebase::AppOptions auth_app_options = firebase::AppOptions();
      auth_app_options.set_app_id("");
      auth_app_options.set_api_key("");
      auth_app_options.set_project_id("");

      firebase::App::Create(auth_app_options);
    }

    /**
     * @brief Constructs an Authentication object.
     *
     * The Firebase Auth handle is not acquired here. Use init()
     * after create() has been called.
     */
    Authentication() : auth_(nullptr) { }

    /**
     * @brief Initializes the Firebase Auth object.
     *
     * Call this after create() to obtain a valid Auth handle.
     * If called multiple times, it only initializes Auth once.
     */
    void init() {
      if (auth_ == nullptr)
        auth_ = firebase::auth::Auth::GetAuth(firebase::App::GetInstance());
    }

    /**
     * @brief Checks if Firebase Auth is available.
     * @return True if Auth is initialized; False otherwise.
     */
    bool hasAuth() const { return auth_ != nullptr; }

    /**
     * @brief Returns a pointer to the internal firebase::auth::Auth object.
     * @return A pointer to the Auth instance, or nullptr if not initialized.
     */
    firebase::auth::Auth* auth() const { return auth_; }

    /**
     * @brief Sets the current authentication token.
     *
     * This method is typically called from onTokenRefreshResult() after
     * successfully retrieving a new token.
     *
     * @param token A string containing the authentication token.
     */
    void setToken(const std::string& token) { token_ = token; }

    /**
     * @brief Returns the most recently retrieved authentication token.
     * @return A string containing the token, or an empty string if none is set.
     */
    std::string token() const { return token_; }

    /**
     * @brief Checks if a user is currently logged in.
     * @return True if a current user is associated with Auth; False otherwise.
     */
    bool loggedIn() { return auth_ && auth_->current_user() != nullptr; }

    /**
     * @brief Initiates an asynchronous token refresh request.
     *
     * This method triggers an asynchronous call to the Firebase Auth SDK to
     * obtain a new token for the currently logged-in user. If successful,
     * onTokenRefreshResult() updates the stored token.
     */
    void refreshToken() {
      if (auth_ == nullptr || auth_->current_user() == nullptr)
        return;

      firebase::Future<std::string> future = auth_->current_user()->GetToken(false);
      future.OnCompletion(onTokenRefreshResult, this);
    }

private:
    firebase::auth::Auth* auth_;  ///< Pointer to the Firebase Auth instance.
    std::string token_;           ///< Stores the last retrieved authentication token.
};

#else

/**
 * @class Authentication
 * @brief A no-op stub implementation used when authentication is disabled.
 *
 * When built in debug mode or with NO_AUTH defined, this stub
 * provides empty implementations that do nothing.
 */
class Authentication {
public:
    /**
     * @brief No-op create method.
     */
    static void create() { }

    /**
     * @brief Returns an empty token string.
     * @return An empty string.
     */
    std::string token() { return ""; }

    /**
     * @brief Always returns false, indicating no user is logged in.
     * @return False.
     */
    bool loggedIn() { return false; }

    /**
     * @brief No-op token refresh method.
     */
    void refreshToken() { }
};

#endif
