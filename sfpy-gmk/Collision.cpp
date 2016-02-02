#include "Collision.h"
#include "Projection.h"

MTV MTV::NONE = MTV(0,maths::Vector2(0,0));

Collision::~Collision(){
}

float Collision::minValue(float a,float b,float c,float d){
	float min = a;
	min = (b < min ? b : min);
	min = (c < min ? c : min);
	min = (d < min ? d : min);
	return min;
}

float Collision::maxValue(float a,float b,float c,float d){
	float max = a;
	max = (b > max ? b : max);
	max = (c > max ? c : max);
	max = (c > max ? c : max);
	return max;
}


MTV Collision::getCollision(const sf::Sprite& object1,Entity::ENTITY_SHAPE shape1,const sf::Sprite& object2,Entity::ENTITY_SHAPE shape2){
	//Use Separating Axis Theorem to determine whether two objects are overlapping
	//See documentation for full explanation of this algorithm

	//Get the oriented bounding box in world coordinates (includes rotation) of objects
	OBB obb1 = getOBB(object1);
	OBB obb2 = getOBB(object2);
	float rot = object1.getRotation();

	double overlap = LONG_MAX;
	maths::Vector2 smallest(0,0);

	std::vector<maths::Vector2> axis1;
	std::vector<maths::Vector2> axis2;

	maths::Vector2 circleCentre1;
	maths::Vector2 circleCentre2;

	sf::FloatRect gbounds1 = object1.getGlobalBounds();
	sf::FloatRect gbounds2 = object2.getGlobalBounds();
	 
	//Find all the axes to check using SAT based on shapes of objects
	//Works with squares/rectangles or circles
	//Rectangles only need 2 axes because they have 2 sets of parallel lines
	if(shape1 == Entity::SHAPE_CIRCLE && shape2 == Entity::SHAPE_CIRCLE){
		//If both shapes are circles, the only axis needed is the axis between centres of circles
		circleCentre1 = maths::Vector2(gbounds1.left + gbounds1.width / 2, gbounds1.top + gbounds1.height / 2);
		circleCentre2 = maths::Vector2(gbounds2.left + gbounds2.width / 2, gbounds2.top + gbounds2.height / 2);
		axis1.push_back(maths::Vector2(circleCentre1 - circleCentre2).normalise());

	}else if(shape1 != shape2){ //if one shape is circle and one shape is rectangle
		maths::Vector2 circleCentre;
		sf::FloatRect squareRect;
		float rotation;

		//First get the unrotated bounding box and centre of the circle of the 2 objects
		if(shape1 == Entity::SHAPE_CIRCLE){
			circleCentre = maths::Vector2(gbounds1.left + gbounds1.width / 2, gbounds1.top + gbounds1.height / 2);
			circleCentre1 = circleCentre;
			squareRect = getOriginalBoundingBox(object2);
			rotation = object2.getRotation();
		}else if(shape2 == Entity::SHAPE_CIRCLE){
			circleCentre = maths::Vector2(gbounds2.left + gbounds2.width / 2, gbounds2.top + gbounds2.height / 2);
			circleCentre2 = circleCentre;
			squareRect = getOriginalBoundingBox(object1);
			rotation = object1.getRotation();
		}

		maths::Vector2 squareCentre(squareRect.left + squareRect.width / 2, squareRect.top + squareRect.height / 2);
		OBB* square = (shape1 == Entity::SHAPE_SQUARE ? &obb1 : &obb2);
		maths::Vector2 relativeCircleCentre = circleCentre.rotate(-rotation, squareCentre); //get circle centre in relation to the rotated square
		bool vertice = false;
		maths::Vector2 axis;

		maths::Vector2 topLeft(squareRect.left, squareRect.top);
		maths::Vector2 topRight(squareRect.left + squareRect.width, squareRect.top);
		maths::Vector2 botLeft(squareRect.left, squareRect.top + squareRect.height);
		maths::Vector2 botRight(squareRect.left + squareRect.width, squareRect.top + squareRect.height);

		//Get the closest vertex of the rectangle to the circle centre.
		//The axis to check is the vector between these 2 points.
		if(circleCentre.x < topLeft.x){
			if(circleCentre.y < topLeft.y){
				vertice = true;
				axis = topLeft;
			}else if(circleCentre.y >  botLeft.y){
				vertice = true;
				axis = botLeft;
			}else{
				axis = maths::Vector2(topLeft - botLeft).normalise();
			}
		}else if(circleCentre.x > topRight.x){
			if(circleCentre.y < topLeft.y){
				vertice = true;
				axis = topRight;
			}else if(circleCentre.y >  botLeft.y){
				vertice = true;
				axis = botRight;
			}else{
				axis = maths::Vector2(topRight - botRight).normalise();
			}
		}else{
			if(circleCentre.y < topLeft.y){
				axis = maths::Vector2(topRight - topLeft).normalise();
			}else if(circleCentre.y >  botLeft.y){
				axis = maths::Vector2(botLeft - botRight).normalise();
			}else{
				//contains point!
			}
		}

		if(vertice){
			axis1.push_back(maths::Vector2(circleCentre - axis).normalise());
		}else{
			axis1.push_back(maths::Vector2(topRight - topLeft).normalise());
			axis1.push_back(maths::Vector2(topRight - botRight).normalise());
		}
	}else{ //If both shapes are rectangles
		//Get vectors for sides of shapes
		maths::Vector2 Xside1(obb1.bot_left - obb1.bot_right);
		maths::Vector2 Yside1(obb1.top_left - obb1.bot_left);
		maths::Vector2 Xside2(obb2.bot_left - obb2.bot_right);
		maths::Vector2 Yside2(obb2.top_left - obb2.bot_left);

		//Axes requires are perpendicular to the sides of the shape
		//Vector2.perpendicular() normalises for greater accuracy
		axis1.push_back(Xside1.perpendicular());
		axis1.push_back(Yside1.perpendicular());
		axis2.push_back(Xside2.perpendicular());
		axis2.push_back(Yside2.perpendicular());
	}

	//We have all the axes to check.
	//Now find details on collisions with projections.

	for(int i=0;i<axis1.size();i++){
		//Get projection of axis for both shapes
		maths::Vector2 axis = axis1[i];
		Projection projection1 = project(obb1, axis);
		if(shape1 == Entity::SHAPE_CIRCLE){
			float radius = gbounds1.width / 2;
			projection1 = projectCircle(circleCentre1, radius, axis);
		}
		Projection projection2 = project(obb2, axis);
		if (shape2 == Entity::SHAPE_CIRCLE){
			float radius = gbounds2.width / 2;
			projection2 = projectCircle(circleCentre2, radius, axis);
		}

		//If a projection does not overlap, we know the objects do not collide so we can exit the function.
		//Otherwise, the MTV (minimum translation vector required to make the objects not collide) is calculated.

		if(!projection1.overlap(projection2)){
			return MTV::NONE;
		}else{
			//The axis with the smallest overlap is the axis used to calculate MTV, so record it.
			double o = projection1.getOverlap(projection2);
			if(o < overlap){
				overlap = o; //set smallest overlap
				smallest = axis; //set smallest separation vector
			}
		}
	}
	
	//Repeat the same process as above with the other set of axes.
	for(int i=0;i<axis2.size();i++){
		maths::Vector2 axis = axis2[i];
		Projection projection1 = project(obb1, axis);
		if(shape1 == Entity::SHAPE_CIRCLE){
			float radius = gbounds1.width/2;
			projection1 = projectCircle(circleCentre1, radius, axis);
		}
		Projection projection2 = project(obb2,axis);
		if(shape2 == Entity::SHAPE_CIRCLE){
			float radius = gbounds2.width / 2;
			projection2 = projectCircle(circleCentre2, radius, axis);
		}

		if(!projection1.overlap(projection2)){
			return MTV::NONE;
		}else{
			double o = projection1.getOverlap(projection2);
			if(o < overlap){
				overlap = o;
				smallest = axis;
			}
		}
	}
	//Get the vector from the centre of object 2 to the centre of object 1
	maths::Vector2 centre1 = maths::Vector2(gbounds1.left + gbounds1.width / 2, gbounds1.top + gbounds1.height / 2);
	maths::Vector2 centre2 = maths::Vector2(gbounds2.left + gbounds2.width / 2, gbounds2.top + gbounds2.height / 2);
	maths::Vector2 between = centre1 - centre2;
	//If the separation vector is in the opposite direction of 'between', flip it round by negating it
	if(between.dot(smallest) < 0){
		smallest = -smallest;
	}
	MTV mtv(overlap, smallest);
	return mtv;
}

