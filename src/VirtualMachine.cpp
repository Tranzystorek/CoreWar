#include "VirtualMachine.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cstddef>

typedef VirtualMachine::Core Core;
typedef VirtualMachine::StatReport StatReport;
typedef VirtualMachine::Core::ProgramPtr ProgramPtr;
typedef VirtualMachine::Core::Instruction Instruction;

typedef VirtualMachine::Core::Instruction::OpCode OpCode;
typedef VirtualMachine::Core::Instruction::Modifier Modifier;
typedef VirtualMachine::Core::Instruction::AddressMode AddressMode;

std::string VirtualMachine::StatReport::op[] = {"KIL", "FRK", "NOP", "MOV", "ADD", "SUB", "MUL", "DIV", "MOD", "JMP"};
std::string VirtualMachine::StatReport::mod[] = {"A", "B", "AB", "BA", "F", "X", "I"};
std::string VirtualMachine::StatReport::address[] = {"#", "$", "*", "@"};

StatReport::RoundState StatReport::state_ = StatReport::ONGOING;

VirtualMachine::VirtualMachine(unsigned int coresize)
	: core_(coresize),
	  maxCycles_(20000),
	  maxProcesses_(64),
	  loaded_p1_(false),
	  loaded_p2_(false)
{

}

void VirtualMachine::executeInstruction(ProcessQueue& proc, StatReport& report)
{
	if(proc.empty())
		throw std::runtime_error("Attempted to obtain process from an empty ProcessQueue");

	//static std::string op[] = {"KIL", "FRK", "NOP", "MOV", "ADD", "SUB", "MUL", "DIV", "MOD", "JMP"};
	//static std::string mod[] = {"A", "B", "AB", "BA", "F", "X", "I"};
	//static std::string address[] = {"#", "$", "*", "@"};

	ProgramPtr p = proc.front();
	proc.pop();

	ProgramPtr ps = p;
	ProgramPtr pd = p;

	//instruction "registers"
	Instruction current = *p;
	Instruction src, dst;

	report.exec(p.pos(), current);

	//std::cout << op[current.op] << "." << mod[current.mod] << "\t"
	//		  << address[current.aMode] << current.aVal << "\t"
	//          << address[current.bMode] << current.bVal;

	//determine SRC instruction
	switch(current.aMode)
	{
	case AddressMode::IMM:
		break;

	case AddressMode::DIR:
		ps = p + current.aVal;
		break;

	case AddressMode::AIN:
	{
		ProgramPtr tmp = p + current.aVal;
		ps = tmp + tmp->aVal;
		break;
	}

	case AddressMode::BIN:
	{
		ProgramPtr tmp = p + current.aVal;
		ps = tmp + tmp->bVal;
		break;
	}
	}

	src = *ps;

	//determine DST instruction
	switch(current.bMode)
	{
		case AddressMode::IMM:
			break;

		case AddressMode::DIR:
			pd = p + current.bVal;
			break;

		case AddressMode::AIN:
		{
			ProgramPtr tmp = p + current.bVal;
			pd = tmp + tmp->aVal;
			break;
		}

		case AddressMode::BIN:
		{
			ProgramPtr tmp = p + current.bVal;
			pd = tmp + tmp->bVal;
			break;
		}
	}

	dst = *pd;

	//execute current instruction
	switch(current.op)
	{
	case OpCode::KIL:
		report.killProcess();
		break;

	case OpCode::FRK:
		proc.push(++p);
		if(proc.size() < maxProcesses_)
		{
			proc.push(ps);
			report.createProcess();
		}
		break;

	case OpCode::NOP:
		proc.push(++p);
		break;

	case OpCode::MOV:
		switch(current.mod)
		{
		case Modifier::A:
			pd->aVal = src.aVal;
			break;

		case Modifier::B:
			pd->bVal = src.bVal;
			break;

		case Modifier::AB:
			pd->bVal = src.aVal;
			break;

		case Modifier::BA:
			pd->aVal = src.bVal;
			break;

		case Modifier::F:
			pd->aVal = src.aVal;
			pd->bVal = src.bVal;
			break;

		case Modifier::X:
			pd->aVal = src.bVal;
			pd->bVal = src.aVal;
			break;

		case Modifier::I:
			*pd = src;
			break;
		}
		report.read(ps.pos());
		report.write(pd.pos());
		proc.push(++p);
		break;

	case OpCode::ADD:
		switch (current.mod)
		{
		case Modifier::A:
			pd->aVal = (dst.aVal + src.aVal) % core_.size_;
			break;

		case Modifier::B:
			pd->bVal = (dst.bVal + src.bVal) % core_.size_;
			break;

		case Modifier::AB:
			pd->bVal = (dst.bVal + src.aVal) % core_.size_;
			break;

		case Modifier::BA:
			pd->aVal = (dst.aVal + src.bVal) % core_.size_;
			break;

		case Modifier::X:
			pd->aVal = (dst.aVal + src.bVal) % core_.size_;
			pd->bVal = (dst.bVal + src.aVal) % core_.size_;
			break;

		case Modifier::F:
		case Modifier::I:
			pd->aVal = (dst.aVal + src.aVal) % core_.size_;
			pd->bVal = (dst.bVal + src.bVal) % core_.size_;
			break;
		}
		report.read(ps.pos());
		report.write(pd.pos());
		proc.push(++p);
		break;

	case OpCode::SUB:
		switch(current.mod)
		{
		case Modifier::A:
			pd->aVal = (core_.size_ + dst.aVal - src.aVal) % core_.size_;
			break;

		case Modifier::B:
			pd->bVal = (core_.size_ + dst.bVal - src.bVal) % core_.size_;
			break;

		case Modifier::AB:
			pd->bVal = (core_.size_ + dst.bVal - src.aVal) % core_.size_;
			break;

		case Modifier::BA:
			pd->aVal = (core_.size_ + dst.aVal - src.bVal) % core_.size_;
			break;

		case Modifier::X:
			pd->aVal = (core_.size_ + dst.aVal - src.bVal) % core_.size_;
			pd->bVal = (core_.size_ + dst.bVal - src.aVal) % core_.size_;
			break;

		case Modifier::F:
		case Modifier::I:
			pd->aVal = (core_.size_ + dst.aVal - src.aVal) % core_.size_;
			pd->bVal = (core_.size_ + dst.bVal - src.bVal) % core_.size_;
			break;
		}
		report.read(ps.pos());
		report.write(pd.pos());
		proc.push(++p);
		break;

	case OpCode::MUL:
		switch(current.mod)
		{
		case Modifier::A:
			pd->aVal = (dst.aVal * src.aVal) % core_.size_;
			break;

		case Modifier::B:
			pd->bVal = (dst.bVal * src.bVal) % core_.size_;
			break;

		case Modifier::AB:
			pd->bVal = (dst.bVal * src.aVal) % core_.size_;
			break;

		case Modifier::BA:
			pd->aVal = (dst.aVal * src.bVal) % core_.size_;
			break;

		case Modifier::X:
			pd->aVal = (dst.aVal * src.bVal) % core_.size_;
			pd->bVal = (dst.bVal * src.aVal) % core_.size_;
			break;

		case Modifier::F:
		case Modifier::I:
			pd->aVal = (dst.aVal * src.aVal) % core_.size_;
			pd->bVal = (dst.bVal * src.bVal) % core_.size_;
			break;
		}
		report.read(ps.pos());
		report.write(pd.pos());
		proc.push(++p);
		break;

	case OpCode::DIV:
	{
		report.read(ps.pos());
		bool divZero = false;
		switch(current.mod)
		{
		case Modifier::A:
			if(!src.aVal)
			{
				divZero = true;
				break;
			}
			pd->aVal = dst.aVal / src.aVal;
			report.write(pd.pos());
			break;

		case Modifier::B:
			if(!src.bVal)
			{
				divZero = true;
				break;
			}
			pd->bVal = dst.bVal / src.bVal;
			report.write(pd.pos());
			break;

		case Modifier::AB:
			if(!src.aVal)
			{
				divZero = true;
				break;
			}
			pd->bVal = dst.bVal / src.aVal;
			report.write(pd.pos());
			break;

		case Modifier::BA:
			if(!src.bVal)
			{
				divZero = true;
				break;
			}
			pd->aVal = dst.aVal / src.bVal;
			report.write(pd.pos());
			break;

		case Modifier::X:
			if(src.bVal)
				pd->aVal = dst.aVal / src.bVal;
			if(src.aVal)
				pd->bVal = dst.bVal / src.aVal;
			if(!src.bVal || !src.aVal)
				divZero = true;
			else
				report.write(pd.pos());
			break;

		case Modifier::F:
		case Modifier::I:
			if(src.aVal)
				pd->aVal = dst.aVal / src.aVal;
			if(src.bVal)
				pd->bVal = dst.bVal / src.bVal;
			if(!src.aVal || !src.bVal)
				divZero = true;
			else
				report.write(pd.pos());
			break;
		}
		if(!divZero)
			proc.push(++p);
		else
			report.killProcess();
		break;
	}//case OpCode::DIV

	case OpCode::MOD:
	{
		report.read(ps.pos());
		bool divZero = false;
		switch(current.mod)
		{
		case Modifier::A:
			if(!src.aVal)
			{
				divZero = true;
				break;
			}
			pd->aVal = dst.aVal % src.aVal;
			report.write(pd.pos());
			break;

		case Modifier::B:
			if(!src.bVal)
			{
				divZero = true;
				break;
			}
			pd->bVal = dst.bVal % src.bVal;
			report.write(pd.pos());
			break;

		case Modifier::AB:
			if(!src.aVal)
			{
				divZero = true;
				break;
			}
			pd->bVal = dst.bVal % src.aVal;
			report.write(pd.pos());
			break;

		case Modifier::BA:
			if(!src.bVal)
			{
				divZero = true;
				break;
			}
			pd->aVal = dst.aVal % src.bVal;
			report.write(pd.pos());
			break;

		case Modifier::X:
			if(src.bVal)
				pd->aVal = dst.aVal % src.bVal;
			if(src.aVal)
				pd->bVal = dst.bVal % src.aVal;
			if(!src.bVal || !src.aVal)
				divZero = true;
			else
				report.write(pd.pos());
			break;

		case Modifier::F:
		case Modifier::I:
			if(src.aVal)
				pd->aVal = dst.aVal % src.aVal;
			if(src.bVal)
				pd->bVal = dst.bVal % src.bVal;
			if(!src.aVal || !src.bVal)
				divZero = true;
			else
				report.write(pd.pos());
			break;
		}
		if(!divZero)
			proc.push(++p);
		else
			report.killProcess();
		break;
	}

	case OpCode::JMP:
		proc.push(ps);
		break;

	case OpCode::JMZ:
		switch(current.mod)
		{
		case Modifier::A:
		case Modifier::BA:
			if(!dst.aVal)
				proc.push(ps);
			else
				proc.push(++p);
			break;

		case Modifier::B:
		case Modifier::AB:
			if(!dst.bVal)
				proc.push(ps);
			else
				proc.push(++p);
			break;

		case Modifier::F:
		case Modifier::X:
		case Modifier::I:
			if(!dst.aVal && !dst.bVal)
				proc.push(ps);
			else
				proc.push(++p);
			break;
		}
		report.read(pd.pos());
		break;

	case OpCode::JMN:
		switch(current.mod)
		{
		case Modifier::A:
		case Modifier::BA:
			if(dst.aVal)
				proc.push(ps);
			else
				proc.push(++p);
			break;

		case Modifier::B:
		case Modifier::AB:
			if(dst.bVal)
				proc.push(ps);
			else
				proc.push(++p);
			break;

		case Modifier::F:
		case Modifier::X:
		case Modifier::I:
			if(dst.aVal && dst.bVal)
				proc.push(ps);
			else
				proc.push(++p);
			break;
		}
		report.read(pd.pos());
		break;

	case OpCode::BEQ:
		switch(current.mod)
		{
		case Modifier::A:
			if(dst.aVal == src.aVal)
				proc.push(p+2);
			else
				proc.push(++p);
			break;

		case Modifier::B:
			if(dst.bVal == src.bVal)
				proc.push(p+2);
			else
				proc.push(++p);
			break;

		case Modifier::AB:
			if(dst.bVal == src.aVal)
				proc.push(p+2);
			else
				proc.push(++p);
			break;

		case Modifier::BA:
			if(dst.aVal == src.bVal)
				proc.push(p+2);
			else
				proc.push(++p);
			break;

		case Modifier::F:
			if(dst.aVal == src.aVal &&
					dst.bVal == src.bVal)
				proc.push(p+2);
			else
				proc.push(++p);
			break;

		case Modifier::X:
			if(dst.aVal == src.bVal &&
					dst.bVal == src.aVal)
				proc.push(p+2);
			else
				proc.push(++p);
			break;

		case Modifier::I:
			if(dst == src)
				proc.push(p+2);
			else
				proc.push(++p);
			break;
		}
		report.read(ps.pos());
		report.read(pd.pos());
		break;

	case OpCode::BNE:
		switch(current.mod)
		{
		case Modifier::A:
			if(dst.aVal != src.aVal)
				proc.push(p+2);
			else
				proc.push(++p);
			break;

		case Modifier::B:
			if(dst.bVal != src.bVal)
				proc.push(p+2);
			else
				proc.push(++p);
			break;

		case Modifier::AB:
			if(dst.bVal != src.aVal)
				proc.push(p+2);
			else
				proc.push(++p);
			break;

		case Modifier::BA:
			if(dst.aVal != src.bVal)
				proc.push(p+2);
			else
				proc.push(++p);
			break;

		case Modifier::F:
			if(dst.aVal != src.aVal &&
					dst.bVal != src.bVal)
				proc.push(p+2);
			else
				proc.push(++p);
			break;

		case Modifier::X:
			if(dst.aVal != src.bVal &&
					dst.bVal != src.aVal)
				proc.push(p+2);
			else
				proc.push(++p);
			break;

		case Modifier::I:
			if(dst != src)
				proc.push(p+2);
			else
				proc.push(++p);
			break;
		}
		report.read(ps.pos());
		report.read(pd.pos());
		break;

	case OpCode::BLT:
		switch(current.mod)
		{
		case Modifier::A:
			if(dst.aVal > src.aVal)
				proc.push(p+2);
			else
				proc.push(++p);
			break;

		case Modifier::B:
			if(dst.bVal > src.aVal)
				proc.push(p+2);
			else
				proc.push(++p);
			break;

		case Modifier::AB:
			if(dst.bVal > src.aVal)
				proc.push(p+2);
			else
				proc.push(++p);
			break;

		case Modifier::BA:
			if(dst.aVal > src.bVal)
				proc.push(p+2);
			else
				proc.push(++p);
			break;

		case Modifier::X:
			if(dst.aVal > src.bVal &&
					dst.bVal > src.aVal)
				proc.push(p+2);
			else
				proc.push(++p);
			break;

		case Modifier::F:
		case Modifier::I:
			if(dst.aVal > src.aVal &&
					dst.bVal > src.bVal)
				proc.push(p+2);
			else
				proc.push(++p);
			break;
		}
		report.read(ps.pos());
		report.read(pd.pos());
		break;
	}//switch
}

