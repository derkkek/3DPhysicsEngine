#pragma once
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Math/Bounds.h"
#include "Math/Quat.h"
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
		virtual Bounds GetBounds(const Vec3& pos, const Quat& orient) const = 0;
		virtual Bounds GetBounds() const = 0;

		virtual Vec3 GetCenterOfMass() const
		{
			return centerOfMass;
		}

		virtual Mat3 InertiaTensor() const = 0;

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

		Mat3 InertiaTensor() const override;

		Bounds GetBounds(const Vec3& pos, const Quat& orient) const override;
		Bounds GetBounds() const override;
	private:

	};

}
