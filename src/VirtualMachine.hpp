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

	ProcessQueue p1_;
	ProcessQueue p2_;
};

//******************************************************************************
//INSTRUCTION
//******************************************************************************

struct VirtualMachine::Core::Instruction
{
	enum OpCode {KIL, FRK, MOV, ADD, SUB, MUL, DIV};

	Instruction(OpCode o = KIL) : op(o) {}

	OpCode op;
};

//******************************************************************************
//PROGRAM_PTR
//******************************************************************************

class VirtualMachine::Core::ProgramPtr
{
public:

	explicit ProgramPtr(Core::Instruction* p, const Core& r) : ptr_(p),
															   ref_(r) {}

	Core::Instruction& operator*();

	ProgramPtr& operator+=(int);

	ProgramPtr& operator++();

private:

	Core::Instruction* ptr_;

	const Core& ref_;
};

#endif //VIRTUALMACHINE_HPP