void VirtualMachine::loadProgram(const std::vector<Instruction>& v, unsigned int offset, bool isP1)
{
	//TODO EXCEPTION
	if(v.size() > core_.size_)
		throw std::invalid_argument("Too many instructions in loaded program");

	ProgramPtr p = core_.begin() + offset;

	if(isP1)
	{
		if(loaded_p1_)
			throw std::runtime_error("Player 1 has already been loaded");

		p1_.push(p);
		loaded_p1_ = true;
	}

	else
	{
		if(loaded_p2_)
			throw std::runtime_error("Player 2 has already been loaded");

		p2_.push(p);
		loaded_p2_ = true;
	}

	//load instructions into core
	for(const auto& ins : v)
		*(p++) = ins;
}

void VirtualMachine::loadProgram(const char* fname, unsigned int offset, bool isP1)
{
	std::ifstream fin(fname, std::ifstream::in |
					  std::ifstream::binary);

	Instruction ins;

	ProgramPtr p = core_.begin() + offset;

	if(isP1)
	{
		if(loaded_p1_)
			throw std::runtime_error("Player 1 has already been loaded");

		p1_.push(p);
		loaded_p1_ = true;
	}

	else
	{
		if(loaded_p2_)
			throw std::runtime_error("Player 2 has already been loaded");

		p2_.push(p);
		loaded_p2_ = true;
	}

	do
	{
		fin.read(reinterpret_cast<char*>(&ins.op), sizeof(OpCode));
		fin.read(reinterpret_cast<char*>(&ins.mod), sizeof(Modifier));
		fin.read(reinterpret_cast<char*>(&ins.aMode), sizeof(AddressMode));
		fin.read(reinterpret_cast<char*>(&ins.bMode), sizeof(AddressMode));
		fin.read(reinterpret_cast<char*>(&ins.aVal), sizeof(unsigned int));
		fin.read(reinterpret_cast<char*>(&ins.bVal), sizeof(unsigned int));

		if(fin.fail())
			break;

		else
			*p = ins;

		++p;

	}while(!fin.eof());
}

