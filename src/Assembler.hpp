#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include "VirtualMachine.hpp"

#include <unordered_map>

#include <fstream>

class Assembler
{
	using uint = unsigned int;

	using dictionary = std::unordered_map<std::string, int>;

public:

	static Assembler& getInstance();

	void openFile(const char*);

	void assembly();

	const std::vector<VirtualMachine::Core::Instruction>& getInstructions();

private:

	using Instruction = VirtualMachine::Core::Instruction;

	Assembler();

	Assembler(const Assembler&) = delete;
	Assembler& operator=(const Assembler&) = delete;

	std::vector<std::string> readLabels();

	void readInstructions(const std::vector<std::string>&);

	std::vector<Instruction> assembledInstructions_;

	void resetFilePos();

	unsigned int normalize(int, unsigned int);

	bool assembled_;

	std::ifstream fin_;

	dictionary labels_;
};

#endif // ASSEMBLER_HPP
