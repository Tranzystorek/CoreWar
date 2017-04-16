#include "Tokenizer.hpp"

#include <cctype>

Tokenizer::Tokenizer(const std::string& s,
					 const char* dropped,
					 const char* kept = "",
					 bool lcase = false)
					 : sep_(dropped, kept),
					   forceLowercase_(lcase),
					   t_(std::string())
{
	if(forceLowercase_)
		t_ = boost::tokenizer< boost::char_separator<char> >(toLCase(s), sep_);

	else
		t_ = boost::tokenizer< boost::char_separator<char> >(s, sep_);

	reset();
}

void Tokenizer::assign(const std::string& s)
{
	if(forceLowercase_)
		t_.assign(toLCase(s));

	else
		t_.assign(s);

	reset();
}

std::string Tokenizer::next()
{
	return *(it_++);
}

bool Tokenizer::isToken()
{
	return it_ != t_.end();
}

void Tokenizer::reset()
{
	it_ = t_.begin();
}

std::string Tokenizer::toLCase(const std::string& s)
{
	std::string conv = s;

	int size = conv.size();

	for(int i=0; i < size; ++i)
		conv[i] = tolower(conv[i]);

	return conv;
}
