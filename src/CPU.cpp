#include "CPU.hpp"
#include <algorithm>
#include <iostream>
#include <tuple>

using namespace std;

CPU::CPU(Interconnect &interconnect) : programCounter(0xbfc00000), nextInstruction(Instruction(0x0)), load({RegisterIndex(), 0}), statusRegister(0), interconnect(interconnect) {
    fill_n(registers, 32, 0xDEADBEEF);
    registers[0] = 0;
    copy(begin(registers), end(registers), begin(outputRegisters));
}

CPU::~CPU() {

}

uint32_t CPU::readWord(uint32_t address) const {
    return interconnect.readWord(address);
}

uint8_t CPU::readByte(uint32_t address) const {
    return interconnect.readByte(address);
}

void CPU::storeWord(uint32_t address, uint32_t value) const {
    if ((statusRegister & 0x10000) != 0) {
        cout << "Cache is isolated, ignoring store at address: 0x" << hex << address << endl;
        return;
    }
    return interconnect.storeWord(address, value);
}

void CPU::storeHalfWord(uint32_t address, uint16_t value) const {
    if ((statusRegister & 0x10000) != 0) {
        cout << "Cache is isolated, ignoring store at address: 0x" << hex << address << endl;
        return;
    }
    return interconnect.storeHalfWord(address, value);
}

void CPU::storeByte(uint32_t address, uint8_t value) const {
    if ((statusRegister & 0x10000) != 0) {
        cout << "Cache is isolated, ignoring store at address: 0x" << hex << address << endl;
        return;
    }
    return interconnect.storeByte(address, value);
}

void CPU::executeNextInstruction() {
    Instruction instruction = nextInstruction;
    uint32_t data = readWord(programCounter);
    nextInstruction = Instruction(data);
    programCounter+=4;

    RegisterIndex loadRegisterIndex;
    uint32_t value;
    tie(loadRegisterIndex, value) = load;
    setRegisterAtIndex(loadRegisterIndex, value);
    load = {RegisterIndex(), 0};

    executeNextInstruction(instruction);
    copy(begin(outputRegisters), end(outputRegisters), begin(registers));
}

uint32_t CPU::registerAtIndex(RegisterIndex index) const {
    return registers[index.idx()];
}

void CPU::setRegisterAtIndex(RegisterIndex index, uint32_t value) {
    outputRegisters[index.idx()] = value;

    // Make sure R0 is always 0
    outputRegisters[0] = 0;
}

void CPU::executeNextInstruction(Instruction instruction) {
    switch (instruction.funct()) {
        case 0b000000: {
            switch (instruction.subfunct()) {
                case 0b000000: {
                    operationShiftLeftLogical(instruction);
                    break;
                }
                case 0b100101: {
                    operationBitwiseOr(instruction);
                    break;
                }
                case 0b101011: {
                    operationSetLessThanUnsigned(instruction);
                    break;
                }
                case 0b100001: {
                    operationAddUnsigned(instruction);
                    break;
                }
                case 0b001000: {
                    operationJumpRegister(instruction);
                    break;
                }
                default: {
                    cout << "Unhandled instruction 0x" << hex << instruction.dat() << endl;
                    exit(1);
                }
            }
        }
        case 0b001111: {
            operationLoadUpperImmediate(instruction);
            break;
        }
        case 0b001101: {
            operationBitwiseOrImmediate(instruction);
            break;
        }
        case 0b101011: {
            operationStoreWord(instruction);
            break;
        }
        case 0b01001: {
            operationAddImmediateUnsigned(instruction);
            break;
        }
        case 0b000010: {
            operationJump(instruction);
            break;
        }
        case 0b010000: {
            operationCoprocessor0(instruction);
            break;
        }
        case 0b000101: {
            operationBranchIfNotEqual(instruction);
            break;
        }
        case 0b001000: {
            operationAddImmediate(instruction);
            break;
        }
        case 0b100011: {
            operationLoadWord(instruction);
            break;
        }
        case 0b101001: {
            operationStoreHalfWord(instruction);
            break;
        }
        case 0b000011: {
            operationJumpAndLink(instruction);
            break;
        }
        case 0b001100: {
            operationBitwiseAndImmediate(instruction);
            break;
        }
        case 0b101000: {
            operationStoreByte(instruction);
            break;
        }
        case 0b100000: {
            operationLoadByte(instruction);
            break;
        }
        case 0b000100: {
            operationBranchIfEqual(instruction);
            break;
        }
        default: {
            cout << "Unhandled instruction 0x" << hex << instruction.dat() << endl;
            exit(1);
        }
    }
}

