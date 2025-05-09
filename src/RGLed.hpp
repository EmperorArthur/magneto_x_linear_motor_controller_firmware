/**
 * SPDX-License-Identifier: MIT
 * SPDX-SnippetCopyrightText: 2025 Arthur Moore <Arthur.Moore.git@cd-net.net>
 * @file
 * @copyright Arthur Moore <Arthur.Moore.git@cd-net.net> 2025
 */

#pragma once
#include <cstdint>

/**
 * @brief LED Which can be red or green.
 * @details Deliberately cannot be both red and green.
 */
class RGLed {
public:
    RGLed(uint8_t red_pin, uint8_t green_pin);

    /**
     * @brief Set the appropriate pins as output.
     * @details Also defaults to turning the LED off.
     */
    void Setup();

    void Off();
    /**
     * @brief Pure red.  Turns off green.
     */
    void SetRed();

    /**
     * @brief Pure green.  Turns off red.
     */
    void SetGreen();

    bool IsRed() const;
    bool IsGreen() const;
private:
    const uint8_t redPin;
    const uint8_t greenPin;
};
