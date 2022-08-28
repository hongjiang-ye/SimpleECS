#pragma once
#include <typeindex>
#include <unordered_set>
#include <iostream>


namespace ECS {

    typedef size_t ComponentTypeID;
    typedef std::unordered_set<ComponentTypeID> ComponentTypeIDSet;

	struct ComponentType {
    
        ComponentTypeID id;
		size_t size;
		std::string name;

		ComponentType() : id(0), size(0), name("NULL_COMPONENT_TYPE") {}

		ComponentType(ComponentTypeID id, size_t size, std::string name) :
			id(id), size(size), name(name) {}
	};
}

namespace std {

    using namespace ECS;

    // define hash functions
    template<> struct hash<ComponentType> {
        size_t operator()(const ComponentType& c_type) const noexcept {
            return std::hash<ComponentTypeID>{}(c_type.id);
        }
    };

    template<> struct hash<ComponentTypeIDSet> {
        size_t operator()(const ComponentTypeIDSet& c_id_set) const noexcept {
            size_t val = 0;
            for (const auto& c_id : c_id_set) {
                val ^= std::hash<ComponentTypeID>{}(c_id);
            }
            return val;
        }
    };

    // the equal_to function for ComponentType
    template<> struct equal_to<ComponentType> {
        bool operator()(const ComponentType& lhs, const ComponentType& rhs) const
        {
            return lhs.id == rhs.id;
        }
    };
}
