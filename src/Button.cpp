/**
 * SPDX-License-Identifier: MIT
 * SPDX-SnippetCopyrightText: 2025 Arthur Moore <Arthur.Moore.git@cd-net.net>
 * @file
 * @copyright Arthur Moore <Arthur.Moore.git@cd-net.net> 2025
 */

#include "Button.hpp"
#include <Arduino.h>
#include <forward_list>

Button::Button(const uint8_t pin, const unsigned long debounce):
    pin(pin),
    debounce(debounce)
{
}

Button::~Button()
{
    detachInterrupt(digitalPinToInterrupt(pin));
}

void Button::begin(const std::function<void()>& callback)
{
    pinMode(pin, INPUT_PULLUP);
    attachInterruptArg(digitalPinToInterrupt(pin), _isr, this, CHANGE);
    this->callback = callback;
}

void Button::update()
{
    if (callback != nullptr && !callbackRan && _pressedForDebounceTimeInternal())
    {
        callbackRan = true;
        callback();
    }
}

ButtonState Button::getState() const
{
    return static_cast<ButtonState>( callbackRan || _pressedForDebounceTimeInternal());
}

bool Button::_pressedForDebounceTimeInternal() const
{
    return isPressed && (pressedAt - debounce) < millis();
}

void Button::_isr(void* buttonPtr)
{
    if (buttonPtr == nullptr)
    {
        return;
    }
    const auto button = static_cast<Button*>(buttonPtr);
    if (digitalRead(button->pin) == LOW)
    {
        button->onPress();
    }
    else
    {
        button->onRelease();
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
