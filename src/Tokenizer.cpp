#include "Tokenizer.hpp"

#include <algorithm>
#include <cctype>

Tokenizer::Tokenizer(const std::string& s,
					 const char* dropped,
					 const char* kept,
					 bool lcase)
	: index_(0),
	  dropped_(dropped),
	  kept_(kept),
	  forceLowercase_(lcase),
	  processed_(false)
{
	if(forceLowercase_)
		text_ = toLCase(s);

	else
		text_ = s;

	strit_ = text_.cbegin();
}

void Tokenizer::assign(const std::string& s)
{
	if(forceLowercase_)
		text_ = toLCase(s);

	else
		text_ = s;

	strit_ = text_.cbegin();

	tokens_.clear();
	index_ = 0;

	processed_ = false;
}

std::string Tokenizer::next()
{
	if(!processed_)
		nextToken();

	return tokens_.at(index_++);
}

bool Tokenizer::isToken()
{
	if( !processed_ && index_ == tokens_.size() )
		return checkToken();

	return (( !processed_ ) || ( index_ != tokens_.size() ));
}

bool Tokenizer::checkToken()
{
	std::string::const_iterator it = strit_;

	while(it != text_.cend())
	{
		std::string::iterator fit = std::find(dropped_.begin(), dropped_.end(), *it);

		if(fit != dropped_.end())
		{
			++it;

			continue;
		}

		return true;
	}

	return false;
}

void Tokenizer::nextToken()
{
	std::string tok;

	std::string::iterator it;

	bool found = false;
	bool inserted = false;

	while(strit_ != text_.cend())
	{
		it = std::find(dropped_.begin(), dropped_.end(), *strit_);

		if( it != dropped_.end() )
		{
			++strit_;

			if(!found)
				continue;

			//tokens_.push_back(tok);
			break;
		}

		it = std::find(kept_.begin(), kept_.end(), *strit_);

		if( it != kept_.end() )
		{
			if(!found)
			{
				tokens_.push_back(std::string() + *(strit_++));
				continue;
			}

			tokens_.push_back(tok);
			tokens_.push_back(std::string() + *(strit_++));
			inserted = true;
			break;
		}

		found = true;
		tok += *(strit_++);
	}//while

	if(found && !inserted)
		tokens_.push_back(tok);

	if(strit_ == text_.cend())
		processed_ = true;
}

std::string Tokenizer::toLCase(const std::string& s)
{
	std::string conv = s;

	int size = conv.size();

	for(int i=0; i < size; ++i)
		conv[i] = tolower(conv[i]);

	return conv;
}
