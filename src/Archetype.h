#pragma once
#include <unordered_set>

#include "ComponentType.h"


namespace ECS
{
	typedef size_t ArchetypeID;
	typedef std::unordered_set<ArchetypeID> ArchetypeIDSet;

	// An archetype object is just an identifier to each unique combination of component types
	class Archetype
	{
	public:

		// Must construct with given ID and set of component types.
		Archetype() = delete;

		Archetype(ArchetypeID a_id, ComponentTypeIDSet c_id_set)
			: id(a_id), component_type_ids(c_id_set)
		{}

		bool hasComponentType(const ComponentTypeID& c_id) const
		{
			return this->component_type_ids.count(c_id) != 0;
		}

		ComponentTypeIDSet GetComponentTypeIDs()
		{
			return this->component_type_ids;
		}

		const ComponentTypeIDSet& GetComponentTypeIDs() const
		{
			return this->component_type_ids;
		}

		const ArchetypeID& GetID() const
		{
			return this->id;
		}

	private:
		ArchetypeID id;
		ComponentTypeIDSet component_type_ids;
	};
}

namespace std
{
	using namespace ECS;

	// the hash function for Archetype
	template<> struct hash<Archetype>
	{
		size_t operator()(Archetype const& a_type) const noexcept
		{
			return std::hash<ComponentTypeID>{}(a_type.GetID());
		}
	};

	// the equal_to function for Archetype
	template<> struct equal_to<Archetype>
	{
		bool operator()(const Archetype& lhs, const Archetype& rhs) const
		{
			return lhs.GetComponentTypeIDs() == rhs.GetComponentTypeIDs() && lhs.GetID() == rhs.GetID();
		}
	};
}
