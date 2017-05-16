#include "Assembler.hpp"

#include "Tokenizer.hpp"

#include <stdexcept>
#include <algorithm>
#include <string>
#include <iostream>

typedef VirtualMachine::Core::Instruction Instruction;

Assembler& Assembler::getInstance()
{
	static Assembler inst;

	return inst;
}

Assembler::Assembler() : assembled_(false)
{}

void Assembler::openFile(const char* fname)
{
	if(fin_.is_open())
	{
		fin_.close();

		instructionLines_.clear();
		assembledInstructions_.clear();

		labels_.clear();

		fname_.clear();

		assembled_ = false;
	}

	try
	{
		fin_.open(fname);
	}
	catch(const std::ifstream::failure& e)
	{
		std::cerr << e.what() << std::endl;
		return;
	}

	fname_.assign(fname);
}

bool Assembler::assembly()
{
	if(!fin_.is_open())
	{
		std::cerr << "No file is open" << std::endl;

		return false;
	}

	return assembled_ = readLabels() & readInstructions();
}

const std::vector<Instruction>& Assembler::getInstructions()
{
	return assembledInstructions_;
}

void Assembler::toFile(const char* fname)
{
	//TODO EXCEPTION
	if(!assembled_)
		throw;

	std::ofstream fout(fname, std::ofstream::out |
					   std::ofstream::binary |
					   std::ofstream::trunc);

	for(auto& ins : assembledInstructions_)
	{
		fout.write(reinterpret_cast<const char*>(&ins.op), sizeof(Instruction::OpCode));
		fout.write(reinterpret_cast<const char*>(&ins.mod), sizeof(Instruction::Modifier));
		fout.write(reinterpret_cast<const char*>(&ins.aMode), sizeof(Instruction::AddressMode));
		fout.write(reinterpret_cast<const char*>(&ins.bMode), sizeof(Instruction::AddressMode));
		fout.write(reinterpret_cast<const char*>(&ins.aVal), sizeof(unsigned int));
		fout.write(reinterpret_cast<const char*>(&ins.bVal), sizeof(unsigned int));
	}
}

bool Assembler::isAssembled()
{
	return assembled_;
}

unsigned int Assembler::normalize(int n, unsigned int coresize)
{
	//TODO EXCEPTION
	if(!coresize)
		throw;

	if(n < 0)
		return static_cast<unsigned int>(coresize - (-n) % coresize);

	else
		return static_cast<unsigned int>(n % coresize);
}

bool Assembler::readLabels()
{
	std::string line;
	std::string label;
	std::string fragment;

	Tokenizer t(std::string(), " \t", ";:", true);

	int ino = 0;
	unsigned int lno = 0;

	bool success = true;

	while( std::getline(fin_, line) )
	{
		std::string newline;

		++lno;

		t.assign(line);

		if(!t.isToken())
			continue;

		label = t.next();

		if(label == ";")
			continue;

		if(!t.isToken())
		{
			instructionLines_.push_back(std::make_pair(lno, line));

			++ino;
			continue;
		}

		//second token == ':'
		if((fragment = t.next()) == ":")
		{
			if(labels_.count(label))
			{
				compilationError("Label already present in file: \'" + label + "\'", lno);

				success = false;
			}

			else
				labels_[label] = ino;

			if(t.isToken())
			{
				if( (fragment = t.next()) == ";")
					continue;

				else
					newline = fragment + " ";

				while(t.isToken())
				{
					if((fragment = t.next()) == ";")
						break;

					newline += fragment + " ";
				}

				instructionLines_.push_back(std::make_pair(lno, newline));

				++ino;
			}
		}

		//second token != ':'
		else
		{
			newline = label + " ";

			if(fragment != ";")
			{
				newline += fragment + " ";

				while(t.isToken())
				{
					if((fragment = t.next()) == ";")
						break;

					newline += fragment + " ";
				}
			}

			instructionLines_.push_back(std::make_pair(lno, newline));

			++ino;
		}
	}//while

	return success;
}

