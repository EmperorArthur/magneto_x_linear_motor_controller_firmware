#pragma once
#include <array>

void sendModbusCommand(uint8_t id, const uint8_t *tx_buffer, size_t len);

inline void sendModbusCommand(uint8_t id, const std::array<uint8_t, 6>& tx_buffer, size_t len)
{
    sendModbusCommand(id, tx_buffer.data(), tx_buffer.size());
}
