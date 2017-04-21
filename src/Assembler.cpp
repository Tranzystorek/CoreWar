#include "Assembler.hpp"

#include "Tokenizer.hpp"
#include <iostream>

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

	readLabels();

	resetFilePos();

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

void Assembler::readLabels()
{
	std::string line;
	std::string label;

	Tokenizer t(std::string(), " \t", ";:.", true);

	uint lno = 1;

	while( std::getline(fin_, line) )
	{
		t.assign(line);

		if(!t.isToken())
			continue;

		label = t.next();

		if(!t.isToken())
		{
			++lno;
			continue;
		}

		if(t.next() == ":")
		{
			if(labels_.count(label))
			{
				//ERROR
				std::cerr << "Label already present in file: " << label << std::endl;

				return;

				//THROW EXCEPTION
			}

			labels_[label] = lno;

			if(t.isToken())
				++lno;
		}

		else
			++lno;

	}//while
}
