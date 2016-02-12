#pragma once

#include <string>

/*
Script:
Stores properties of a script.
*/

class Script{
public:
	Script(std::string name, std::string filename); 
	~Script();

	std::string getName() const;
	std::string getFilename() const;
	std::string getImportName() const;
private:
	std::string name_;
	std::string filename_;

};