void CPU::branch(uint32_t offset) {
    // Align to 32 bits
    offset <<= 2;
    programCounter += offset;
    // PC is positioned already at the next instruction
    // we need to take in consideration that
    programCounter -= 4;
}

void CPU::operationCoprocessor0(Instruction instruction) {
    switch (instruction.copcode()) {
        case 0b00100: {
            operationMoveToCoprocessor0(instruction);
            break;
        }
        case 0x00000: {
            operationMoveFromCoprocessor0(instruction);
            break;
        }
        default: {
            cout << "Unhandled coprocessor0 instruction 0x" << hex << instruction.dat() << endl;
            exit(1);
        }
    }
}


void CPU::operationMoveToCoprocessor0(Instruction instruction) {
    RegisterIndex cpuRegisterIndex = instruction.rt();
    RegisterIndex copRegisterIndex = instruction.rd();

    uint32_t value = registerAtIndex(cpuRegisterIndex);

    switch (copRegisterIndex.idx()) {
        case 3:
        case 5:
        case 6:
        case 7:
        case 9:
        case 11: {
            if (value != 0) {
                cout << "Unhandled MTC0 at index " << copRegisterIndex.idx() << endl;
                exit(1);
            }
            break;
        }
        case 12: {
            statusRegister = value;
            break;
        }
        case 13: {
            if (value != 0) {
                cout << "Unhandled MTC0 at CAUSE register" << endl;
                exit(1);
            }
            break;
        }
        default: {
            cout << "Unhandled MTC0 at index " << copRegisterIndex.idx() << endl;
            exit(1);
        }
    }
}

void CPU::operationLoadUpperImmediate(Instruction instruction) {
    uint32_t imm = instruction.imm();
    RegisterIndex rt = instruction.rt();

    uint32_t value = imm << 16;
    setRegisterAtIndex(rt, value);
}

void CPU::operationBitwiseOrImmediate(Instruction instruction) {
    uint32_t imm = instruction.imm();
    RegisterIndex rt = instruction.rt();
    RegisterIndex rs = instruction.rs();

    uint32_t value = registerAtIndex(rs) | imm;

    setRegisterAtIndex(rt, value);
}

void CPU::operationStoreWord(Instruction instruction) const {
    uint32_t imm = instruction.immSE();
    RegisterIndex rt = instruction.rt();
    RegisterIndex rs = instruction.rs();

    uint32_t address = registerAtIndex(rs) + imm;
    uint32_t value = registerAtIndex(rt);

    storeWord(address, value);
}

void CPU::operationShiftLeftLogical(Instruction instruction) {
    uint32_t imm = instruction.shiftimm();
    RegisterIndex rt = instruction.rt();
    RegisterIndex rd = instruction.rd();

    uint32_t value = registerAtIndex(rt) << imm;
    setRegisterAtIndex(rd, value);
}

void CPU::operationAddImmediateUnsigned(Instruction instruction) {
    uint32_t imm = instruction.immSE();
    RegisterIndex rt = instruction.rt();
    RegisterIndex rs = instruction.rs();

    uint32_t value = registerAtIndex(rs) + imm;
    setRegisterAtIndex(rt, value);
}

void CPU::operationJump(Instruction instruction) {
    uint32_t imm = instruction.immjump();
    programCounter = (programCounter & 0xF0000000) | (imm << 2);
}

void CPU::operationBitwiseOr(Instruction instruction) {
    RegisterIndex rd = instruction.rd();
    RegisterIndex rs = instruction.rs();
    RegisterIndex rt = instruction.rt();

    uint32_t value = registerAtIndex(rs) | registerAtIndex(rt);
    setRegisterAtIndex(rd, value);
}

void CPU::operationBranchIfNotEqual(Instruction instruction) {
    uint32_t imm = instruction.immSE();
    RegisterIndex rs = instruction.rs();
    RegisterIndex rt = instruction.rt();

    if (registerAtIndex(rs) != registerAtIndex(rt)) {
        branch(imm);
    }
}

void CPU::operationAddImmediate(Instruction instruction) {
    uint32_t imm = instruction.immSE();
    RegisterIndex rt = instruction.rt();
    RegisterIndex rs = instruction.rs();

    uint32_t value = registerAtIndex(rs);
    uint32_t result = value + imm;
    if (result < value) {
        cout << "Unhandled ADDI overflow" << endl;
        exit(1);
    } else {
        setRegisterAtIndex(rt, result);
    }
}

