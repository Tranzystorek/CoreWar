#ifndef VIRTUALMACHINE_HPP
#define VIRTUALMACHINE_HPP

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
		~Core();

		ProgramPtr begin() const;

		unsigned int getSize() const;

	private:

		Instruction* memory_;

		unsigned int size_;

		friend class VirtualMachine;
	};

private:

	using ProcessQueue = std::queue<Core::ProgramPtr>;

public:

	VirtualMachine(unsigned int);

	void executeInstruction(ProcessQueue&);

private:

	int convertValue(int);

	ProcessQueue p1_;
	ProcessQueue p2_;

	Core core_;
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

	explicit ProgramPtr(Core::Instruction* p, const Core& r) : ptr_(p),
															   ref_(const_cast<Core&>(r)) {}

	ProgramPtr& operator=(const ProgramPtr&);

	Core::Instruction& operator*();
	Core::Instruction* operator->();

	ProgramPtr operator+(unsigned int);

	ProgramPtr& operator+=(unsigned int);

	ProgramPtr& operator++();
	ProgramPtr operator++(int);

private:

	Core::Instruction* ptr_;

	Core& ref_;
};

#endif //VIRTUALMACHINE_HPP
