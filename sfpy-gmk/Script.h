#pragma once

#include <string>

class Script{
public:
	Script(std::string name, std::string filename); //todo more properties
	~Script();

	std::string getName() const;
	std::string getFilename() const;
	std::string getImportName() const;
private:
	std::string name_;
	std::string filename_;

};