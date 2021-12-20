#pragma once


#include <Windows.h>
#include <stdio.h>
#include <time.h>
#include <string>


std::string* splitString(std::string str, char sym) {

	int parts = 0;

	for (int i = 0; i < str.size(); i++) {

		if (str[i] == '\t') {
			parts++;
		}

	}


	if (str.size() == 0) {

		return NULL;
	}

	size_t pos = 0;
	parts = 1;

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
	time_t dt;
	char* _str;


public:

	ChatMessage() :name{ NULL }, message{ NULL }, dt{ time(NULL) }, _str{ NULL }{

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


	char* toStringDT() {
		int text_len = strlen(message);
		int name_len = strlen(name);
		char* timestamp = new char[16];
		_itoa(this->dt, timestamp, 10);
		int dt_len = strlen(timestamp);

		if (_str) {
			delete[] _str;

		}
		_str = new char[text_len + 1 + name_len + 1 + dt_len + 1];

		sprintf(_str, "%s\t%s\t%d", getName(), getMessage(), dt);

		return _str;

	}

	char* toString() {
		int text_len = strlen(message);
		int name_len = strlen(name);

		if (_str) {
			delete[] _str;

		}
		_str = new char[text_len + 1 + name_len + 1];

		sprintf(_str, "%s\t%s", getName(), getMessage());

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

		return userData;
	}

	bool parseStringDT(char* str) {

		std::string* userData = splitString(str, '\t');

		setName(userData[0].c_str());
		setMessage(userData[1].c_str());
		dt = atoi(userData[2].c_str());
		return userData;
	}

	char* toClientString() {

		if (this->message == NULL || this->name == NULL) {
			return NULL;
		}
		int text_len = strlen(message);
		int name_len = strlen(name);
		tm* t = localtime(&this->dt);
		time_t now_t = time(NULL);
		tm* now = localtime(&now_t);
		
		if (_str) {

			delete[] _str;
		}

		_str = new char[text_len + 1 + name_len + 1 + 32];

		sprintf(_str, "%.2d:%.2d %s:%s", t->tm_hour, t->tm_min, getName(), getMessage());
		return _str;

	}

	char* toDateString() {

		if (this->message == NULL || this->name == NULL) {
			return NULL;
		}
		int text_len = strlen(message);
		int name_len = strlen(name);
		//tm* t = localtime(&this->dt);
		tm* t = new tm;
		localtime_s(t, &this->dt);
		time_t now_t = time(NULL);
		tm* now = localtime(&now_t);

		if (_str) {

			delete[] _str;
		}

		_str = new char[text_len + 1 + name_len + 1 + 32];

		if (t->tm_year == now->tm_year && t->tm_mon == now->tm_mon && t->tm_mday == now->tm_mday) {

			sprintf(_str, "Today at %.2d:%.2d %s:%s", t->tm_hour, t->tm_min, getName(), getMessage());

		}
		else if (t->tm_year == now->tm_year && t->tm_mon == now->tm_mon && t->tm_mday+1 == now->tm_mday) {

			sprintf(_str, "Yesterday at %.2d:%.2d %s:%s", t->tm_hour, t->tm_min, getName(), getMessage());

		}
		else if (t->tm_year == now->tm_year && t->tm_mon == now->tm_mon) {

			sprintf(_str, "%d days ago at %.2d:%.2d %s:%s", now->tm_mday - t->tm_mday, t->tm_hour, t->tm_min, getName(), getMessage());

		}
		else {

			sprintf(_str, "%d:%d:%d at %.2d:%.2d %s:%s", t->tm_mday,t->tm_mon,t->tm_year, t->tm_hour, t->tm_min, getName(), getMessage());

		}
		delete t;
		return _str;


	}

	~ChatMessage() {

		if (_str) {

			delete[] _str;

		}
	}

};