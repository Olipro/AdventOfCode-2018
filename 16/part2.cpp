#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

using namespace std::string_literals;

constexpr auto INSN_COUNT = 16;

struct Insn {
    uint16_t op;
    uint16_t arg0;
    uint16_t arg1;
    uint16_t arg2;
    Insn(uint16_t op, uint16_t arg0, uint16_t arg1, uint16_t arg2) : op{op}, arg0{arg0}, arg1{arg1}, arg2{arg2} {}
};

class CPU {
    std::array<uint16_t, 4> reg{};
    std::array<void (CPU::*)(const Insn&), INSN_COUNT> dispatch{};

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

protected:
    void SwapInsns(uint16_t a, uint16_t b) {
        std::swap(dispatch[a], dispatch[b]);
    }

public:
    constexpr CPU(uint16_t r0, uint16_t r1, uint16_t r2, uint16_t r3) : 
        reg{r0, r1, r2, r3},
        dispatch{&CPU::AddR, &CPU::AddI,
                 &CPU::MulR, &CPU::MulI,
                 &CPU::BanR, &CPU::BanI,
                 &CPU::BorR, &CPU::BorI,
                 &CPU::SetR, &CPU::SetI,
                 &CPU::GtiR, &CPU::GtrI, &CPU::GtrR,
                 &CPU::EqiR, &CPU::EqrI, &CPU::EqrR} {}

    CPU& operator=(const CPU& other) noexcept {
        reg = other.reg;
        return *this;
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
        (this->*dispatch[insn.op])(insn);
    }
};

class DynamicCPU : public CPU {
    std::array<bool, INSN_COUNT> learned;

    void LearnInsn(std::pair<uint16_t, uint16_t> toSwap) {
        auto& [matched, old] = toSwap;
        SwapInsns(old, matched);
        learned[matched] = true;
        if (LearnedCount() == (learned.size() - 1))
            for (auto&& i : learned)
                i = true;
    }

    DynamicCPU& operator=(const CPU& other) noexcept {
        CPU::operator=(other);
        return *this;
    }

public:
    constexpr DynamicCPU() : CPU{0, 0, 0, 0}, learned{} {}

    void TryLearnInsn(const CPU& initState, const Insn& op, const CPU& resultState) {
        std::optional<std::pair<uint16_t, uint16_t>> matchedOp;
        auto simOp = op;
        auto cpu = *this;
        for (uint16_t i = 0; i < INSN_COUNT; ++i) {
            if (learned[i])
                continue;
            cpu = initState;
            simOp.op = i;
            cpu.Execute(simOp);
            if (cpu == resultState) {
                if (!matchedOp)
                    matchedOp = std::make_pair(op.op, i);
                else
                    return;
            }
        }
        if (matchedOp)
            LearnInsn(matchedOp.value());
    }

    size_t LearnedCount() const noexcept {
        return std::count(learned.begin(), learned.end(), true);
    }

    bool AllLearned() const noexcept {
        return std::find(learned.begin(), learned.end(), false) == learned.end();
    }
};

class StateChangeLog {
    CPU initialState;
    Insn op;
    CPU resultState;
public:
    StateChangeLog(CPU initialState, Insn op, CPU resultState) noexcept :
        initialState{std::move(initialState)}, op{std::move(op)}, resultState{std::move(resultState)} {}

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
    std::vector<Insn> instructions;

    static decltype(changeLogs) InitChangeLogs(std::istream& in) {
        decltype(changeLogs) ret;
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
            Insn op{insn, arg0, arg1, arg2};
            in.seekg(10, std::ios_base::cur);
            in >> r0;
            in.seekg(2, std::ios_base::cur);
            in >> r1;
            in.seekg(2, std::ios_base::cur);
            in >> r2;
            in.seekg(2, std::ios_base::cur);
            in >> r3;
            CPU resultState{r0, r1, r2, r3};
            ret.emplace_back(std::move(initialState), std::move(op), std::move(resultState));
            in.seekg(3, std::ios_base::cur);
            chk = (in.get() << 8) | in.get();
        }
        return ret;
    }

    static decltype(instructions) InitInsns(std::istream& in) {
        decltype(instructions) ret;
        uint16_t insn, arg0, arg1, arg2;
        while (in) {
            in >> insn >> arg0 >> arg1 >> arg2;
            if (in)
                ret.emplace_back(insn, arg0, arg1, arg2);
            in.seekg(1, std::ios_base::cur);
        }
        return ret;
    }

public:
    StateLogs(std::istream& in) : changeLogs{InitChangeLogs(in)}, instructions{InitInsns(in)} {}

    uint16_t SimulateProgram() const {
        DynamicCPU cpu;
        while (!cpu.AllLearned())
            for (const auto& log : changeLogs)
                cpu.TryLearnInsn(log.InitialState(), log.Op(), log.ResultState());
        for (const auto& insn : instructions)
            cpu.Execute(insn);
        return cpu.R0();
    }
};

int main(int argc, const char* argv[]) {
    std::ifstream file{argc == 2 ? argv[1] : "input.txt"};
    StateLogs logs{file};
    std::cout << logs.SimulateProgram() << '\n';
}
