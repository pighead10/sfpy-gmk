#pragma once

#include "SFML/Graphics.hpp"
#include "Vector2.h"
#include <boost/python.hpp>
#include "Game.h"

class Entity{
public:
	enum ENTITY_SHAPE{
		SHAPE_CIRCLE,
		SHAPE_SQUARE,
	};

	Entity();
	void init_entity(Game* game);

	void update(int frametime);
	void render(sf::RenderTarget* target);

	maths::Vector2 getPosition() const;
	boost::python::object getPyPosition() const;

	void setPyPosition(boost::python::object position);
	void setPosition(maths::Vector2 position);

	void setPyVelocity(boost::python::object velocity);
	void setVelocity(maths::Vector2 velocity);

	maths::Vector2 getVelocity() const;
	boost::python::object getPyVelocity() const;

	bool getVisible() const;
	void setVisible(bool visible);

	bool getCollidable() const;
	void setCollidable(bool collidable);

	std::string getTexture() const; //gets name of texture
	void setTexture(std::string texture_name);

	ENTITY_SHAPE getShape() const;
	sf::Sprite getSprite() const;

	void registerEvent(Game::GAME_EVENT evt, boost::python::object func);
	void fireEvent(Game::GAME_EVENT evt,boost::python::tuple args);
private:
	void move(maths::Vector2 offset);

	Game* parent_;

	ENTITY_SHAPE shape_;
	sf::Sprite sprite_;
	boost::python::object position_; //Store position as python object so that position can be referenced properly from Python code
	boost::python::object velocity_;
	bool visible_;
	bool collidable_;
	std::string texture_name_;

	std::map<Game::GAME_EVENT, boost::python::object> events_;
};