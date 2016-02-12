#include <SFML/Graphics.hpp>
#include "Vector2.h"
#include <boost/python.hpp>

class Game;

/*
Text:
Stores properties of a Python Text object and provides functions for Python to
manipulate it.
Requires rendering every frame.
*/

class Text{
public:
	Text();

	void setParent(Game* parent);
	void render(sf::RenderTarget* target);

	maths::Vector2 getPosition() const;
	boost::python::object getPyPosition() const;

	void setPyPosition(boost::python::object position);
	void setPosition(maths::Vector2 position);

	void setText(std::string text);
	std::string getText() const;

	void setColour(int r, int g, int b);

	int getFontSize() const;
	void setFontSize(int size);
private:
	sf::Text text_;
	boost::python::object position_;
	Game* parent_;
};