#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include "VirtualMachine.hpp"

#include <unordered_map>
#include <utility>

#include <fstream>

class Assembler
{
	using uint = unsigned int;

	using dictionary = std::unordered_map<std::string, int>;

public:

	static Assembler& getInstance();

	void openFile(const char*);

	bool assembly();

	const std::vector<VirtualMachine::Core::Instruction>& getInstructions();

	void toFile(const char*);

	bool isAssembled();

private:

	using Instruction = VirtualMachine::Core::Instruction;

	Assembler();

	Assembler(const Assembler&) = delete;
	Assembler& operator=(const Assembler&) = delete;

	bool readLabels();

	bool readInstructions();

	std::vector<std::pair<uint, std::string>> instructionLines_;
	std::vector<Instruction> assembledInstructions_;

	void compilationError(const std::string&, unsigned int);

	unsigned int normalize(int, unsigned int = 8000);

	bool assembled_;

	std::ifstream fin_;

	std::string fname_;

	dictionary labels_;
};

#endif // ASSEMBLER_HPP
