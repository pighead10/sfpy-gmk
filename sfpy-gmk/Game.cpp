#include "Game.h"
#include "Entity.h"
#include <boost/python.hpp>
#include <memory>
#include <sstream>
#include <fstream>
#include <string>
#include "gil.h"

#define MS_PER_UPDATE 1

Game::Game():game_name_(""){
}

void Game::setPyInterpreterState(PyInterpreterState* interpreter_state){
	interpreter_state_ = interpreter_state;
}

void Game::start_gameloop(){
	//Create state for this thread

	//Creates window and updates and renders game every frame
	sf::VideoMode vm(1024, 768);
	sf::RenderWindow window;
	window.create(vm, game_name_);

	sf::Clock clock;
	int previous = clock.getElapsedTime().asMilliseconds();
	int lag = 0;
	bool exit_ = false;


	while (!exit_){
		int current = clock.getElapsedTime().asMilliseconds();
		int elapsed = current - previous;
		previous = current;
		lag += elapsed;

		sf::Event evt;
		while (window.pollEvent(evt)){
			if (evt.type == sf::Event::Closed){
				exit_ = true;
			}
			else if (evt.type == sf::Event::KeyPressed || evt.type == sf::Event::KeyReleased){
				GAME_EVENT game_evt = (evt.type == sf::Event::KeyPressed ? KeyPressed : KeyReleased);
				std::string arg;
				if (evt.key.code >= 0 && evt.key.code <= 25){
					arg = (char)(65 + evt.key.code);
				}
				else if (evt.key.code >= 26 && evt.key.code <= 35){
					arg = std::to_string(evt.key.code - 26);
				}
				else if (evt.key.code == sf::Keyboard::Left){
					arg = "left";
				}
				else if (evt.key.code == sf::Keyboard::Right){
					arg = "right";
				}
				else if (evt.key.code == sf::Keyboard::Up){
					arg = "up";
				}
				else if (evt.key.code == sf::Keyboard::Down){
					arg = "down";
				}

				fireGlobalEvent(game_evt, boost::python::make_tuple(arg));
			}
			else if (evt.type == sf::Event::MouseButtonPressed || evt.type == sf::Event::MouseButtonReleased){
				AcquireGIL gil = AcquireGIL();
				GAME_EVENT game_evt = (evt.type == sf::Event::MouseButtonPressed ? MousePressed : MouseReleased);
				MOUSE_BUTTON but;
				if (evt.mouseButton.button == sf::Mouse::Left){
					but = Button1;
				}
				else if (evt.mouseButton.button == sf::Mouse::Right){
					but = Button2;
				}
				else if (evt.mouseButton.button == sf::Mouse::Middle){
					but = Button3;
				}
				fireGlobalEvent(game_evt, boost::python::make_tuple(but,maths::Vector2(evt.mouseButton.x, evt.mouseButton.y)));
			}
		}

		//Run graphics as fast as possible while keeping physics framerate constant
		//Avoids problems that could arise due to variable time steps
		while (lag >= MS_PER_UPDATE && !exit_){
			//update function requires python API so needs to hold GIL
			AcquireGIL gil = AcquireGIL(); 
			update(MS_PER_UPDATE);
			lag -= MS_PER_UPDATE;
		}
		window.clear(sf::Color::White);
		render(&window);
		window.display();
	}

	window.close();
}

void Game::load(std::string gamefile){
	//Read gamefile to store game properties (objects, sprites, etc)
	using namespace std;

	ifstream fin(gamefile);
	string line;
	CFG_TYPE type = TYPE_NONE;	
	std::string prop;
	CFG_PREFIX prefix = PREFIX_NONE;

	while (getline(fin, line)){	
		if (line.length() > 0 && line[0] != '#'){ //ignore commented lines
			//Check if prefix signifies that the next line is a saved value
			if (prefix == PREFIX_PROP){
				if (type == TYPE_OBJ){
					obj_props_.back()[prop] = line;
					prefix = PREFIX_NONE;
				}
				else if (type == TYPE_TEX){
					tex_props_.back()[prop] = line;
					prefix = PREFIX_NONE;
				}
			}
			else if (prefix == PREFIX_NAME){
				if (type == TYPE_OBJ){
					obj_props_.back()["$name"] = line;
					prefix = PREFIX_NONE;
				}
				else if (type == TYPE_GAME){
					game_name_ = line;
					prefix = PREFIX_NONE;
				}
			}
			else{
				istringstream s(line);
				string word;
				for (int i = 0; s >> word; i++){
					//check if word is a prefix
					if (word == "$type"){
						prefix = PREFIX_TYPE;
					}
					else if (word == "$prop"){
						prefix = PREFIX_PROP;
					}
					else if (word == "$name"){
						prefix = PREFIX_NAME;
					}
					else{
						if (prefix == PREFIX_TYPE){
							if (word == "obj"){
								type = TYPE_OBJ;
								obj_props_.push_back(StringMap());
							}
							else if (word == "game"){
								type = TYPE_GAME;
							}
							else if (word == "tex"){
								type = TYPE_TEX;
								tex_props_.push_back(StringMap());
							}
						}
						else if (prefix == PREFIX_PROP){
							prop = word;
						}
					}
				}
			}
		}
	}

	load_textures();
}

