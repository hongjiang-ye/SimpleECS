#pragma once
#include <unordered_map>
#include <cassert>

#include "Entity.h"
#include "ComponentTypeManager.h"
#include "Archetype.h"


namespace ECS
{
	// Manage all archetypes that are existing or once existed in the current world.
	class ArchetypeManager
	{
	public:

		ArchetypeManager() {}

		// Avoid unintentional copy
		ArchetypeManager(const ArchetypeManager&) = delete;
		ArchetypeManager operator=(const ArchetypeManager&) = delete;

		void Init(ComponentTypeManager* c_mgr_ptr)
		{
			this->c_mgr_ptr = c_mgr_ptr;
		}

		ArchetypeID CreateArchetype(const ComponentTypeIDSet& c_id_set)
		{
			assert(archetype_id_by_component_set.count(c_id_set) == 0);

			ArchetypeID new_a_id = this->archetype_id_counter++;
			archetype_id_by_component_set.insert({ c_id_set,  new_a_id });
			archetypes.push_back(Archetype(new_a_id, c_id_set));

			assert(archetypes.size() == archetype_id_counter);

			return new_a_id;
		}

		ArchetypeID GetOrCreateArchetype(const ComponentTypeIDSet& c_id_set)
		{
			if (archetype_id_by_component_set.count(c_id_set) == 0) {
				this->CreateArchetype(c_id_set);
			}
			return archetype_id_by_component_set.at(c_id_set);
		}

		const Archetype& GetArchtype(const ArchetypeID& a_id) const
		{
			return archetypes[a_id];
		}

		ArchetypeIDSet GetArchetypeContains(const ComponentTypeIDSet& c_id_set)
		{
			ArchetypeIDSet a_id_set{};
			for (const auto& pair : archetype_id_by_component_set) {
				// check if a subset (we can do better if change the underlying data structure)
				bool is_subset = true;
				for (const auto& c_id : c_id_set) {
					if (pair.first.count(c_id) == 0) {
						is_subset = false;
						break;
					}
				}
				if (is_subset) {
					a_id_set.insert(pair.second);
				}
			}
			return a_id_set;
		}

		void PrintArchetypesInfo() const
		{
			cout << "\n====== Archetype Info ======" << endl;

			cout << "World has " << archetypes.size() << " archetypes:" << endl;
			for (const auto& archetype : archetypes) {
				cout << "ArchetypeID " << archetype.GetID() << ": " << endl;
				cout << "\tComponents: ";
				for (const auto& c_id : archetype.GetComponentTypeIDs()) {
					cout << c_mgr_ptr->GetComponentType(c_id).name << ", ";
				}
				cout << endl;
			}
		}

	private:
		const ComponentTypeManager* c_mgr_ptr = nullptr;

		// Index by archetype ID, which grows from 0;
		std::vector<Archetype> archetypes;
		size_t archetype_id_counter = 0;

		// A fast retrival of archetype ID by combination of component types.
		std::unordered_map<ComponentTypeIDSet, ArchetypeID> archetype_id_by_component_set;
	};
}
