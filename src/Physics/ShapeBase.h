#pragma once
#include "Math/Vector.h"
namespace Cacti
{
	class Shape
	{
	public:
		enum ShapeType
		{
			SPHERE,
		};
		Shape() = default;
		~Shape() = default;

		virtual ShapeType GetType() const = 0;

		virtual Vec3 GetCenterOfMass() const
		{
			return centerOfMass;
		}

	protected:
		Vec3 centerOfMass;
	};

	class Sphere : public Shape
	{
	public:
		Sphere(float radius) 
			: radius(radius)
		{
			centerOfMass.Zero();
		}
		~Sphere() = default;

		ShapeType GetType() const override
		{
			return SPHERE;
		}
		float radius;

	private:

	};

}
