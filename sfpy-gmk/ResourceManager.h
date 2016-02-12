#pragma once

#include <exception>
#include <memory>
#include <string>
#include <map>

/*
ResourceManager:
Templated class that stores a map of a given type of resource with an identifier for easy storage/retrieval.
Suitable only for SFML resouces that have a .loadFromFile function.
*/

template <typename Resource, typename Identifier>
class ResourceManager{
public:
	ResourceManager();
	void setDirectory(const std::string& path);
	void load(const Identifier& identifier, const std::string& filename);
	Resource& get(const Identifier& identifier);
private:
	std::string directory;
	std::map<Identifier, std::unique_ptr<Resource>> resources_;
};

#include "ResourceManager.inl"