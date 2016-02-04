#include "Text.h"
#include "Game.h"

Text::Text(){
}

void Text::setParent(Game* parent){
	parent_ = parent;
	text_.setFont(*parent->getFont());
}

void Text::setText(std::string text){
	text_.setString(text);
}

std::string Text::getText() const{
	return text_.getString();
}

maths::Vector2 Text::getPosition() const{
	return boost::python::extract<maths::Vector2>(position_);
}

boost::python::object Text::getPyPosition() const{
	return position_;
}

void Text::setColour(int r, int g, int b){
	text_.setColor(sf::Color(r, g, b));
}

void Text::setFontSize(int size){
	text_.setCharacterSize(size);
}

int Text::getFontSize() const{
	return text_.getCharacterSize();
}

void Text::setPyPosition(boost::python::object position){
	position_ = position;
	text_.setPosition(getPosition());
}

void Text::setPosition(maths::Vector2 position){
	setPyPosition(boost::python::object(position));
}

void Text::render(sf::RenderTarget* target){
	target->draw(text_);
}