void CPU::operationLoadWord(Instruction instruction) {
    uint32_t imm = instruction.immSE();
    RegisterIndex rt = instruction.rt();
    RegisterIndex rs = instruction.rs();

    uint32_t address = registerAtIndex(rs) + imm;
    if ((statusRegister & 0x10000) != 0) {
        cout << "Cache is isolated, ignoring store at address: 0x" << hex << address << endl;
        return;
    }
    uint32_t value = readWord(address);
    load = {rt, value};
}

void CPU::operationSetLessThanUnsigned(Instruction instruction) {
    RegisterIndex rd = instruction.rd();
    RegisterIndex rs = instruction.rs();
    RegisterIndex rt = instruction.rt();

    uint32_t value = registerAtIndex(rs) < registerAtIndex(rt);
    setRegisterAtIndex(rd, value);
}

void CPU::operationAddUnsigned(Instruction instruction) {
    RegisterIndex rs = instruction.rs();
    RegisterIndex rt = instruction.rt();
    RegisterIndex rd = instruction.rd();

    uint32_t value = registerAtIndex(rs) + registerAtIndex(rt);
    setRegisterAtIndex(rd, value);
}

void CPU::operationStoreHalfWord(Instruction instruction) const {
    uint32_t imm = instruction.immSE();
    RegisterIndex rt = instruction.rt();
    RegisterIndex rs = instruction.rs();

    uint32_t address = registerAtIndex(rs) + imm;
    if ((statusRegister & 0x10000) != 0) {
        cout << "Cache is isolated, ignoring store at address: 0x" << hex << address << endl;
        return;
    }

    uint32_t value = registerAtIndex(rt);
    storeHalfWord(address, value);
}

void CPU::operationJumpAndLink(Instruction instruction) {
    uint32_t returnAddress = programCounter;
    setRegisterAtIndex(RegisterIndex(31), returnAddress);
    operationJump(instruction);
}

void CPU::operationBitwiseAndImmediate(Instruction instruction) {
    uint32_t imm = instruction.imm();
    RegisterIndex rt = instruction.rt();
    RegisterIndex rs = instruction.rs();

    uint32_t value = registerAtIndex(rs) & imm;

    setRegisterAtIndex(rt, value);
}

void CPU::operationStoreByte(Instruction instruction) const {
    uint32_t imm = instruction.immSE();
    RegisterIndex rt = instruction.rt();
    RegisterIndex rs = instruction.rs();

    uint32_t address = registerAtIndex(rs) + imm;
    if ((statusRegister & 0x10000) != 0) {
        cout << "Cache is isolated, ignoring store at address: 0x" << hex << address << endl;
        return;
    }

    uint32_t value = registerAtIndex(rt);
    storeByte(address, value);
}

void CPU::operationJumpRegister(Instruction instruction) {
    RegisterIndex rs = instruction.rs();
    programCounter = registerAtIndex(rs);
}

void CPU::operationLoadByte(Instruction instruction) {
    uint32_t imm = instruction.immSE();
    RegisterIndex rt = instruction.rt();
    RegisterIndex rs = instruction.rs();

    uint32_t address = registerAtIndex(rs) + imm;
    uint32_t value = (int8_t)readByte(address);
    load = {rt, value};
}

void CPU::operationBranchIfEqual(Instruction instruction) {
    uint32_t imm = instruction.immSE();
    RegisterIndex rs = instruction.rs();
    RegisterIndex rt = instruction.rt();

    if (registerAtIndex(rs) == registerAtIndex(rt)) {
        branch(imm);
    }
}

void CPU::operationMoveFromCoprocessor0(Instruction instruction) {
    RegisterIndex cpuRegisterIndex = instruction.rt();
    RegisterIndex copRegisterIndex = instruction.rd();

    uint32_t value;
    switch (copRegisterIndex.idx()) {
        case 12: {
            value = statusRegister;
            break;
        }
        case 13: {
            cout << "Unhandled MFC0 at CAUSE register" << endl;
            exit(1);
            break;
        }
        default: {
            cout << "Unhandled MFC0 at index " << copRegisterIndex.idx() << endl;
            exit(1);
        }
    }
    load = {cpuRegisterIndex, value};
}
