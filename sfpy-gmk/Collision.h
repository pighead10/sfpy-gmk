/*
Collision:
Static class containing functions relating o SAT collision algorithms.
*/

#pragma once

#include "Entity.h"
#include "Vector2.h"

struct MTV{
	MTV(double overl,maths::Vector2 axe):overlap(overl),axis(axe){}
	double overlap;
	maths::Vector2 axis;
	static MTV NONE;
};

class Projection;

class Collision{
public:
	~Collision();
	static float minValue(float a, float b, float c, float d);
	static float maxValue(float a, float b, float c, float d);
	static sf::FloatRect getOriginalBoundingBox(const sf::Sprite& sprite);

	struct OBB{
		OBB(const maths::Vector2& tl,const maths::Vector2& bl,const maths::Vector2& tr,
			const maths::Vector2& br):top_left(tl),bot_left(bl),top_right(tr),bot_right(br){
		}
		maths::Vector2 top_left;
		maths::Vector2 bot_left;
		maths::Vector2 top_right;
		maths::Vector2 bot_right;
	};
	
	static MTV getCollision(const sf::Sprite& object1, Entity::ENTITY_SHAPE shape1, const sf::Sprite& object2, Entity::ENTITY_SHAPE shape2);
	static OBB getOBB(const sf::Sprite& object);
	static Projection project(const OBB& box, const maths::Vector2& vec);
	static Projection projectCircle(const maths::Vector2& centre, float radius, const maths::Vector2& axis);
private:
	Collision(){}

};