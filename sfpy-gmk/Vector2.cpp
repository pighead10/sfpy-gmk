#include "Vector2.h"
#include <cmath>

#define PI 3.14159f

namespace maths{
	Vector2::Vector2(){}
	Vector2::Vector2(sf::Vector2f vec){
		x = vec.x;
		y = vec.y;
	}
	Vector2::Vector2(double x_, double y_){
		x = x_;
		y = y_;
	}
	double Vector2::getX() const{
		return x;
	}
	double Vector2::getY() const{
		return y;
	}
	void Vector2::setX(double x_){
		x = x_;
	}
	void Vector2::setY(double y_){
		y = y_;
	}
	Vector2 Vector2::rotate(float degrees, const Vector2& origin) const{
		float radians = maths::toRadians(degrees);
		Vector2 difference = *this - origin;
		Vector2 rotated;
		rotated.x = difference.x * cos(radians) + difference.y * sin(radians);
		rotated.y = -difference.x * sin(radians) + difference.y * cos(radians);
		return rotated;
	}
	float Vector2::dot(const Vector2& other) const{
		return (x * other.x) + (y * other.y);
	}
	Vector2 Vector2::normalise() const{
		float len = length();
		if (len == 0) return Vector2(0, 0);
		return Vector2(x / len, y / len);
	}
	float Vector2::length() const{
		return sqrt(x*x + y*y);
	}
	Vector2 Vector2::perpendicular() const{
		return Vector2(-y, x);
	}
	Vector2 Vector2::lerp(const Vector2& start, const Vector2& end, float percent) const{
		return start + percent*(end - start);
	}
	Vector2 Vector2::negate() const{
		return Vector2(-x, -y);
	}

	float toRadians(float degrees){
		return degrees * (PI / 180);
	}

	float toDegrees(float radians){
		return radians * (180 / PI);
	}
}