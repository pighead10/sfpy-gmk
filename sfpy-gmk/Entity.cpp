#include "Entity.h"
#include "Game.h"
#include "Collision.h"

Entity::Entity(){
}

void Entity::init_entity(Game* game){
	//C++ initialisation of entity
	//Called from python as _init_entity before Python constructor
	parent_ = game;
	sprite_ = sf::Sprite();
	shape_ = SHAPE_CIRCLE;
	render_behind_ = false;
	destroyed_ = false;
	outbounds_ = false;
}

Entity::ENTITY_SHAPE Entity::getShape() const{
	return shape_;
}

bool Entity::getOutBounds() const{
	return outbounds_;
}

void Entity::setOutBounds(bool value){
	outbounds_ = value;
}

sf::Sprite Entity::getSprite() const{
	return sprite_;
}

void Entity::destroy(){
	destroyed_ = true;
}

bool Entity::getDestroyed() const{
	return destroyed_;
}

void Entity::setShape(std::string shape){
	if (shape == "circle"){
		shape_ = SHAPE_CIRCLE;
	}
	else if (shape == "rectangle"){
		shape_ = SHAPE_SQUARE;
	}
}

void Entity::setRenderBehind(bool value){
	render_behind_ = value;
}

bool Entity::getRenderBehind() const{
	return render_behind_;
}

void Entity::move(maths::Vector2 offset){
	//Move entity, resolving collisions

	maths::Vector2 direction = offset.normalise();
	float magnitude = offset.length();

	ObjectList* list = parent_->getObjects();
	for (auto& it : *list){
		Entity* entity = it.second.get();

		if (entity != this){
			float dist = maths::Vector2(entity->getPosition() - getPosition()).length();
			//If distance to entity is below [some number], it may be collided with this, so proceed to accurate SAT collision detection
			//Otherwise, entity is definitely not collided with this, so the expensive SAT function does not need to be run

			int maxl1 = std::max(getSprite().getLocalBounds().width, getSprite().getLocalBounds().height);
			int maxl2 = std::max(entity->getSprite().getLocalBounds().width, entity->getSprite().getLocalBounds().height);
			if (dist <= maxl1+maxl2){
				MTV mtv(Collision::getCollision(getSprite(), getShape(), entity->getSprite(), entity->getShape()));

				if (!(mtv.axis == MTV::NONE.axis && mtv.overlap == MTV::NONE.overlap)){ //If a collision has occurred
					if (entity->getCollidable()){
						//If the entity is collidable, it cannot be walked through, so change movement direction

						maths::Vector2 n = mtv.axis;
						maths::Vector2 comp_u(0, 0);

						if (direction.dot(n) < 0){ //TODO check if this line is necessary?
							if (n != maths::Vector2(0, 0)){
								comp_u = n * (direction.dot(n) / n.dot(n)); //Get the component of direction that is in th same direction as the MTV axis
							}
						}
						direction = direction - comp_u; //Subtract this component from direction, so the component of direction towards the object that it collided with is 0
						//This means the object will not move at all towards the object it collided with, but will move normally in other directions
					}
					//Resolve all effects of a collision excluding movement
					fireEvent(Game::Collision, boost::python::make_tuple(it.first));
				}
			}
		}
	}
	
	setPosition(getPosition() + direction*magnitude);
}

void Entity::registerEvent(Game::GAME_EVENT evt, boost::python::object func){
	//Registers Python function for event
	//This function will then be called whenever event is fired
	events_[evt] = func;
}

void Entity::fireEvent(Game::GAME_EVENT evt, boost::python::tuple args){
	using namespace boost::python;
	//Runs function that is registered for event with args in separate thread

	//Check if event is registered
	if (events_.find(evt) != events_.end()){
		//Run event with contents of 'arg'
		//Call function defined in Python to create and run thread for f
		boost::python::object f = events_[evt];
		object global = import("__main__");
		global.attr("game").attr("_runEvent")(f, args);
	}
}

void Entity::update(int frametime){
	move(maths::Vector2(getVelocity().x*frametime, getVelocity().y*frametime));

	maths::Vector2 position = getPosition();
	sprite_.setPosition(position.getX(), position.getY());
}

std::string Entity::getName() const{
	return name_;
}

void Entity::setName(std::string name){
	name_ = name;
}

std::string Entity::getTexture() const{
	return texture_name_;
}

void Entity::setTexture(std::string texture){
	texture_name_ = texture;
	sprite_.setTexture(parent_->getTextures()->get(texture_name_));
}

void Entity::setPyVelocity(boost::python::object velocity){
	velocity_ = velocity;
}

void Entity::setVelocity(maths::Vector2 velocity){
	setPyVelocity(boost::python::object(velocity));
}

maths::Vector2 Entity::getVelocity() const{
	return boost::python::extract<maths::Vector2>(velocity_);
}

boost::python::object Entity::getPyVelocity() const{
	return velocity_;
}

maths::Vector2 Entity::getPosition() const{
	return boost::python::extract<maths::Vector2>(position_);
}

boost::python::object Entity::getPyPosition() const{
	return position_;
}

void Entity::setPyPosition(boost::python::object position){
	position_ = position;
}

void Entity::setPosition(maths::Vector2 position){
	maths::Vector2 old_pos = getPosition();
	setPyPosition(boost::python::object(position));
	if (position.x > 1024 || position.x < 0 || position.y > 768 || position.y < 0){
		if (!outbounds_){
			fireEvent(Game::OutOfBounds, boost::python::make_tuple(old_pos));
		}
		outbounds_ = true;
	}
	else{
		outbounds_ = false;
	}
}

void Entity::render(sf::RenderTarget* target){
	target->draw(sprite_);
}

bool Entity::getVisible() const{
	return visible_;
}

void Entity::setVisible(bool visible){
	visible_ = visible;
}

bool Entity::getCollidable() const{
	return collidable_;
}

void Entity::setCollidable(bool collidable){
	collidable_ = collidable;
}
