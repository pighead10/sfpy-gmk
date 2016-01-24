#pragma once

#include <SFML/System/Vector2.hpp>

namespace maths{ 
	class Vector2 : public sf::Vector2f{
	public:
		Vector2();
		Vector2(sf::Vector2f vec); //conversion constructor
		Vector2(double x, double y);
		double getX() const;
		double getY() const;
		void setX(double x);
		void setY(double y);
		Vector2 rotate(float degrees, const Vector2& origin) const;
		float dot(const Vector2& other) const;
		Vector2 normalise() const;
		float length() const;
		Vector2 perpendicular() const;
		Vector2 lerp(const Vector2& start, const Vector2& end, float percent) const;
		Vector2 negate() const;
	};

	float toRadians(float degrees);
	float toDegrees(float radians);
}