#include "Assembler.hpp"

#include "Tokenizer.hpp"

#include <algorithm>
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
		fin_.close();

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

	//TODO

	for(const auto &l : labels_)
	{
		std::cout << l.first << " " << l.second << std::endl;
	}
}

void Assembler::resetFilePos()
{
	//WATCH OUT FOR CLOSED STREAM

	fin_.seekg(0, fin_.beg);
}

std::vector<std::string> Assembler::readLabels()
{
	std::string line;
	std::string label;

	std::string fragment;
	std::string newline;

	Tokenizer t(std::string(), " \t", ";:.", true);

	uint lno = 0;

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

std::vector<Instruction> Assembler::readInstructions(std::vector<std::string> v)
{
	static std::string op[] = {"kil", "frk", "nop", "mov", "add", "sub", "mul", "div", "mod", "jmp"};
	static std::string mod[] = {"a", "b", "ab", "ba", "f", "x", "i"};
	static std::string address[] = {"#", "$", "@", "*"};

	std::vector<Instruction> ret;

	Instruction ins;

	Tokenizer t(std::string(), " ", "#$@*.,;", false);

	const unsigned int vsize = tokens_.size();

	//analyze instructions
	for(int i = 0; i < vsize; ++i)
	{
		std::string::iterator it;

		t.assign(v[i]);

		if(!t.isToken())
		{
			//TODO
		}

		it = std::find(op.begin(), op.end(), t.next());

		if(it != op.end())
		{
			ins.op = it - op.begin();
		}

		if(!t.isToken())
		{
			//TODO
		}

		if(t.next != ".")
		{
			//TODO
		}
	}

	return ret;
}
