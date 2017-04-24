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

Assembler::Assembler()
{}

void Assembler::openFile(const char* fname)
{
	if(fin_.is_open())
	{
		fin_.close();

		assembledInstructions_.clear();
	}

	try
	{
		fin_.open(fname);

		labels_.clear();
	}
	catch(const std::ifstream::failure& e)
	{
		std::cerr << e.what() << std::endl;
	}
}

void Assembler::assembly()
{
	if(!fin_.is_open())
	{
		std::cerr << "No file is open" << std::endl;

		return;
	}

	readInstructions(readLabels());
}

const std::vector<Instruction>& Assembler::getInstructions()
{
	return assembledInstructions_;
}

void Assembler::resetFilePos()
{
	//WATCH OUT FOR CLOSED STREAM

	fin_.seekg(0, fin_.beg);
}

unsigned int Assembler::normalize(int n, unsigned int coresize = 8000)
{
	//TODO EXCEPTION
	if(!coresize)
		throw;

	if(n < 0)
		return static_cast<unsigned int>(coresize - (-n) % coresize);

	else
		return static_cast<unsigned int>(n % coresize);
}

std::vector<std::string> Assembler::readLabels()
{
	std::string line;
	std::string label;

	std::string fragment;
	std::string newline;

	Tokenizer t(std::string(), " \t", ";:.,", true);

	int lno = 0;

	std::vector<std::string> instructions;

	while( std::getline(fin_, line) )
	{
		t.assign(line);

		if(!t.isToken())
			continue;

		label = t.next();

		if(label == ";")
			continue;

		if(!t.isToken())
		{
			instructions.push_back(line);

			++lno;
			continue;
		}

		if((fragment = t.next()) == ":")
		{
			if(labels_.count(label))
			{
				//ERROR
				std::cerr << "Label already present in file: " << label << std::endl;

				throw;

				//THROW EXCEPTION
			}

			labels_[label] = lno;

			if(t.isToken())
			{
				if( (fragment = t.next()) == ";")
					continue;

				else
					newline += fragment;

				while(t.isToken())
				{
					if((fragment = t.next()) == ";")
						break;

					newline += fragment + " ";
				}

				instructions.push_back(newline);

				++lno;
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

			instructions.push_back(newline);

			++lno;
		}
	}//while

	return instructions;
}

void Assembler::readInstructions(const std::vector<std::string>& v)
{
	static std::vector<std::string> op = {"kil", "frk", "nop", "mov", "add", "sub", "mul", "div", "mod", "jmp"};
	static std::vector<std::string> mod = {"a", "b", "ab", "ba", "f", "x", "i"};
	static std::vector<std::string> address = {"#", "$", "*", "@"};

	//std::vector<Instruction> ret;

	Instruction ins;

	Tokenizer t(std::string(), " ", "#$@*.,;", false);

	const unsigned int vsize = v.size();

	std::string token;

	//analyze instructions
    for(unsigned int i = 0; i < vsize; ++i)
	{
		bool defaultMod = false;
		std::vector<std::string>::iterator it;

		t.assign(v[i]);

		it = std::find(op.begin(), op.end(), t.next());

		if(it != op.end())
		{
			ins.op = static_cast<Instruction::OpCode>(it - op.begin());
		}

		else
		{
			//ERROR
		}

		if(!t.isToken())
		{
			//ERROR
		}

		if( (token = t.next()) != "." )
		{
			defaultMod = true;
		}

		else
		{
			if(!t.isToken())
			{
				//ERROR
			}

			it = std::find(mod.begin(), mod.end(), t.next());

			if(it != mod.end())
			{
				ins.mod = static_cast<Instruction::Modifier>(it - mod.begin());
			}

			else
			{
				//ERROR
			}
		}

		if(!t.isToken())
		{
			//ERROR
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
				//ERROR
			}

			token = t.next();

			size_t pos = 0;
			int val;

			try
			{
				val = std::stoi(token, &pos);
			}
			catch(std::invalid_argument& e)
			{
				//ERROR
			}

			if(pos == token.size())
			{
				ins.aVal = normalize(val);
			}

			else
			{
				//ERROR
			}
		}

		else
		{
			ins.aMode = Instruction::AddressMode::DIR;

			size_t pos = 0;
			int val;

			try
			{
				val = std::stoi(token, &pos);
			}
			catch(std::invalid_argument& e)
			{
				if(labels_.count(token))
				{
					ins.aVal = normalize(labels_[token] - i);
				}

				else
				{
					//ERROR
				}
			}

			if(pos == token.size())
			{
				ins.aVal = normalize(val);
			}

			else
			{
				//ERROR
			}
		}

		if(!t.isToken())
		{
			//ERROR
		}

		//look for a comma
		token = t.next();

		if(token != ",")
		{
			//ERROR
		}

		if(!t.isToken())
		{
			if(ins.op == Instruction::OpCode::FRK ||
			   ins.op == Instruction::OpCode::JMP)
			{
				ins.bVal = 0;
			}

			//ERROR
		}

		//analyze B-field
		token = t.next();

		it = std::find(address.begin(), address.end(), token);

		if(it != address.end())
		{
			ins.bMode = static_cast<Instruction::AddressMode>(it - address.begin());

			if(!t.isToken())
			{
				//ERROR
			}

			token = t.next();

			size_t pos = 0;
			int val;

			try
			{
				val = std::stoi(token, &pos);
			}
			catch(std::invalid_argument& e)
			{
				//ERROR
			}

			if(pos == token.size())
			{
				ins.bVal = normalize(val);
			}

			else
			{
				//ERROR
			}
		}

		else
		{
			ins.bMode = Instruction::AddressMode::DIR;

			size_t pos = 0;
			int val;

			try
			{
				val = std::stoi(token, &pos);
			}
			catch(std::invalid_argument& e)
			{
				if(labels_.count(token))
				{
					ins.bVal = normalize(labels_[token] - i);
				}

				else
				{
					//ERROR
				}
			}

			if(pos == token.size())
			{
				ins.bVal = normalize(val);
			}

			else
			{
				//ERROR
			}
		}

		if(defaultMod)
		{
			switch(ins.op)
			{
				case Instruction::OpCode::KIL:
				case Instruction::OpCode::NOP:
					ins.mod = Instruction::Modifier::F;
					break;

				case Instruction::OpCode::FRK:
				case Instruction::OpCode::JMP:
					ins.mod = Instruction::Modifier::B;
					break;

				case Instruction::OpCode::MOV:
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
			}
		}

		if(t.isToken())
		{
			//ERROR
		}

		assembledInstructions_.push_back(ins);
	}//for
}
