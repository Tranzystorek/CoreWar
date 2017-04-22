#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include "VirtualMachine.hpp"

#include <unordered_map>

#include <fstream>

class Assembler
{
	using uint = unsigned int;

	using dictionary = std::unordered_map<std::string, uint>;

public:

	static Assembler& getInstance();

	void openFile(const char*);

	void assembly();

private:

	using Instruction = VirtualMachine::Core::Instruction;

	Assembler();

	Assembler(const Assembler&) = delete;
	Assembler& operator=(const Assembler&) = delete;

	std::vector<std::string> readLabels();

	std::vector<Instruction> readInstructions(std::vector<std::string>);

	void resetFilePos();

	bool assembled_;

	std::ifstream fin_;

	dictionary labels_;
};

#endif // ASSEMBLER_HPP
