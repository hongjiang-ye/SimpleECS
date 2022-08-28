#pragma once


namespace ECS
{
	class World;

	class System
	{
	public:
		friend class World;

		System() {}

		virtual void Init() = 0;
		virtual void Update(double delta_time) = 0;

		virtual ~System() = default;  // since it's a class with virtual function

		World* world_ptr = nullptr;
	};
}
