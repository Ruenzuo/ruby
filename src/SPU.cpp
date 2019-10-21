#include "SPU.hpp"

SPU::SPU(LogLevel logLevel) : logger(logLevel, "  SPU: "), control(), status(), voiceKeyOff() {

}

SPU::~SPU() {

}

uint16_t SPU::controlRegister() const {
    return control.value;
}

void SPU::setControlRegister(uint16_t value) {
    // TODO: mirror Bit5-0 to SPUSTAT.Bit5-0
    // docs say it should be a delayed write
    control.value = value;
}

uint16_t SPU::statusRegister() const {
    return status.value;
}

void SPU::setVoiceKeyOffRegister(uint32_t value) {
    voiceKeyOff.value = value;
}

void SPU::setPitchModulationEnableFlagsRegister(uint32_t value) {
    pitchModulationEnableFlags.value = value;
}

uint32_t SPU::pitchModulationEnableFlagsRegister() const {
    return pitchModulationEnableFlags.value;
}

void SPU::setNoiseModeEnableRegister(uint32_t value) {
    noiseModeEnable.value = value;
}

uint32_t SPU::noiseModeEnableRegister() const {
    return noiseModeEnable.value;
}

void SPU::setReverbModeRegister(uint32_t value) {
    reverbMode.value = value;
}

uint32_t SPU::reverbModeRegister() const {
    return reverbMode.value;
}

void SPU::setCDAudioInputVolumeLeft(uint16_t value) {
    CDAudioInputVolume.left = value;
}

uint16_t SPU::CDAudioInputVolumeLeft() const {
    return CDAudioInputVolume.left;
}

void SPU::setCDAudioInputVolumeRight(uint16_t value) {
    CDAudioInputVolume.right = value;
}

uint16_t SPU::CDAudioInputVolumeRight() const {
    return CDAudioInputVolume.right;
}
