#include "VirtualMachine.hpp"

#include <cstddef>

VirtualMachine::Core::Core(unsigned int s) : size_(s)
{
	memory_ = new Instruction[size_]{};
}

VirtualMachine::Core::~Core()
{
	delete[] memory_;
}

VirtualMachine::Core::ProgramPtr VirtualMachine::Core::begin() const
{
	return ProgramPtr(memory_, *this);
}

unsigned int VirtualMachine::Core::getSize() const
{
	return size_;
}

VirtualMachine::Core::Instruction& VirtualMachine::Core::ProgramPtr::operator*()
{
	return *ptr_;
}

VirtualMachine::Core::ProgramPtr& VirtualMachine::Core::ProgramPtr::operator+=(int n)
{
	std::ptrdiff_t d = ptr_ - ref_.memory_;

	d += n;

	while(d < 0)
		d += ref_.size_;

	d %= ref_.size_;

	ptr_ = ref_.memory_ + d;

	return *this;
}

VirtualMachine::Core::ProgramPtr& VirtualMachine::Core::ProgramPtr::operator++()
{
	if(ptr_ == ref_.memory_ + ref_.size_ - 1)
		ptr_ = ref_.memory_;

	else
		++ptr_;

	return *this;
}
