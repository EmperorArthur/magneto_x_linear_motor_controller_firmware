/**
 * SPDX-License-Identifier: MIT
 * SPDX-SnippetCopyrightText: 2025 Arthur Moore <Arthur.Moore.git@cd-net.net>
 * @file
 * @copyright Arthur Moore <Arthur.Moore.git@cd-net.net> 2025
 */

#include "RGLed.hpp"
#include <Arduino.h>

RGLed::RGLed(uint8_t red_pin, uint8_t green_pin): redPin(red_pin), greenPin(green_pin)
{
}

void RGLed::Setup()
{
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    Off();
}

void RGLed::Off()
{
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);
}

void RGLed::SetRed()
{
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, HIGH);
}

void RGLed::SetGreen()
{
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);
}

bool RGLed::IsRed() const
{
    return digitalRead(redPin);
}

bool RGLed::IsGreen() const
{
    return digitalRead(greenPin);
}
