/**
* SPDX-License-Identifier: MIT
 * SPDX-SnippetCopyrightText: 2025 Arthur Moore <Arthur.Moore.git@cd-net.net>
 * @file
 * @copyright Arthur Moore <Arthur.Moore.git@cd-net.net> 2025
 */

#pragma once
#include <functional>

enum ButtonState: bool
{
    RELEASED = false,
    PRESSED = true
};

/**
 * @brief Simple button handling code.
 * @details This deliberately does not use interrupts.
 *          Checks are only preformed when `Update()` is called.
 * @warning This is currently limited to buttons using `INPUT_PULLUP`.
 */
class Button
{
public:
    Button(uint8_t pin, unsigned long debounce);
    ~Button();
    Button(const Button&) = delete;
    Button(const Button&&) = delete;

    /**
     * @brief Register the pin as an input, and a callback function
     * @details Also enable the built-in pull-up resistor for the pin.
     * @param callback Function to execute when the button is pressed.
     */
    void begin(const std::function<void()>& callback);

    ///@brief Run the callback once, if button has been pressed for debounce time.
    void update();

    ///@brief Check if button has been pressed for debounce time.
    [[nodiscard]] ButtonState getState() const;

private:
    const uint8_t pin;
    const unsigned long debounce;
    std::function<void()> callback = nullptr;
    volatile bool isPressed = false;
    volatile unsigned long pressedAt = -1;
    volatile bool callbackRan = false;
    [[nodiscard]] inline bool _pressedForDebounceTimeInternal() const;

    // ISR Routines. Each is idempotent.
    ///@brief Read the pin and call either `onPress()` or `onRelease()`.
    static void _isr(void *buttonPtr);
    void onPress();
    void onRelease();
};
