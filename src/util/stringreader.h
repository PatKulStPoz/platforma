#pragma once
#include<string>
#include<iostream>


class ParseError {
    private:
	int pos_;
	std::string msg_;
	public:
	ParseError(int pos, std::string msg);
	int pos();
	std::string msg();
};

template <typename T>
class Result {
    private:
	ParseError err_;
	T result_;
	public:
	Result(T result): err_(ParseError(-1, "")), result_(result) {};
	Result(ParseError err, T result): err_(err), result_(result) {};

	inline bool success() {
		return this->err_.pos() == -1;
	}

	inline T result() {
		return this->result_;
	}

	inline ParseError err() {
		return this->err_;
	}
};

class StringReader {
    private:
	std::string &data_;
	int index_ = 0;
	public:
	StringReader(std::string &data);
	StringReader();

	int index();
	void index(int index);
	void advance();
	void skipWhitespace();

	std::string readWord();
    std::string readWordLowercase();
	std::string readGreedyString();
	Result<std::string> readQuotedString();

	Result<int> readInt();
	Result<uint32_t> readUInt();
	Result<double> readDouble();
	Result<bool> readBool();

	char read();
	char peek();
	bool end();
};