/**
 * SPDX-License-Identifier: MIT
 * SPDX-SnippetCopyrightText: 2025 Arthur Moore <Arthur.Moore.git@cd-net.net>
 * @file
 * @copyright Arthur Moore <Arthur.Moore.git@cd-net.net> 2025
 */

#pragma once
#include <cstdint>

enum RGLedColor: uint8_t
{
    OFF = 0,
    RED = 1,
    GREEN = 2
};

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
    void begin();

    void setColor(RGLedColor color);

    /**
     * @warning This assumes the class has exclusive control of the pins.
     * @return The current color.
     */
    [[nodiscard]] RGLedColor getColor() const
    {
        return color;
    }

private:
    const uint8_t redPin;
    const uint8_t greenPin;
    RGLedColor color = OFF;
};
