/**
* SPDX-License-Identifier: MIT
 * SPDX-SnippetCopyrightText: 2025 Arthur Moore <Arthur.Moore.git@cd-net.net>
 * @file
 * @copyright Arthur Moore <Arthur.Moore.git@cd-net.net> 2025
 */

#pragma once
#include <functional>

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

    /**
     * @brief Register the pin as an input, and a callback function
     * @details Also enable the built-in pull-up resistor for the pin.
     * @param callback Function to execute when the button is pressed.
     */
    void Setup(const std::function<void()>& callback);

    /**
     * @brief Handle button events.
     * @details Check the button state, and if it is pressed for debounce time,
     *          run the callback function once.
     */
    void Update();

private:
    const uint8_t pin;
    const unsigned long debounce;
    std::function<void()> callback = nullptr;
    volatile bool isPressed = false;
    volatile unsigned long pressedAt = -1;
    volatile bool callbackRan = false;
    void OnPress();
    void OnRelease();
};
