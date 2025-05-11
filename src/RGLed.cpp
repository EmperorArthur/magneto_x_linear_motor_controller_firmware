/**
 * SPDX-License-Identifier: MIT
 * SPDX-SnippetCopyrightText: 2025 Arthur Moore <Arthur.Moore.git@cd-net.net>
 * @file
 * @copyright Arthur Moore <Arthur.Moore.git@cd-net.net> 2025
 */

#include "RGLed.hpp"
#include <Arduino.h>

RGLed::RGLed(const uint8_t red_pin, const uint8_t green_pin): redPin(red_pin), greenPin(green_pin)
{
}

void RGLed::begin()
{
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    setColor(OFF);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void RGLed::setColor(const RGLedColor color) // NOLINT(*-make-member-function-const)
{
    this->color = color;
    switch (color) {
    case OFF:
        digitalWrite(redPin, HIGH);
        digitalWrite(greenPin, HIGH);
        break;
    case RED:
        digitalWrite(redPin, LOW);
        digitalWrite(greenPin, HIGH);
        break;
    case GREEN:
        digitalWrite(redPin, HIGH);
        digitalWrite(greenPin, LOW);
        break;
    }
}
