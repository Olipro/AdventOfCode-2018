#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace std::string_literals;

enum OpCode {
    ADDR,
    ADDI,
    MULR,
    MULI,
    BANR,
    BANI,
    BORR,
    BORI,
    SETR,
    SETI,
    GTIR,
    GTRI,
    GTRR,
    EQIR,
    EQRI,
    EQRR,
    OUT_OF_RANGE,
};

struct Insn {
    OpCode op;
    uint16_t arg0;
    uint16_t arg1;
    uint16_t arg2;
};

class CPU {
    std::vector<uint16_t> reg;

    void AddR(const Insn& insn) {
        reg[insn.arg2] = reg[insn.arg0] + reg[insn.arg1];
    }

    void AddI(const Insn& insn) {
        reg[insn.arg2] = reg[insn.arg0] + insn.arg1;
    }

    void MulR(const Insn& insn) {
        reg[insn.arg2] = reg[insn.arg0] * reg[insn.arg1];
    }

    void MulI(const Insn& insn) {
        reg[insn.arg2] = reg[insn.arg0] * insn.arg1;
    }

    void BanR(const Insn& insn) {
        reg[insn.arg2] = reg[insn.arg0] & reg[insn.arg1];
    }

    void BanI(const Insn& insn) {
        reg[insn.arg2] = reg[insn.arg0] & insn.arg1;
    }

    void BorR(const Insn& insn) {
        reg[insn.arg2] = reg[insn.arg0] | reg[insn.arg1];
    }

    void BorI(const Insn& insn) {
        reg[insn.arg2] = reg[insn.arg0] | insn.arg1;
    }

    void SetR(const Insn& insn) {
        reg[insn.arg2] = reg[insn.arg0];
    }

    void SetI(const Insn& insn) {
        reg[insn.arg2] = insn.arg0;
    }

    void GtiR(const Insn& insn) {
        reg[insn.arg2] = insn.arg0 > reg[insn.arg1] ? 1 : 0;
    }

    void GtrI(const Insn& insn) {
        reg[insn.arg2] = reg[insn.arg0] > insn.arg1 ? 1 : 0;
    }

    void GtrR(const Insn& insn) {
        reg[insn.arg2] = reg[insn.arg0] > reg[insn.arg1] ? 1 : 0;
    }

    void EqiR(const Insn& insn) {
        reg[insn.arg2] = insn.arg0 == reg[insn.arg1] ? 1 : 0;
    }

    void EqrI(const Insn& insn) {
        reg[insn.arg2] = reg[insn.arg0] == insn.arg1 ? 1 : 0;
    }

    void EqrR(const Insn& insn) {
        reg[insn.arg2] = reg[insn.arg0] == reg[insn.arg1] ? 1 : 0;
    }
public:
    CPU(uint16_t r0, uint16_t r1, uint16_t r2, uint16_t r3) : reg(4, 0) {
        reg[0] = r0;
        reg[1] = r1;
        reg[2] = r2;
        reg[3] = r3;
    }

    uint16_t R0() const noexcept {
        return reg[0];
    }

    uint16_t R1() const noexcept {
        return reg[1];
    }

    uint16_t R2() const noexcept {
        return reg[2];
    }

    uint16_t R3() const noexcept {
        return reg[3];
    }

    bool operator==(const CPU& other) const noexcept {
        return std::equal(reg.begin(), reg.end(), other.reg.begin(), other.reg.end());
    }

    void Execute(const Insn& insn) {
        switch (insn.op) {
            case OpCode::ADDR:
                return AddR(insn);
            case OpCode::ADDI:
                return AddI(insn);
            case OpCode::MULR:
                return MulR(insn);
            case OpCode::MULI:
                return MulI(insn);
            case OpCode::BANR:
                return BanR(insn);
            case OpCode::BANI:
                return BanI(insn);
            case OpCode::BORR:
                return BorR(insn);
            case OpCode::BORI:
                return BorI(insn);
            case OpCode::SETR:
                return SetR(insn);
            case OpCode::SETI:
                return SetI(insn);
            case OpCode::GTIR:
                return GtiR(insn);
            case OpCode::GTRI:
                return GtrI(insn);
            case OpCode::GTRR:
                return GtrR(insn);
            case OpCode::EQIR:
                return EqiR(insn);
            case OpCode::EQRI:
                return EqrI(insn);
            case OpCode::EQRR:
                return EqrR(insn);
            case OpCode::OUT_OF_RANGE:
                break;
        }
    }
};

class StateChangeLog {
    CPU initialState;
    Insn op;
    CPU resultState;
public:
    StateChangeLog(CPU initialState, Insn op, CPU resultState) noexcept :
        initialState{std::move(initialState)}, op{op}, resultState{std::move(resultState)} {}

    const CPU& InitialState() const noexcept {
        return initialState;
    }

    const Insn& Op() const noexcept {
        return op;
    }

    const CPU& ResultState() const noexcept {
        return resultState;
    }
};

class StateLogs {
    std::vector<StateChangeLog> changeLogs;
public:
    StateLogs(std::istream& in) {
        uint16_t chk = (in.get() << 8) | in.get();
        while (in && chk == (('B' << 8) | 'e')) {
            in.seekg(7, std::ios_base::cur);
            uint16_t r0, r1, r2, r3;
            in >> r0;
            in.seekg(2, std::ios_base::cur);
            in >> r1;
            in.seekg(2, std::ios_base::cur);
            in >> r2;
            in.seekg(2, std::ios_base::cur);
            in >> r3;
            CPU initialState{r0, r1, r2, r3};
            in.seekg(2, std::ios_base::cur);
            uint16_t insn, arg0, arg1, arg2;
            in >> insn >> arg0 >> arg1 >> arg2;
            Insn op{static_cast<OpCode>(insn), arg0, arg1, arg2};
            in.seekg(10, std::ios_base::cur);
            in >> r0;
            in.seekg(2, std::ios_base::cur);
            in >> r1;
            in.seekg(2, std::ios_base::cur);
            in >> r2;
            in.seekg(2, std::ios_base::cur);
            in >> r3;
            CPU resultState{r0, r1, r2, r3};
            changeLogs.emplace_back(std::move(initialState), op, std::move(resultState));
            in.seekg(3, std::ios_base::cur);
            chk = (in.get() << 8) | in.get();
        }
    }

    size_t CountOpsMatching3Insns() const {
        size_t ret = 0;
        for (const auto& log : changeLogs) {
            auto insn = log.Op();
            auto matches = 0;
            for (int i = OpCode::ADDR; i < OpCode::OUT_OF_RANGE; ++i) {
                insn.op = static_cast<OpCode>(i);
                auto tmpCpu = log.InitialState();
                tmpCpu.Execute(insn);
                if (tmpCpu == log.ResultState() && ++matches == 3) {
                    ++ret;
                    break;
                }
            }
        }
        return ret;
    }
};

int main(int argc, const char* argv[]) {
    std::ifstream file{argc == 2 ? argv[1] : "input.txt"};
    StateLogs logs{file};
    std::cout << logs.CountOpsMatching3Insns() << '\n';
}
