#ifndef WORD_PARSER_H_
#define WORD_PARSER_H_

class WordParser{
	private:
		std::string line;
		int index;
		bool notBlank;
	public:
		WordParser(std::string line);
		std::string nextWord();
		bool hasNext();
};

#endif
