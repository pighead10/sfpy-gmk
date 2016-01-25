#pragma once

#include <vector>
#include <map>
#include <SFML/Graphics.hpp>
#include <boost/noncopyable.hpp>
#include <boost/python.hpp>
#include <memory>
#include "ResourceManager.h"

class Entity;
typedef std::map<std::string, std::string> StringMap;
typedef std::vector<std::pair<boost::python::object, std::unique_ptr<Entity>>> ObjectList;

class Game :  private boost::noncopyable{
public:
	Game();
	void add(boost::python::object entity);
	void update(int frametime);
	void render(sf::RenderTarget* target);

	void addThread(boost::python::object thread);

	void start_gameloop();
	void load(std::string gamefile);
	void start_py();

	void setPyInterpreterState(PyInterpreterState* interpreter_state);

	ResourceManager<sf::Texture, std::string>* getTextures();
	ObjectList* getObjects();

	enum GAME_EVENT{
		Collision,
		KeyPressed,
		KeyReleased,
		MousePressed,
		MouseReleased
	};

	enum MOUSE_BUTTON{
		Button1,
		Button2,
		Button3
	};

	void fireGlobalEvent(GAME_EVENT evt,boost::python::tuple args);
private:
	void load_textures();
	std::string generatePyClassString(const StringMap& props) const;

	enum CFG_TYPE{
		TYPE_NONE,
		TYPE_OBJ,
		TYPE_GAME,
		TYPE_TEX
	};

	enum CFG_PREFIX{
		PREFIX_NONE,
		PREFIX_TYPE,
		PREFIX_PROP,
		PREFIX_NAME,
	};
	ResourceManager<sf::Texture, std::string> textures_;

	std::string game_name_;
	std::vector<StringMap> obj_props_;
	std::vector<StringMap> tex_props_;
	ObjectList objects_;
	std::vector<boost::python::object> threads_;

	PyInterpreterState* interpreter_state_;
};