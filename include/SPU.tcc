#include "SPU.hpp"

template <typename T>
inline T SPU::load(uint32_t offset) const {
    static_assert(std::is_same<T, uint8_t>() || std::is_same<T, uint16_t>() || std::is_same<T, uint32_t>(), "Invalid type");
    switch (offset) {
        case 0x1ae: {
            return statusRegister();
        }
        default: {
            logger.logError("Unhandled Sound Processing Unit read at offset: %#x, of size: %d", offset, sizeof(T));
            return 0;
        }
    }
}

template <typename T>
inline void SPU::store(uint32_t offset, T value) {
    static_assert(std::is_same<T, uint8_t>() || std::is_same<T, uint16_t>() || std::is_same<T, uint32_t>(), "Invalid type");
    switch (offset) {
        case 0x180: {
            logger.logMessage("Unhandled Sound Processing Unit main volume left write");
            break;
        }
        case 0x182: {
            logger.logMessage("Unhandled Sound Processing Unit main volume right write");
            break;
        }
        case 0x184: {
            logger.logMessage("Unhandled Sound Processing Unit reverb volume output left write");
            break;
        }
        case 0x186: {
            logger.logMessage("Unhandled Sound Processing Unit reverb volume output right write");
            break;
        }
        case 0x1aa: {
            if (sizeof(T) != 2) {
                logger.logError("Unsupported SPUCNT write with size: %d", sizeof(T));
            }
            setControlRegister(value);
            break;
        }
        case 0x18c: {
            if (sizeof(T) != 2) {
                logger.logError("Unsupported KOFF write with size: %d", sizeof(T));
            }
            uint32_t upper = (voiceKeyOff.value >> 16) << 16;
            uint32_t toWrite = upper | value;
            setVoiceKeyOffRegister(toWrite);
            break;
        }
        case 0x18e: {
            if (sizeof(T) != 2) {
                logger.logError("Unsupported KOFF write with size: %d", sizeof(T));
            }
            uint32_t lower = voiceKeyOff.value & 0xFFFF;
            uint32_t toWrite = (value << 16) | lower;
            setVoiceKeyOffRegister(toWrite);
            break;
        }
        default: {
            logger.logError("Unhandled Sound Processing Unit write at offset: %#x, of size: %d", offset, sizeof(T));
            return;
        }
    }
}