bool Assembler::readInstructions()
{
	static std::vector<std::string> op = {"kil", "frk", "nop", "mov", "add",
										  "sub", "mul", "div", "mod", "jmp",
										  "jmz", "jmn", "beq", "bne", "blt"};
	static std::vector<std::string> mod = {"a", "b", "ab", "ba", "f", "x", "i"};
	static std::vector<std::string> address = {"#", "$", "*", "@"};

	bool success = true;

	Instruction ins;

	Tokenizer t(std::string(), " ", "#$@*.,", false);

	const unsigned int vsize = instructionLines_.size();

	std::string token;

	//analyze instructions
	for(unsigned int i = 0; i < vsize; ++i)
	{
		bool defaultMod = false;
		std::vector<std::string>::iterator it;

		t.assign(instructionLines_[i].second);

		it = std::find(op.begin(), op.end(), t.next());

		if(it != op.end())
			ins.op = static_cast<Instruction::OpCode>(it - op.begin());

		else
		{
			compilationError("Invalid instruction", instructionLines_[i].first);

			success = false;
			continue;
		}

		if(!t.isToken())
		{
			compilationError("No instruction arguments specified", instructionLines_[i].first);

			success = false;
			continue;
		}

		if( (token = t.next()) != "." )
		{
			switch(ins.op)
			{
			case Instruction::OpCode::KIL:
			case Instruction::OpCode::NOP:
				ins.mod = Instruction::Modifier::F;
				break;

			case Instruction::OpCode::FRK:
			case Instruction::OpCode::JMP:
			case Instruction::OpCode::JMZ:
			case Instruction::OpCode::JMN:
				ins.mod = Instruction::Modifier::B;
				break;

			default:
				break;
			}

			defaultMod = true;
		}

		else
		{
			if(!t.isToken())
			{
				compilationError("Expected instruction modifier after \'.\'", instructionLines_[i].first);

				success = false;
				continue;
			}

			it = std::find(mod.begin(), mod.end(), t.next());

			if(it != mod.end())
				ins.mod = static_cast<Instruction::Modifier>(it - mod.begin());

			else
			{
				compilationError("Invalid instruction modifier", instructionLines_[i].first);

				success = false;
				continue;
			}
		}

		if(!t.isToken() && ins.op != Instruction::OpCode::JMP &&
				ins.op != Instruction::OpCode::FRK)
		{
			compilationError("Too few instruction arguments specified", instructionLines_[i].first);

			success = false;
			continue;
		}

		//analyze A-field
		if(!defaultMod)
			token = t.next();

		it = std::find(address.begin(), address.end(), token);

		if(it != address.end())
		{
			ins.aMode = static_cast<Instruction::AddressMode>(it - address.begin());

			if(!t.isToken())
			{
				compilationError("Missing A-Value", instructionLines_[i].first);

				success = false;
				continue;
			}

			token = t.next();

			size_t pos = 0;
			int val;
			bool label = false;

			try
			{
				val = std::stoi(token, &pos);
			}
			catch(std::invalid_argument& e)
			{
				if(labels_.count(token))
				{
					if(ins.aMode != Instruction::AddressMode::IMM)
					{
						ins.aVal = normalize(labels_[token] - i);
						label = true;
					}

					else
					{
						compilationError("Label address cannot be immediate", instructionLines_[i].first);

						success = false;
						continue;
					}
				}

				else
				{
					compilationError("Label \'" + token + "\' does not exist", instructionLines_[i].first);

					success = false;
					continue;
				}
			}

			if(pos == token.size())
				ins.aVal = normalize(val);

			else if(!label)
			{
				compilationError("Invalid label", instructionLines_[i].first);

				success = false;
				continue;
			}
		}

		else
		{
			ins.aMode = Instruction::AddressMode::DIR;

			size_t pos = 0;
			int val;
			bool label = false;

			try
			{
				val = std::stoi(token, &pos);
			}
			catch(std::invalid_argument& e)
			{
				if(labels_.count(token))
				{
					ins.aVal = normalize(labels_[token] - i);
					label = true;
				}

				else
				{
					compilationError("Label \'" + token + "\' does not exist", instructionLines_[i].first);

					success = false;
					continue;
				}
			}

			if(pos == token.size())
				ins.aVal = normalize(val);

			else if(!label)
			{
				compilationError("Invalid label", instructionLines_[i].first);

				success = false;
				continue;
			}
		}

		if(!t.isToken())
		{
			if(ins.op == Instruction::OpCode::FRK ||
					ins.op == Instruction::OpCode::JMP)
			{
				ins.bMode = Instruction::AddressMode::IMM;
				ins.bVal = 0;

				assembledInstructions_.push_back(ins);

				continue;
			}
		}

		//look for a comma
		token = t.next();

		if(token != ",")
		{
			compilationError("Expected \',\' before B-Value", instructionLines_[i].first);

			success = false;
			continue;
		}

		if(!t.isToken())
		{
			compilationError("Wrong number of arguments", instructionLines_[i].first);

			success = false;
			continue;
		}

		//analyze B-field
		token = t.next();

		it = std::find(address.begin(), address.end(), token);

		if(it != address.end())
		{
			ins.bMode = static_cast<Instruction::AddressMode>(it - address.begin());

			if(!t.isToken())
			{
				compilationError("Missing B-Value", instructionLines_[i].first);

				success = false;
				continue;
			}

			token = t.next();

			size_t pos = 0;
			int val;
			bool label = false;

			try
			{
				val = std::stoi(token, &pos);
			}
			catch(std::invalid_argument& e)
			{
				if(labels_.count(token))
				{
					if(ins.bMode != Instruction::AddressMode::IMM)
					{
						ins.bVal = normalize(labels_[token] - i);
						label = true;
					}

					else
					{
						compilationError("Label address cannot be immediate", instructionLines_[i].first);

						success = false;
						continue;
					}
				}

				else
				{
					compilationError("Label \'" + token + "\' does not exist", instructionLines_[i].first);

					success = false;
					continue;
				}
			}

			if(pos == token.size())
			{
				ins.bVal = normalize(val);
			}

			else if(!label)
			{
				compilationError("Invalid label", instructionLines_[i].first);

				success = false;
				continue;
			}
		}

		else
		{
			ins.bMode = Instruction::AddressMode::DIR;

			size_t pos = 0;
			int val;
			bool label = false;

			try
			{
				val = std::stoi(token, &pos);
			}
			catch(std::invalid_argument& e)
			{
				if(labels_.count(token))
				{
					ins.bVal = normalize(labels_[token] - i);
					label = true;
				}

				else
				{
					compilationError("Label \'" + token + "\' does not exist", instructionLines_[i].first);

					success = false;
					continue;
				}
			}

			if(pos == token.size())
				ins.bVal = normalize(val);

			else if(!label)
			{
				compilationError("Invalid label", instructionLines_[i].first);

				success = false;
				continue;
			}
		}

		if(defaultMod)
		{
			switch(ins.op)
			{
			case Instruction::OpCode::MOV:
			case Instruction::OpCode::BEQ:
			case Instruction::OpCode::BNE:
				if(ins.aMode == Instruction::AddressMode::IMM)
					ins.mod = Instruction::Modifier::AB;
				else if(ins.bMode == Instruction::AddressMode::IMM)
					ins.mod = Instruction::Modifier::B;
				else
					ins.mod = Instruction::Modifier::I;
				break;

			case Instruction::OpCode::ADD:
			case Instruction::OpCode::SUB:
			case Instruction::OpCode::MUL:
			case Instruction::OpCode::DIV:
			case Instruction::OpCode::MOD:
				if(ins.aMode == Instruction::AddressMode::IMM)
					ins.mod = Instruction::Modifier::AB;
				else if(ins.bMode == Instruction::AddressMode::IMM)
					ins.mod = Instruction::Modifier::B;
				else
					ins.mod = Instruction::Modifier::F;
				break;

			case Instruction::OpCode::BLT:
				if(ins.aMode == Instruction::AddressMode::IMM)
					ins.mod = Instruction::Modifier::AB;
				else
					ins.mod = Instruction::Modifier::B;
				break;

			default:
				break;
			}
		}

		if(t.isToken())
		{
			compilationError("Too many arguments", instructionLines_[i].first);

			success = false;
			continue;
		}

		if(success)
			assembledInstructions_.push_back(ins);
	}//for

	if(!success)
		assembledInstructions_.clear();

	return success;
}

void Assembler::compilationError(const std::string & err, unsigned int lnumber)
{
	std::cerr << fname_ << ':' << lnumber << ": " << err << std::endl;
}
