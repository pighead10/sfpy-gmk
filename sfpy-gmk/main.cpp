#include <iostream>
#include <boost/python.hpp>
#include <SFML/Graphics.hpp>
#include <boost/thread.hpp>
#include "Game.h"
#include "Entity.h"
#include "gil.h"
#include "Vector2.h"

using namespace boost::python;

//Create module initialision function for entire sfgame module

Game* game = new Game();

BOOST_PYTHON_MODULE(sfgame){
	//Enable threads
	PyEval_InitThreads();

	//Define classes
	class_<maths::Vector2>("Vector2", init<double, double>()) //TODO: operator loading for exposed python vector2 class
															//TODO: from python "vector.x = 10" not working, fix it
		.add_property("x", &maths::Vector2::getX, &maths::Vector2::setX)
		.add_property("y", &maths::Vector2::getY, &maths::Vector2::setY)
		.def("rotate",&maths::Vector2::rotate)
		.def("dot", &maths::Vector2::dot)
		.def("normalise", &maths::Vector2::normalise)
		.def("length", &maths::Vector2::length)
		.def("perpendicular", &maths::Vector2::perpendicular)
		.def("lerp", &maths::Vector2::lerp)
		.def("negate", &maths::Vector2::negate)
		;
	class_<Game,boost::noncopyable>("Game", no_init)
		.def("_addToC", &Game::add)
		.def("_addThread",&Game::addThread)
		;
	class_<Entity>("Entity", init<>())
		.def("_init_entity",&Entity::init_entity)
		.add_property("position", &Entity::getPyPosition, &Entity::setPyPosition)
		.add_property("visible",&Entity::getVisible,&Entity::setVisible)
		.add_property("collidable",&Entity::getCollidable,&Entity::setCollidable)
		.add_property("texture",&Entity::getTexture,&Entity::setTexture)
		.add_property("velocity",&Entity::getPyVelocity,&Entity::setPyVelocity)
		.def("registerEvent",&Entity::registerEvent)
		;
	enum_<Game::GAME_EVENT>("GameEvent")
		.value("Collision", Game::Collision)
		;
	scope().attr("game") = ptr(game);
}

int main(){
	//Register the sfgame module
	PyImport_AppendInittab("sfgame", &initsfgame);

	//initialise python interpreter
	Py_Initialize();

	//Import and intitialise the sfgame module
	import("sfgame");

	game->load("testgame.game");

	//Release GIL so that gameloop_thread can acquire it
	ReleaseGIL gil = ReleaseGIL();

	boost::thread gameloop_thread(boost::bind(&Game::start_gameloop, game));
	game->start_py();

	gameloop_thread.join();

	return 0;
}