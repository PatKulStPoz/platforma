#include "stringreader.h"


std::string lowercase(std::string str) {
	for (int i = 0; i < str.size(); i++) {
		str[i] = tolower(str[i]);
	}
	return str;
}

ParseError :: ParseError(int pos, std::string msg): pos_(pos), msg_(msg) {};

int ParseError :: pos() {
	return this->pos_;
}
std::string ParseError :: msg() {
	return this->msg_;
}

bool isWhitespace(char chr) {
	return chr == 0 || chr == ' ' || chr == '\n';
}

StringReader :: StringReader(std::string &data): data_(data) {};

int StringReader :: index() {
	return this->index_;
}

void StringReader :: index(int index) {
	this->index_ = index;
}

char StringReader :: read() {
	if (this->index_ < this->data_.size()) {
		return this->data_[this->index_++];
	}
	return 0;
}

void StringReader :: advance() {
	if (this->index_ < this->data_.size()) {
		this->index_++;
	}
}

char StringReader :: peek() {
	if (this->index_ < this->data_.size()) {
		return this->data_[this->index_];
	}
	return 0;
}

bool StringReader :: end() {
	return this->index_ >= this->data_.size();
}

void StringReader :: skipWhitespace() {
	while(isWhitespace(this->peek()) && !this->end()) {
		this->advance();
	}
}

std::string StringReader :: readWord() {
	this->skipWhitespace();
	std::string out = "";
	while (!this->end()) {
		char chr = this->peek();
		if (isWhitespace(chr)) {
			return out;
		}
		this->advance();
		out += chr;
	}
	
	return out;
}

std::string StringReader :: readWordLowercase() {
	this->skipWhitespace();
	std::string out = "";
	while (!this->end()) {
		char chr = this->peek();
		if (isWhitespace(chr)) {
			return out;
		}
		this->advance();
		out += tolower(chr);
	}
	
	return out;
}

std::string StringReader :: readGreedyString() {
	this->skipWhitespace();
	if (!this->end()) {
		auto out = this->data_.substr(this->index_, this->data_.size());
		this->index_ = this->data_.size();
		return out;
	}
	return "";
}

Result<std::string> StringReader :: readQuotedString() {
	this->skipWhitespace();
	int index = this->index();
	char chr = this->read();
	if (chr == 0) {
		return Result(ParseError(index, "Expected String, reached end!"), std::string(""));
	}

	if (chr == '"' || chr == '\'') {
		std::string out = "";

		char splitAt = chr;
		bool skipNext = false;
		while (!this->end()) {
			chr = this->read();
			if (chr == '\\') {
				skipNext = true;
			} else if (!skipNext && chr == splitAt) {
				return out;
			} else {
				skipNext = false;
				out += chr;
			}
		}
		this->index(index);
		return Result(ParseError(index, "Expected \" or ', reached end!"), out);
	} else {
		this->index(index);
		return this->readWord();
	}

}

Result<int> StringReader :: readInt() {
	this->skipWhitespace();
	int out = 0;
	int index = this->index();
	bool negate = false;
	if (this->end()) {
		return Result(ParseError(index, "Expected Integer, reached end!"), out);
	}
	bool hasNumber = false;
	char chr = this->read();
	if (chr == '-') {
		negate = true;
	} else if (chr == '+') {

	} else if (chr >= '0' && chr <= '9') {
		out = chr - '0';
		hasNumber = true;
	} else {
		this->index(index);
		return Result(ParseError(index, "Non-numeric character!"), out);
	}

	while(!this->end()) {
		chr = this->peek();
		if (chr >= '0' && chr <= '9') {
			out = out * 10 + chr - '0';
			hasNumber = true;
			this->advance();
		} else if (chr == '_') {
			this->advance();
			continue;
		} else if (isWhitespace(chr) || hasNumber) {
			break;
		} else {
			int err = this->index() - 1;
			this->index(index);
			return Result(ParseError(err, "Non-numeric character!"), out);
		}
	}
	return negate ? -out : out;
}

Result<uint32_t> StringReader :: readUInt() {
	this->skipWhitespace();
	uint32_t out = 0;
	int index = this->index();
	if (this->end()) {
		return Result(ParseError(index, "Expected Integer, reached end!"), out);
	}
	bool hasNumber = false;
	char chr = this->read();
	if (chr >= '0' && chr <= '9') {
		out = chr - '0';
		hasNumber = true;
	} else {
		this->index(index);
		return Result(ParseError(index, "Non-numeric character!"), out);
	}

	while(!this->end()) {
		chr = this->peek();
		if (chr >= '0' && chr <= '9') {
			out = out * 10 + chr - '0';
			hasNumber = true;
			this->advance();
		} else if (chr == '_') {
			this->advance();
			continue;
		} else if (isWhitespace(chr) || hasNumber) {
			break;
		} else {
			int err = this->index() - 1;
			this->index(index);
			return Result(ParseError(err, "Non-numeric character!"), out);
		}
	}
	return out;
}

Result<double> StringReader :: readDouble() {
	this->skipWhitespace();
	double out = 0;
	int index = this->index();
	bool negate = false;
	if (this->end()) {
		return Result(ParseError(index, "Expected Double, reached end!"), out);
	}
	bool hasNumber = false;
	bool decimal = false;
	char chr = this->read();
	if (chr == '-') {
		negate = true;
	} else if (chr == '+') {

	} else if (chr == '.') {
		decimal = true;
	} else if (chr >= '0' && chr <= '9') {
		out = chr - '0';
		hasNumber = true;
	} else {
		int err = this->index() - 1;
		this->index(index);
		return Result(ParseError(err, "Non-numeric character!"), out);
	}
	double decimalMult = 0.1;

	while(!this->end()) {
		chr = this->read();
		if (chr >= '0' && chr <= '9') {
			if (decimal) {
				out = out + (chr - '0') * decimalMult ;
				decimalMult *= 0.1;
			} else {
				out = out * 10 + chr - '0';
			}
			hasNumber = true;
		} else if (chr == '_') {
			continue;
		} else if (chr == '.' && !decimal) {
			decimal = true;
		} else if (isWhitespace(chr) || hasNumber) {
			break;
		} else {
			int err = this->index() - 1;
			this->index(index);
			return Result(ParseError(err, "Non-numeric character!"), out);
		}
	}
	return negate ? -out : out;
}

Result<bool> StringReader :: readBool() {
	auto word = lowercase(this->readWord());

	if (word == "") {
		return Result(ParseError(this->index(), "Expected Bool, reached end!"), false);
	}

	return word == "true" || word == "yes" || word == "1";
}