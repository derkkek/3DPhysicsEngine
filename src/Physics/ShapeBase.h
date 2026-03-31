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
			BOX,
			CONVEX
		};
		Shape() = default;
		~Shape() = default;

		virtual void Build(const Vec3* points, const int num){}
		virtual Vec3 Support(const Vec3& dir, const Vec3& pos, const Quat& orient, const float bias) const = 0;
		virtual float FastestLinearSpeed(const Vec3& angularVelocity, const Vec3& dir) const { return  0.0f; }

		virtual ShapeType GetType() const = 0;
		virtual Bounds GetBounds(const Vec3& pos, const Quat& orient) const = 0;
		virtual Bounds GetBounds() const = 0;

		virtual Vec3 GetCenterOfMass() const
		{
			return centerOfMass;
		}

		virtual Mat3 InertiaTensor() const = 0;

		Bounds bounds;


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

		virtual Vec3 Support(const Vec3& dir, const Vec3& pos, const Quat& orient, const float bias) const override;

		Mat3 InertiaTensor() const override;

		Bounds GetBounds(const Vec3& pos, const Quat& orient) const override;
		Bounds GetBounds() const override;
	private:

	};

	class Box : public Shape 
	{
	public:
		explicit Box(const Vec3* pts, const int num) {
			Build(pts, num);
		}
		void Build(const Vec3* pts, const int num);

		Vec3 Support(const Vec3& dir, const Vec3& pos, const Quat& orient, const float bias) const override;

		Mat3 InertiaTensor() const override;

		Bounds GetBounds(const Vec3& pos, const Quat& orient) const override;
		Bounds GetBounds() const override { return bounds; }

		float FastestLinearSpeed(const Vec3& angularVelocity, const Vec3& dir) const override;

		ShapeType GetType() const override { return BOX; }

		std::vector< Vec3 > points;

	};

	class Convex : public Shape {
	public:
		explicit Convex(const Vec3* pts, const int num) {
			Build(pts, num);
		}
		void Build(const Vec3* pts, const int num);

		Vec3 Support(const Vec3& dir, const Vec3& pos, const Quat& orient, const float bias) const override;

		Mat3 InertiaTensor() const override { return inertiaTensor; }

		Bounds GetBounds(const Vec3& pos, const Quat& orient) const override;
		Bounds GetBounds() const override { return bounds; }

		float FastestLinearSpeed(const Vec3& angularVelocity, const Vec3& dir) const override;

		ShapeType GetType() const override { return CONVEX; }

		std::vector< Vec3 > points;
		Mat3 inertiaTensor;
	};

}
