/**
 * SPDX-License-Identifier: MIT
 * SPDX-SnippetCopyrightText: 2025 Arthur Moore <Arthur.Moore.git@cd-net.net>
 * @file
 * @copyright Arthur Moore <Arthur.Moore.git@cd-net.net> 2025
 */

#include "Button.hpp"
#include <Arduino.h>

Button::Button(uint8_t pin, unsigned long debounce): pin(pin),
                                                     debounce(debounce)
{
}

void Button::begin(const std::function<void()>& callback)
{
    pinMode(pin, INPUT_PULLUP);
    this->callback = callback;
}

void Button::update()
{
    onStateChange();
    if (callback != nullptr && !callbackRan && isPressed && (pressedAt - debounce) < millis())
    {
        callbackRan = true;
        callback();
    }
}

ButtonState Button::getState()
{
    onStateChange();
    return static_cast<ButtonState>(isPressed && (pressedAt - debounce) < millis());
}

void Button::onStateChange()
{
    if (digitalRead(pin) == LOW)
    {
        onPress();
    }
    else
    {
        onRelease();
    }
}

void Button::onPress()
{
    if (isPressed)
    {
        return;
    }
    isPressed = true;
    pressedAt = millis();
}

void Button::onRelease()
{
    isPressed = false;
    callbackRan = false;
    pressedAt = -1;
}
