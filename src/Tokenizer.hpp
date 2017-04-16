#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <boost/tokenizer.hpp>

class Tokenizer
{
public:

	Tokenizer() = delete;

	Tokenizer(const std::string&,
			  const char*,
			  const char*,
			  bool);

	void assign(const std::string&);

	std::string next();

	bool isToken();

	void reset();

private:

	std::string toLCase(const std::string&);

	boost::char_separator<char> sep_;

	bool forceLowercase_;

	boost::tokenizer< boost::char_separator<char> > t_;

	boost::tokenizer< boost::char_separator<char> >::iterator it_;
};

#endif //TOKENIZER_HPP
