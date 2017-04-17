#include "VirtualMachine.hpp"

#include <stdexcept>
#include <cstddef>

typedef VirtualMachine::Core Core;
typedef VirtualMachine::Core::ProgramPtr ProgramPtr;
typedef VirtualMachine::Core::Instruction Instruction;

typedef VirtualMachine::Core::Instruction::OpCode OpCode;
typedef VirtualMachine::Core::Instruction::Modifier Modifier;
typedef VirtualMachine::Core::Instruction::AddressMode AddressMode;

VirtualMachine::VirtualMachine(unsigned int coresize) : core_(coresize)
{

}

void VirtualMachine::executeInstruction(ProcessQueue& proc)
{
	if(proc.empty())
		throw std::runtime_error("Attempted to obtain process from an empty ProcessQueue");

	ProgramPtr p = proc.front();
	proc.pop();

	ProgramPtr pd = p;
	ProgramPtr ps = p;

	//instruction "registers"
	Instruction current = *p;
	Instruction src, dst;

	//determine SRC instruction
	switch(current.aMode)
	{
		case AddressMode::IMM:
			ps = p;
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
			pd = p;
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

	switch(current.op)
	{
		case OpCode::KIL:
			break;

		case OpCode::FRK:
			proc.push(++p);
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
					pd->aVal = (dst.aVal - src.aVal) + core_.size_;
					break;

				case Modifier::B:
					pd->bVal = (dst.bVal - src.bVal) + core_.size_;
					break;

				case Modifier::AB:
					pd->bVal = (dst.bVal - src.aVal) + core_.size_;
					break;

				case Modifier::BA:
					pd->aVal = (dst.aVal - src.bVal) + core_.size_;
					break;

				case Modifier::X:
					pd->aVal = (dst.aVal - src.bVal) + core_.size_;
					pd->bVal = (dst.bVal - src.aVal) + core_.size_;
					break;

				case Modifier::F:
				case Modifier::I:
					pd->aVal = (dst.aVal - src.aVal) + core_.size_;
					pd->bVal = (dst.bVal - src.bVal) + core_.size_;
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

int VirtualMachine::convertValue(int v)
{
	if(v < 0)
		return core_.size_ - (-v) % core_.size_;

	else
		return v % core_.size_;
}

Core::Core(unsigned int s) : size_(s)
{
	if(!size_)
		throw std::invalid_argument("Core size cannot be zero");

	memory_ = new Instruction[size_]{};
}

Core::~Core()
{
	delete[] memory_;
}

ProgramPtr Core::begin() const
{
	return ProgramPtr(memory_, *this);
}

unsigned int Core::getSize() const
{
	return size_;
}

ProgramPtr& ProgramPtr::operator=(const ProgramPtr& other)
{
	ptr_ = other.ptr_;
	ref_ = other.ref_;
}

Instruction& ProgramPtr::operator*()
{
	return *ptr_;
}

Instruction* ProgramPtr::operator->()
{
	return ptr_;
}

ProgramPtr ProgramPtr::operator+(int n)
{
	ProgramPtr result = *this;

	result += n;

	return result;
}

ProgramPtr& ProgramPtr::operator+=(int n)
{
	std::ptrdiff_t d = ptr_ - ref_.memory_;

	d += n;

	if(d < 0)
		d = ref_.size_ - (-d) % ref_.size_;

	else
		d %= ref_.size_;

	ptr_ = ref_.memory_ + d;

	return *this;
}

ProgramPtr& ProgramPtr::operator++()
{
	if(ptr_ == ref_.memory_ + ref_.size_ - 1)
		ptr_ = ref_.memory_;

	else
		++ptr_;

	return *this;
}

ProgramPtr ProgramPtr::operator++(int)
{
	ProgramPtr ret = *this;

	++(*this);

	return ret;
}