void VirtualMachine::executeCycle()
{
	if(currentCycle_ >= maxCycles_)
		return;

	++currentCycle_;

	p1Report_.clear();
	p2Report_.clear();

	executeInstruction(p1_, p1Report_);

	if(p1_.empty())
	{
		StatReport::setState(StatReport::P2_WON);

		return;
	}

	executeInstruction(p2_, p2Report_);

	if(p2_.empty())
	{
		StatReport::setState(StatReport::P1_WON);

		return;
	}

	if(currentCycle_ >= maxCycles_)
		StatReport::setState(StatReport::DRAW);
}

void VirtualMachine::reset()
{
	core_ = Core(core_.getSize());

	currentCycle_ = 0;

	loaded_p1_ = false;
	loaded_p2_ = false;

	while(!p1_.empty())
		p1_.pop();

	while(!p2_.empty())
		p2_.pop();
}

unsigned int VirtualMachine::getCoreSize() const
{
	return core_.size_;
}

bool VirtualMachine::isLoadedP1() const
{
	return loaded_p1_;
}

bool VirtualMachine::isLoadedP2() const
{
	return loaded_p2_;
}

VirtualMachine::StatReport& VirtualMachine::getP1Report()
{
	return p1Report_;
}

VirtualMachine::StatReport& VirtualMachine::getP2Report()
{
	return p2Report_;
}

