#include "Game.h"
#include "Entity.h"
#include <boost/python.hpp>
#include <memory>
#include <sstream>
#include <fstream>
#include <string>
#include "gil.h"
#include "Script.h"
#include "Text.h"

#define MS_PER_UPDATE 1

Game::Game():game_name_(""),init_(false){
	background_set_ = false;
	background_colour_ = sf::Color::White;
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
		if (init_){
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
					if (evt.key.code == sf::Keyboard::L){ //TEMP
						//scheduleLevel("testgame.level");
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
					fireGlobalEvent(game_evt, boost::python::make_tuple(but, maths::Vector2(evt.mouseButton.x, evt.mouseButton.y)));
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
			window.clear(background_colour_);
			render(&window);
			window.display();
		}
	}

	window.close();
}

void Game::setBackgroundColour(int r, int g, int b){
	background_colour_ = sf::Color(r, g, b);
}

sf::Font* Game::getFont(){
	return &font_;
}

void Game::load(std::string filename){
	load_file_ = filename;
	std::string gamefile = load_file_ + ".game";
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
				else if (type == TYPE_SCRIPT){
					script_props_.back()[prop] = line;
					prefix = PREFIX_NONE;
				}
				else if (type == TYPE_SOUND){
					sound_props_.back()[prop] = line;
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
							else if (word == "sound"){
								type = TYPE_SOUND;
								sound_props_.push_back(StringMap());
							}
							else if (word == "script"){
								type = TYPE_SCRIPT;
								script_props_.push_back(StringMap());
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
	load_sounds();
	load_scripts();

}

void Game::load_scripts(){
	for (auto& it : script_props_){
		scripts_.push_back(std::unique_ptr<Script>(new Script(it["name"],it["filename"])));
	}
}

ObjectList* Game::getObjects(){
	return &objects_;
}

void Game::load_textures(){
	for (auto& it : tex_props_){
		textures_.load(it["name"], it["filename"]);
	}
}

void Game::load_sounds(){
	for (auto& it : sound_props_){
		sounds_.load(it["name"], it["filename"]);
	}
}

ResourceManager<sf::Texture, std::string>* Game::getTextures(){
	return &textures_;
}

void Game::scheduleLevel(std::string filename){
	scheduled_levels_.push_back(filename);
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

	s +=
		"	def start(self):			\n"
		"		pass					\n";

	return s;
}

void Game::playSound(std::string soundname){
	std::cout << "PLAY SOUND!!" << std::endl;
	sf::Sound* sound = new sf::Sound();
	sound->setBuffer(sounds_.get(soundname));
	sound_list_.push_back(sound);
	sound->play();
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
		font_.loadFromFile("font.ttf");
		//import global Python namespace
		main_module = import("__main__");
		global = main_module.attr("__dict__");

		object sfgame_module = import("sfgame");

		init_ = true;
	
		std::string string1 = 
			"from threading import Thread, Lock		\n"
			"from sfgame import *					\n";

		for (auto& it : scripts_){ //TODO add enabled property
			string1 += "import " + it->getImportName() + "\n";
		}
		
		//define all classes
		for (auto& it : obj_props_){
			string1 += generatePyClassString(it);
		}
		std::cout << string1 << std::endl;

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
			string2 +=
				"try:					\n"
				"	import " + name + "\n" //in case object was not assigned a script
				"except:				\n"
				"	print('CANNOT IMPORT  " + name + "')				\n";
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

		std::cout << string2 << std::endl;

		object result2 = exec(string2.c_str(), global, global);
		std::string string3 = "";
		std::cout << "creating string 3..." << std::endl;
		if (scripts_.size() > 0){
			string3 =
				"def _start():					\n";
			for (auto& it : scripts_){
				string3 +=
					"	t = Thread(target=" + it->getImportName() + ".run)		\n"
					"	t.start()												\n"
					"	game._addThread(t)										\n";
			}
			string3 += "_start()\n";
		}
		std::cout << "loading level..." << std::endl;
		loadLevel(load_file_ + ".level");

		std::cout << "STRING3:" << std::endl;
		std::cout << string3 << std::endl;

		std::cout << "before run scripts" << std::endl;
		object run_scripts = exec(string3.c_str(), global, global);
		std::cout << "after run scripts" << std::endl;

		//object run_script = exec_file("testglob.py", global, global);

		//Block this thread until all the threads created have terminated.
		//Necessary because this thread is holding the GIL.
		/*while (!threads_.empty()){
			//Iterate on a copy of threads to prevent iterator invalidating due to new thread being added
			std::vector<boost::python::object> current_it = threads_;
			threads_.clear();
			for (auto& it : current_it){
				it.attr("join")();
			}
		}*/

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

void Game::setBackgroundTexture(std::string texname){
	background_spr_.setTexture(textures_.get(texname));
	background_set_ = true;
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
	object_queue_.push_back(std::make_pair(entity, std::unique_ptr<Entity>(ent)));
}

void Game::addText(boost::python::object text){
	Text* t = boost::python::extract<Text*>(text);
	t->setParent(this);
	text_queue_.push_back(std::make_pair(text, std::unique_ptr<Text>(t)));
}

void Game::update(int frametime){
	//load levels that are scheduled
	for (auto& it = scheduled_levels_.begin(); it != scheduled_levels_.end();){
		loadLevel(*it);
		it = scheduled_levels_.erase(it);
	}

	//add objects in queue to list to be updated
	//mutex_.lock();
	//boost::python::exec("_mutex = Lock()\n_mutex.acquire()\n", global, global);

	int maxn = object_queue_.size();
	for (int n = 0; n < maxn;n++){ //This errors when using iterators due to multithreaded program adding object into queue while iterating.
									//Should fix by locking mutex but I can't figure out how. So just abandon iterators for now.
		auto element = &object_queue_[0];
		objects_.push_back(std::make_pair(element->first, std::move(element->second)));
		object_queue_.erase(object_queue_.begin());
		fireGlobalEvent(ObjectAdded, boost::python::make_tuple(objects_.back().first));
	}

	maxn = text_queue_.size();
	for (int n = 0; n < maxn; n++){ //This errors when using iterators due to multithreaded program adding object into queue while iterating.
		//Should fix by locking mutex but I can't figure out how. So just abandon iterators for now.
		auto element = &text_queue_[0];
		texts_.push_back(std::make_pair(element->first, std::move(element->second)));
		text_queue_.erase(text_queue_.begin());
	}
	//boost::python::exec("_mutex.release()\n", global, global);
	//mutex_.unlock();

	//remove objects that have been destroyed
	for (auto& it = objects_.begin(); it != objects_.end();){
		if (it->second->getDestroyed()){
			fireGlobalEvent(ObjectRemoved, boost::python::make_tuple(it->first));
			it->second.release();
			it = objects_.erase(it);
		}
		else{
			it++;
		}
	}

	for (auto& it : objects_){
		it.second->update(frametime);
	}
}

boost::python::object Game::getObjectByName(std::string name){
	//returns first object found with given name (not much use if more than 1 object exists...)
	//or None if no object exists
	for (auto& it : objects_){
		if (it.second->getName() == name){
			return it.first;
		}
	}
	return boost::python::object();
}

void Game::render(sf::RenderTarget* target){
	if (background_set_){
		target->draw(background_spr_);
	}
	for (auto& it : objects_){
		if (it.second->getVisible() && it.second->getRenderBehind()){ //render objects at back first so they are behind other objects
			it.second->render(target);
		}
	}
	for (auto& it : objects_){
		if (it.second->getVisible() && !it.second->getRenderBehind()){
			it.second->render(target);
		}
	}
	for (auto& it : texts_){
		it.second->render(target);
	}
}

void Game::loadLevel(std::string filename){
	using namespace std;
	ifstream fin(filename);
	string line;
	CFG_TYPE type = TYPE_NONE;
	CFG_PREFIX prefix = PREFIX_NONE;

	for (auto& it = objects_.begin(); it != objects_.end();){
		it->second.release(); //release ownership of pointer 
		it = objects_.erase(it);
	}
	object_queue_.clear();
	
	string python_execute = "from sfgame import *\n";
	while (getline(fin, line)){
		if (type == TYPE_OBJ){
			//if the previous line was an object then add it
			python_execute += "game.add(_auto_loaded_object)\n";
		}

		type = TYPE_NONE;
		prefix = PREFIX_NONE;
		istringstream s(line);
		string word;
		
		for (int i = 0; s >> word; i++){
			if (type == TYPE_NONE){
				if (word == "obj"){
					type = TYPE_OBJ;
					//current_entity = new Entity(this);
					//addEntity(current_entity);
					prefix = PREFIX_NEXT;
				}
			}
			else if (type == TYPE_OBJ){
				if (prefix == PREFIX_NONE){
					prefix = PREFIX_NEXT;
				}
				else if (prefix == PREFIX_NEXT){
					if (word == "name"){
						prefix = PREFIX_NAME;
					}
					else if (word == "pos"){
						prefix = PREFIX_POSX;
					}
				}
				else if (prefix == PREFIX_NAME){
					python_execute += "_auto_loaded_object = " + word + "()\n";
					prefix = PREFIX_NEXT;
				}
				else if (prefix == PREFIX_POSX){
					python_execute += "_auto_loaded_object.position = Vector2(" + word + ",_auto_loaded_object.position.y)\n";
					prefix = PREFIX_POSY;
				}
				else if (prefix == PREFIX_POSY){
					python_execute += "_auto_loaded_object.position = Vector2(_auto_loaded_object.position.x, " + word + ")\n";
					prefix = PREFIX_NEXT;
				}
			}
		}
	}
	//if last line is adding an object
	if (type == TYPE_OBJ){
		//if the previous line was an object then add it
		python_execute += "game.add(_auto_loaded_object)\n";
	}

	std::cout << python_execute << std::endl;
	boost::python::object main_module = boost::python::import("__main__");
	boost::python::object global = main_module.attr("__dict__");
	boost::python::exec(boost::python::str(python_execute), global, global);
}