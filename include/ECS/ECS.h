#pragma once
#include <map>
#include <list>
#include <functional>
#include <type_traits>

#include "EntityManager.h"
#include "System.h"


namespace ECS
{
	class World
	{
	public:

		World()
		{
			entity_mgr.Init();
		}

		void Update(double delta_time)
		{
			for (const auto& pair : systems) {
				for (System* system_ptr : pair.second) {
					system_ptr->Update(delta_time);
				}
			}
		}

		void AddSystem(System* system_ptr, int priority = 0)
		{
			systems[priority].push_back(system_ptr);
			system_ptr->Init();
			system_ptr->world_ptr = this;
		}

		EntityManager& GetEntityManager()
		{
			return this->entity_mgr;
		}

		// In g++, must use type traits to extract the type and must be qualified by typename. No need in MSVC.
		template<typename... Args>
		void ForEach(typename std::common_type<std::function<void(const Entity*, Args*...)>>::type func)
		{
			entity_mgr.ForEach<decltype(func), Args...>(func);
		}

	private:
		EntityManager entity_mgr;

		// Systems sorted by priority, highest first
		std::map<int, std::list<System*>, std::greater<int>> systems;
	};
}
