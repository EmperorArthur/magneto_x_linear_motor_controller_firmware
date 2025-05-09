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

void Button::Setup(const std::function<void()>& callback)
{
    pinMode(pin, INPUT_PULLUP);
    this->callback = callback;
}

void Button::Update()
{
    if (digitalRead(pin) == LOW)
    {
        OnPress();
    }
    else
    {
        OnRelease();
    }

    if (callback != nullptr && !callbackRan && isPressed && (pressedAt - debounce) < millis())
    {
        callbackRan = true;
        callback();
    }
}

void Button::OnPress()
{
    if (isPressed)
    {
        return;
    }
    isPressed = true;
    pressedAt = millis();
}

void Button::OnRelease()
{
    isPressed = false;
    callbackRan = false;
    pressedAt = -1;
}
