#pragma once
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

	private:

	};

	class Sphere : public Shape
	{
	public:
		Sphere(float radius) : radius(radius){}
		~Sphere() = default;

		ShapeType GetType() const override
		{
			return SPHERE;
		}
		float radius;

	private:

	};

}