ObjectList* Game::getObjects(){
	return &objects_;
}

void Game::load_textures(){
	for (auto& it : tex_props_){
		textures_.load(it["name"], it["filename"]);
	}
}

ResourceManager<sf::Texture, std::string>* Game::getTextures(){
	return &textures_;
}

std::string Game::generatePyClassString(const StringMap& props) const{
	//Create Python code defining class, creating a constructor that sets values correspondings to props
	//Also, define function creating thread for running events
	std::string s =
		"class " + props.at("$name") + "(Entity):		\n"
		"	def __init__(self):							\n"
		"		Entity.__init__(self)					\n"
		"		self._init_entity(game)					\n";

	for (auto& it : props){
		if (it.first[0] != '$'){
			s += "		self." + it.first + " = " + it.second + "\n";
		}
	}

	//Users can optionally create an init function as well.
	s +=
		"		try:							\n"
		"			self.init()					\n"
		"		except:							\n"
		"			print('init not found')		\n";

	return s;
}

void Game::start_py(){
	/*
	Run python code that:
	-Defines all user created classes, including creating the constructor
	-Imports all user files
	-Defines any necessary Python functions
	-Runs necessary user code
	*/

	using namespace boost::python;
	AcquireGIL gil = AcquireGIL();

	std::cout << "acquired gil for start py" << std::endl;

	//in case user left game name blank
	if (game_name_ == ""){
		game_name_ = "sfpy-gmk game";
	}
	try{
		//import global Python namespace
		object main_module = import("__main__");
		object global = main_module.attr("__dict__");

		object sfgame_module = import("sfgame");

		
		std::string string1 = 
			"from threading import Thread		\n"
			"from sfgame import *				\n";

		//define all classes
		for (auto& it : obj_props_){
			string1 += generatePyClassString(it);
		}

		object result = exec(string1.c_str(), global, global);

		//store all classes just created in the sfgame module
		for (auto& it : obj_props_){
			object o = global[it["$name"]];
			sfgame_module.attr(it.at("$name").c_str()) = o;
		}

		//import user files relating to classes
		std::string string2 = "";
		for (auto& it : obj_props_){
			std::string name = it["$name"];
			string2 += ("import " + name + "\n");
		}

		//Define functions that generate Python threads, as this is difficult to do from C++
		//Passes threads back into C++ so that the C++ controls their lifetimes
		string2 +=
			"def add(entity):							\n"
			"	t = Thread(target=entity.start)			\n"
			"	game._addToC(entity)					\n"
			"	t.start()								\n"
			"	game._addThread(t)						\n"
			"game.add = add								\n"
			"def _runEvent(func,evt_args):				\n"
			"	t = Thread(target=func,args=evt_args)	\n"
			"	t.start()								\n"
			"	game._addThread(t)						\n"
			"game._runEvent = _runEvent					\n";




		object result2 = exec(string2.c_str(), global, global);

		object run_script = exec_file("testglob.py", global, global);

		//Block this thread until all the threads created have terminated.
		//Necessary because this thread is holding the GIL.
		while (!threads_.empty()){
			//Iterate on a copy of threads to prevent iterator invalidating due to new thread being added
			std::vector<boost::python::object> current_it = threads_; 
			threads_.clear();
			for (auto& it : current_it){
				it.attr("join")();
			}
		}

		//TODO: maybe don't need this? thread can end safely? hm?

	}
	catch (const boost::python::error_already_set&){
		PyObject* ptype, *pvalue, *ptraceback;
		PyErr_Fetch(&ptype, &pvalue, &ptraceback);
		std::string error;
		//if (pvalue != NULL){
		error = boost::python::extract<std::string>(pvalue);
		std::cout << error << std::endl;
		//}
	}
}

void Game::fireGlobalEvent(GAME_EVENT evt, boost::python::tuple args){
	AcquireGIL gil = AcquireGIL();
	//Fire event in all entities
	for (auto& it : objects_){
		it.second->fireEvent(evt, args);
	}
}


void Game::addThread(boost::python::object thread){
	//Add thread to list so that its lifetime is controlled
	//Called as _addThread from Python
	threads_.push_back(thread);
}

void Game::add(boost::python::object entity){
	/*
	Add C++ pointer of entity to container for updating, and python references 
	to entity so that C++ controls its lifetime.
	Called as "game._addToC" from Python-defined game.add
	*/
	
	Entity* ent = boost::python::extract<Entity*>(entity);
	objects_.push_back(std::make_pair(entity, std::unique_ptr<Entity>(ent)));
}

void Game::update(int frametime){
	for (auto& it : objects_){
		it.second->update(frametime);
	}
}

void Game::render(sf::RenderTarget* target){
	for (auto& it : objects_){
		it.second->render(target);
	}
}