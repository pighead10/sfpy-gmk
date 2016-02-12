#include "Script.h"

Script::Script(std::string name, std::string filename):name_(name),filename_(filename){
}

Script::~Script(){
}

std::string Script::getFilename() const{
	return filename_;
}

std::string Script::getName() const{
	return name_;
}

std::string Script::getImportName() const{
	//Returns name which should be used by python script to import this script
	//This is simply truncating to not include the extension - "myfile.py" becomes "myfile"
	return filename_.substr(0, filename_.length() - 3);
}