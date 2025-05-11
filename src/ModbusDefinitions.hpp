/**
 * SPDX-License-Identifier: MIT
 * SPDX-SnippetCopyrightText: 2025 Arthur Moore <Arthur.Moore.git@cd-net.net>
 * @file
 * @copyright Arthur Moore <Arthur.Moore.git@cd-net.net> 2025
 * @brief Simple helper definitions for Modbus.
 */

#pragma once
#include <ModbusADU.h>

/**
 * @brief 'GATEWAY PATH UNAVAILABLE' exception
 * @details For use with `ModbusADU::prepareExceptionResponse`
 * @see Modbus Specification V1.1b3 P.49
 */
constexpr uint8_t GATEWAY_PATH_UNAVAILABLE = 0x0A;

/**
 * @brief 'GATEWAY TARGET DEVICE FAILED TO RESPOND' exception
 * @details For use with `ModbusADU::prepareExceptionResponse`
 * @see Modbus Specification V1.1b3 P.49
 */
constexpr uint8_t GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND = 0x0B;