Core::Core(unsigned int s) : size_(s)
{
	if(!size_)
		throw std::invalid_argument("Core size cannot be zero");

	memory_ = std::vector<Instruction>(size_, Instruction());
}

ProgramPtr Core::begin()
{
	return ProgramPtr(memory_.begin(), *this);
}

unsigned int Core::getSize() const
{
	return size_;
}

ProgramPtr& ProgramPtr::operator=(const ProgramPtr& other)
{
	it_ = other.it_;
	ref_ = other.ref_;

	return *this;
}

Instruction& ProgramPtr::operator*()
{
	return *it_;
}

Instruction* ProgramPtr::operator->()
{
	return it_.operator->();
}

ProgramPtr ProgramPtr::operator+(unsigned int n)
{
	ProgramPtr result = *this;

	result += n;

	return result;
}

ProgramPtr& ProgramPtr::operator+=(unsigned int n)
{
	std::ptrdiff_t d = it_ - ref_.memory_.begin();

	d = (d + n) % ref_.size_;

	it_ = ref_.memory_.begin() + d;

	return *this;
}

ProgramPtr& ProgramPtr::operator++()
{
	if(it_ == ref_.memory_.end() - 1)
		it_ = ref_.memory_.begin();

	else
		++it_;

	return *this;
}

