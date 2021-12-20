#pragma once


#include <Windows.h>
#include <stdio.h>
#include <time.h>
#include <string>


std::string* splitString(std::string str, char sym) {

	if (str.size() == 0) {

		return NULL;
	}

	size_t pos = 0;
	int parts = 1;

	while ((pos = str.find(sym, pos + 1)) != std::string::npos) {
		parts++;
	}

	std::string* res = new std::string[parts];
	pos = 0;
	size_t pos2;

	for (int i = 0; i < parts - 1; i++) {

		pos2 = str.find(sym, pos + 1);
		res[i] = str.substr(pos, pos2 - pos);
		pos = pos2;

	}

	res[parts - 1] = str.substr(pos + 1);

	if (parts == 1) {


	}

	return res;
}


class ChatMessage {

private:

	char* name;
	char* message;
	//SYSTEMTIME  sysTime;
	time_t dt;
	char* _str;


public:

	ChatMessage() :name{ NULL }, message{ NULL }, dt{ time(NULL) }, _str{ NULL }{

		//GetLocalTime(&sysTime);
	}
	ChatMessage(char* name, char* message) :ChatMessage() {

		setName(name);
		setMessage(message);

	}

	char* getName() {

		return name;
	}
	char* getMessage() {

		return message;
	}
	//SYSTEMTIME getSysTime() {

	//	return sysTime;
	//}

	char* toString() {
		int text_len = strlen(message);
		int name_len = strlen(name);
		char* timestamp = new char[16];
		_itoa(this->dt, timestamp, 10);
		int dt_len = strlen(timestamp);

		if (_str) {
			delete[] _str;

		}
		_str = new char[text_len + 1 + name_len + 1 + dt_len + 1];

		sprintf(_str, "%s%s\t%d", getName(), getMessage(), dt);

		return _str;

	}

	void setName(const char* name) {

		if (!name) {

			return;
		}
		if (this->name) {

			delete this->name;
		}
		this->name = new char[strlen(name)];
		strcpy(this->name, name);
	}
	void setMessage(const char* message) {

		if (!message) {

			return;
		}
		if (this->message) {

			delete this->message;
		}
		this->message = new char[strlen(message)];
		strcpy(this->message, message);
	}

	bool parseString(char* str) {

		std::string* userData = splitString(str, '\t');

		setName(userData[0].c_str());
		setMessage(userData[1].c_str());
		dt = atoi(userData[2].c_str());
		return userData;
	}

	~ChatMessage() {

		if (_str) {

			delete[] _str;

		}
	}

};