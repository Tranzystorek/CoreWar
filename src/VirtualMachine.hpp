#ifndef VIRTUALMACHINE_HPP
#define VIRTUALMACHINE_HPP

#include <vector>
#include <queue>

class VirtualMachine
{
public:

	class Core
	{
	public:

		class ProgramPtr;
		struct Instruction;

		Core(unsigned int);

		ProgramPtr begin();

		unsigned int getSize() const;

	private:

		std::vector<Instruction> memory_;

		unsigned int size_;

		friend class VirtualMachine;
	};

private:

	using ProcessQueue = std::queue<Core::ProgramPtr>;

public:

    VirtualMachine(unsigned int = 8000);

	void loadProgram(const std::vector<Core::Instruction>&, unsigned int, bool = true);

	void run();

private:

	void executeInstruction(ProcessQueue&);

	ProcessQueue p1_;
	ProcessQueue p2_;

	Core core_;

	unsigned int maxCycles_;
    unsigned int maxProcesses_;

	bool loaded_p1_;
	bool loaded_p2_;
};

//******************************************************************************
//INSTRUCTION
//******************************************************************************

struct VirtualMachine::Core::Instruction
{
	enum OpCode : unsigned char {KIL, FRK, NOP, MOV, ADD, SUB, MUL, DIV, MOD,
								 JMP};
	enum Modifier : unsigned char {A, B, AB, BA, F, X, I};
	enum AddressMode : unsigned char {IMM, DIR, AIN, BIN};

	Instruction(OpCode o = KIL, Modifier m = F,
				int a = 0, int b = 0,
				AddressMode am = DIR, AddressMode bm = DIR)

				: 	op(o), mod(m),
					aMode(am), aVal(a),
					bMode(bm), bVal(b) {}

	OpCode op;

	Modifier mod;

	AddressMode aMode;
	unsigned int aVal;

	AddressMode bMode;
	unsigned int bVal;
};

//******************************************************************************
//PROGRAM_PTR
//******************************************************************************

class VirtualMachine::Core::ProgramPtr
{
public:

	explicit ProgramPtr(std::vector<Core::Instruction>::iterator i, const Core& r) : it_(i),
															   ref_(const_cast<Core&>(r)) {}

	ProgramPtr& operator=(const ProgramPtr&);

	Core::Instruction& operator*();
	Core::Instruction* operator->();

	ProgramPtr operator+(unsigned int);

	ProgramPtr& operator+=(unsigned int);

	ProgramPtr& operator++();
	ProgramPtr operator++(int);

private:

	std::vector<Core::Instruction>::iterator it_;

	Core& ref_;
};

#endif //VIRTUALMACHINE_HPP