ProgramPtr ProgramPtr::operator++(int)
{
	ProgramPtr ret = *this;

	++(*this);

	return ret;
}

unsigned int ProgramPtr::pos() const
{
	return it_ - ref_.memory_.begin();
}

bool Instruction::operator==(const Instruction& other) const
{
	return op == other.op &&
			mod == other.mod &&
			aMode == other.aMode &&
			aVal == other.aVal &&
			bMode == other.bMode &&
			bVal == other.bVal;
}

bool Instruction::operator!=(const Instruction& other) const
{
	return !operator==(other);
}

StatReport::StatReport() : procCount_(1)
{

}

void StatReport::exec(unsigned int adr, const Instruction & ins)
{
	executedAdr_ = adr;
	ins_ = ins;
}

void StatReport::read(unsigned int adr)
{
	readAdrs_.push_back(adr);
}

void StatReport::write(unsigned int adr)
{
	writeAdr_ = adr;
}

void StatReport::createProcess()
{
	++procCount_;
}

void StatReport::killProcess()
{
	--procCount_;
}

void StatReport::setState(StatReport::RoundState rs)
{
	state_ = rs;
}

void StatReport::clear()
{
	readAdrs_.clear();

	state_ = ONGOING;
}

std::string StatReport::toString()
{
	return op[ins_.op] + "." +
			mod[ins_.mod] + "\t" +
			address[ins_.aMode] + std::to_string(ins_.aVal) + "\t" +
			address[ins_.bMode] + std::to_string(ins_.bVal);
}

unsigned int StatReport::getProcessCount() const
{
	return procCount_;
}

unsigned int StatReport::getExecutedAdr() const
{
	return executedAdr_;
}

unsigned int StatReport::getWriteAdr() const
{
	return writeAdr_;
}

StatReport::RoundState StatReport::getState()
{
	return state_;
}
