# SimpleECS

TODO: 

* googletest
* 粒子demo

A simple Entity Component System (ECS) engine implemented in C++. 

This library has not tested for any production purpose, and is mostly a wheel built for trying to implement some basic functions of ECS.

## Features

* Implements the "guaranteed perfect" memory model for ECS: component data for entities of a single archetype is stored contiguously as SOA in one or more memory chunks. 

* No boilerplate component registration: Component types are recognized by RTTI (though of course, with overheads) and registered automatically.
* Simple template-based APIs (which requires C++17).

* Being header-only and has no third-party dependencies.

## Example

```c++
#include <iostream>
#include "ECS.h"

struct Position {
	// Components must have a default constructor.
	Position() : x(10.0), y(10.0) {}

	// Components can have any other direct constructors.
	Position(double x, double y) : x(x), y(y) {}

	double x;
	double y;
};

struct Name
{
	Name(std::string name) : name(name) {}
	Name() : name("default") {}

	std::string name;
};

struct Dummy {};

struct DecreseNamedPositionSystem : ECS::System
{
	DecreseNamedPositionSystem() {}

	virtual void Init() override {}
	virtual void Update(double delta_time) override
	{
		this->world_ptr->ForEach<Position, Name>(
			[&](const ECS::Entity* entity_ptr, Position* pos_ptr, Name* name_ptr) -> void {
			pos_ptr->x -= delta_time * 2;
			pos_ptr->y -= delta_time;

			std::cout << "Processing entity " << name_ptr->name <<
				", x: " << pos_ptr->x << " y: " << pos_ptr->y << std::endl;
		});
	}
};

int main()
{
	ECS::World world;
	ECS::EntityManager& entity_mgr = world.GetEntityManager();

	world.AddSystem(new DecreseNamedPositionSystem());

	// Create entities with a list of components, which will be constructed by default.
	ECS::Entity entity_1 = entity_mgr.CreateEntity();
	ECS::Entity entity_2 = entity_mgr.CreateEntity<Position>();
	ECS::Entity entity_3 = entity_mgr.CreateEntity<Position>();
	ECS::Entity entity_4 = entity_mgr.CreateEntity<Position, Name, Dummy>();
	ECS::Entity entity_5 = entity_mgr.CreateEntity<Name>();

	// Add or set a component to an entity, and directly consturct the component with given arguments.
	entity_mgr.AddEntityComponent<Name>(entity_2, "Entity 2");
	entity_mgr.SetEntityComponent<Name>(entity_4, "Entity 4");

	// Systems are being called, ordered by their priority
	std::cout << "Update 1.0:" << std::endl;
	world.Update(1.0);

	// Component removal
	entity_mgr.RemoveEntityComponent<Name>(entity_4);
	entity_mgr.RemoveEntityAllComponents(entity_2);

	entity_mgr.AddEntityComponent<Position>(entity_1, 20.0, 30.0);
	entity_mgr.AddEntityComponent<Name>(entity_1, "Entity 1");

	entity_mgr.AddEntityComponent<Position>(entity_5, 50.0, 60.0);
	entity_mgr.SetEntityComponent<Name>(entity_5, "Entity 5");

	// Another set of entities will be visited by the system.
	std::cout << "\nUpdate 2.0:" << std::endl;
	world.Update(2.0);

	// Directly access an entity's component.
	std::cout << "\nEntity 4's x: " << entity_mgr.GetEntityComponent<Position>(entity_4)->x << std::endl;
}
```

The outputs are:

```text
Update 1.0:
Processing entity Entity 4, x: 8 y: 9
Processing entity Entity 2, x: 8 y: 9

Update 2.0:
Processing entity Entity 1, x: 16 y: 28
Processing entity Entity 5, x: 46 y: 58

Entity 4's x: 8
```

## Architecture

The overall architecture of the implemented ECS is roughly illustrated as follows:

![](https://s3.bmp.ovh/imgs/2022/08/28/6aa829a48ae60c0c.png)

* `World`: Manage all entities, components, and systems.

  * `System`: Base system classes stored and called by the `world`.

  * `EntityManager`: The core module that manages all entity and component related stuff.
    * `ComponentTypeManager`: Manages the component type registration.
    * `ArchetypeManager`: Managers the archetype registration.
    * `ComponentStorageManager`: Manage the storage of archetypes.

## Possible TODOs

* Try to reduce the RTTI (such as support for compile-time component registration) and template instantiation overheads.
* Faster entity iterators for `ForEach` call.
* Implement an event system to allow communication between systems.
* Allow shared-components.
* Exception handling.
* CPU and memory profiling to validate the data I/O performance.