#include "VirtualMachine.hpp"

#include <iostream>
#include <stdexcept>
#include <cstddef>

typedef VirtualMachine::Core Core;
typedef VirtualMachine::Core::ProgramPtr ProgramPtr;
typedef VirtualMachine::Core::Instruction Instruction;

typedef VirtualMachine::Core::Instruction::OpCode OpCode;
typedef VirtualMachine::Core::Instruction::Modifier Modifier;
typedef VirtualMachine::Core::Instruction::AddressMode AddressMode;

VirtualMachine::VirtualMachine(unsigned int coresize) : core_(coresize),
														maxCycles_(100000),
                                                        maxProcesses_(64),
														loaded_p1_(false),
														loaded_p2_(false)
{

}

void VirtualMachine::executeInstruction(ProcessQueue& proc)
{
	if(proc.empty())
		throw std::runtime_error("Attempted to obtain process from an empty ProcessQueue");

	static std::string op[] = {"KIL", "FRK", "NOP", "MOV", "ADD", "SUB", "MUL", "DIV", "MOD", "JMP"};
	static std::string mod[] = {"A", "B", "AB", "BA", "F", "X", "I"};
	static std::string address[] = {"#", "$", "*", "@"};

	ProgramPtr p = proc.front();
	proc.pop();

	ProgramPtr ps = p;
	ProgramPtr pd = p;

	//instruction "registers"
	Instruction current = *p;
	Instruction src, dst;

	std::cout << op[current.op] << "." << mod[current.mod] << "\t"
			  << address[current.aMode] << current.aVal << "\t"
			  << address[current.bMode] << current.bVal << std::endl;

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
			break;

		case OpCode::FRK:
			proc.push(++p);
            if(proc.size() < maxProcesses_)
                proc.push(ps);
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
			proc.push(++p);
			break;

		case OpCode::DIV:
			{
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
						break;

					case Modifier::B:
						if(!src.bVal)
						{
							divZero = true;
							break;
						}
						pd->bVal = dst.bVal / src.bVal;
						break;

					case Modifier::AB:
						if(!src.aVal)
						{
							divZero = true;
							break;
						}
						pd->bVal = dst.bVal / src.aVal;
						break;

					case Modifier::BA:
						if(!src.bVal)
						{
							divZero = true;
							break;
						}
						pd->aVal = dst.aVal / src.bVal;
						break;

					case Modifier::X:
						if(src.bVal)
							pd->aVal = dst.aVal / src.bVal;
						if(src.aVal)
							pd->bVal = dst.bVal / src.aVal;
						if(!src.bVal || !src.aVal)
							divZero = true;
						break;

					case Modifier::F:
					case Modifier::I:
						if(src.aVal)
							pd->aVal = dst.aVal / src.aVal;
						if(src.bVal)
							pd->bVal = dst.bVal / src.bVal;
						if(!src.aVal || !src.bVal)
							divZero = true;
						break;
				}
				if(!divZero)
					proc.push(++p);
				break;
			}//case OpCode::DIV

		case OpCode::MOD:
			{
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
						break;

					case Modifier::B:
						if(!src.bVal)
						{
							divZero = true;
							break;
						}
						pd->bVal = dst.bVal % src.bVal;
						break;

					case Modifier::AB:
						if(!src.aVal)
						{
							divZero = true;
							break;
						}
						pd->bVal = dst.bVal % src.aVal;
						break;

					case Modifier::BA:
						if(!src.bVal)
						{
							divZero = true;
							break;
						}
						pd->aVal = dst.aVal % src.bVal;
						break;

					case Modifier::X:
						if(src.bVal)
							pd->aVal = dst.aVal % src.bVal;
						if(src.aVal)
							pd->bVal = dst.bVal % src.aVal;
						if(!src.bVal || !src.aVal)
							divZero = true;
						break;

					case Modifier::F:
					case Modifier::I:
						if(src.aVal)
							pd->aVal = dst.aVal % src.aVal;
						if(src.bVal)
							pd->bVal = dst.bVal % src.bVal;
						if(!src.aVal || !src.bVal)
							divZero = true;
						break;
				}
				if(!divZero)
					proc.push(++p);
				break;
			}

		case OpCode::JMP:
			proc.push(ps);
			break;
	}
}

void VirtualMachine::loadProgram(const std::vector<Instruction>& v, unsigned int offset, bool isP1)
{
	//TODO EXCEPTION
	if(v.size() > core_.size_)
		throw;

	ProgramPtr p = core_.begin() + offset;

	if(isP1)
	{
		//TODO EXCEPTION
		if(loaded_p1_)
			throw;

		p1_.push(p);
		loaded_p1_ = true;
	}

	else
	{
		//TODO EXCEPTION
		if(loaded_p2_)
			throw;

		p2_.push(p);
		loaded_p2_ = true;
	}

	//load instructions into core
	for(const auto& ins : v)
		*(p++) = ins;
}

void VirtualMachine::run()
{
	bool draw = true;

	for(int i = 0; i < maxCycles_; ++i)
	{
		executeInstruction(p1_);

		if(p1_.empty())
		{
            std::cout << "P2 wins!" << std::endl;

			draw = false;

			break;
		}

		executeInstruction(p2_);

		if(p2_.empty())
		{
            std::cout << "P1 wins!" << std::endl;

			draw = false;

			break;
		}
	}//for

	if(draw)
        std::cout << "Draw." << std::endl;
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
