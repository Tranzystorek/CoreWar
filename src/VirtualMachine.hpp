#ifndef VIRTUALMACHINE_HPP
#define VIRTUALMACHINE_HPP

#include <vector>
#include <queue>
#include <string>

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
	};//Core

	struct Core::Instruction
	{
		enum OpCode : unsigned char {KIL, FRK, NOP, MOV, ADD, SUB, MUL, DIV, MOD,
									 JMP, JMZ, JMN, BEQ, BNE, BLT};
		enum Modifier : unsigned char {A, B, AB, BA, F, X, I};
		enum AddressMode : unsigned char {IMM, DIR, AIN, BIN};

		Instruction(OpCode o = KIL, Modifier m = F,
					int a = 0, int b = 0,
					AddressMode am = DIR, AddressMode bm = DIR)

			: op(o), mod(m),
			  aMode(am), aVal(a),
			  bMode(bm), bVal(b) {}

		OpCode op;

		Modifier mod;

		AddressMode aMode;
		unsigned int aVal;

		AddressMode bMode;
		unsigned int bVal;

		bool operator==(const Instruction&) const;
		bool operator!=(const Instruction&) const;
	};//Instruction

	class StatReport
	{
	public:

		enum RoundState {ONGOING, DRAW, P1_WON, P2_WON};

		StatReport();

		void exec(unsigned int, const VirtualMachine::Core::Instruction&);

		void read(unsigned int);
		void write(unsigned int);

		void createProcess();
		void killProcess();

		static void setState(RoundState);

		void clear();

		std::string toString();

		unsigned int getProcessCount() const;

		unsigned int getExecutedAdr() const;

		unsigned int getWriteAdr() const;

		static RoundState getState();

	private:

		std::vector<unsigned int> readAdrs_;
		unsigned int writeAdr_;

		unsigned int executedAdr_;

		unsigned int procCount_;

		VirtualMachine::Core::Instruction ins_;

		static RoundState state_;

		static std::string op[10];
		static std::string mod[7];
		static std::string address[4];
	};//StatReport

private:

	using ProcessQueue = std::queue<Core::ProgramPtr>;

public:

	VirtualMachine(unsigned int = 8000);

	void loadProgram(const std::vector<Core::Instruction>&, unsigned int, bool = true);
	void loadProgram(const char*, unsigned int, bool = true);

	void executeCycle();

	void reset();

	unsigned int getCoreSize() const;

	bool isLoadedP1() const;
	bool isLoadedP2() const;

	StatReport& getP1Report();
	StatReport& getP2Report();

private:

	void executeInstruction(ProcessQueue&, StatReport&);

	ProcessQueue p1_;
	ProcessQueue p2_;

	StatReport p1Report_;
	StatReport p2Report_;

	Core core_;

	unsigned int maxCycles_;
	unsigned int maxProcesses_;

	unsigned int currentCycle_;

	bool loaded_p1_;
	bool loaded_p2_;
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

	unsigned int pos() const;

private:

	std::vector<Core::Instruction>::iterator it_;

	Core& ref_;
};

#endif //VIRTUALMACHINE_HPP
