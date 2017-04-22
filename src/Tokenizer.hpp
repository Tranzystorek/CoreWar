#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <vector>
#include <boost/tokenizer.hpp>

class Tokenizer
{
public:

	Tokenizer();

	Tokenizer(const std::string&,
			  const char*,
			  const char*,
			  bool);

	void assign(const std::string&);

	std::string next();

	bool isToken();

private:

	std::string toLCase(const std::string&);

	void nextToken();

	//boost::char_separator<char> sep_;

	std::string text_;

	std::string::const_iterator strit_;

	std::vector<std::string> tokens_;

	unsigned int index_;

	std::string dropped_;

	std::string kept_;

	bool forceLowercase_;

	bool processed_;

	//boost::tokenizer< boost::char_separator<char> > t_;

	//boost::tokenizer< boost::char_separator<char> >::iterator it_;
};

#endif //TOKENIZER_HPP