Projection Collision::project(const OBB& box,const maths::Vector2& axis){
	//Gets projection of the oriented bounding box (rectangle, not circle) on the given axis
	//See documentation for explanation
	std::vector<maths::Vector2> vertices;

	//Get all vertices of shape
	vertices.push_back(box.top_left); vertices.push_back(box.top_right);
	vertices.push_back(box.bot_left); vertices.push_back(box.bot_right);

	double min = vertices[0].dot(axis);
	double max = min;
	for(int i=1; i < vertices.size(); i++){
		double p = vertices[i].dot(axis);
		if(p < min){
			min = p;
		}else if(p > max){
			max = p;
		}
	}
	return Projection(min,max);
}

Projection Collision::projectCircle(const maths::Vector2& centre,float radius,const maths::Vector2& axis){
	//Get projection of circle onto given axis
	double p = centre.dot(axis);
	double min = p - radius;
	double max = p + radius;

	return Projection(min,max);
}

Collision::OBB Collision::getOBB(const sf::Sprite& object){
	//Gets the oriented bounding box of object, which is the local bounds of the object rotated
	sf::FloatRect box(object.getLocalBounds());
	maths::Vector2 topleft, botright, topright, botleft;

	float rot = object.getRotation();
	maths::Vector2 origin = object.getOrigin();
	maths::Vector2 position = object.getPosition();
	
	//Get rotated coordinates of vertices
	topleft = position + maths::Vector2(box.left, box.top).rotate(rot, origin);
	botright = position + maths::Vector2(box.left + box.width, box.top + box.height).rotate(rot, origin);
	topright = position + maths::Vector2(box.left + box.width, box.top).rotate(rot, origin);
	botleft = position + maths::Vector2(box.left, box.top + box.height).rotate(rot, origin);


	return OBB(topleft, botleft, topright, botright);
}

sf::FloatRect Collision::getOriginalBoundingBox(const sf::Sprite& object){
	//Get unrotated bounding box in world coordinates
	sf::FloatRect rect = object.getLocalBounds();
	maths::Vector2 tl = object.getPosition() - object.getOrigin() + maths::Vector2(rect.left, rect.top);
	return sf::FloatRect(tl.x, tl.y, rect.width, rect.height);
}