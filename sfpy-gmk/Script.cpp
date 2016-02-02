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
	return filename_.substr(0, filename_.length() - 3);
}