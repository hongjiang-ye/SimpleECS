#pragma once
#include <functional>
        

namespace ECS {

	struct Entity {
		size_t id;

        Entity() : id(0) {}
        Entity(size_t id) : id(id) {}
	};

}

namespace std {

    using namespace ECS;

    // the hash function for entity
    template<> struct hash<Entity> {
        size_t operator()(Entity const& e) const noexcept {
            return std::hash<size_t>{}(e.id);
        }
    };

    // the equal_to function for Entity
    template<> struct equal_to<Entity> {
        bool operator()(const Entity& lhs, const Entity& rhs) const
        {
            return lhs.id == rhs.id;
        }
    };

}
