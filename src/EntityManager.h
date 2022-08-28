#pragma once
#include <iostream>
using std::cout;
using std::endl;

#include "ComponentTypeManager.h"
#include "ArchetypeManager.h"
#include "ComponentStorageManager.h"


namespace ECS {

	const Entity NULL_ENTITY;  // The default constructed entity should be an invalid entity

	class EntityManager {
	public:

		EntityManager() {}

		// Avoid unintentional copy
		EntityManager(const EntityManager&) = delete;
		EntityManager operator=(const EntityManager&) = delete;

		void Init() {
			component_type_mgr.Init();
			archetype_mgr.Init(&component_type_mgr);
			storage_mgr.Init(&component_type_mgr, &archetype_mgr);
		}

		// Create an entity with the specified combination of component types, and their data are defaultly constructed.
		template <typename... Args>
		Entity CreateEntity() {
			Entity new_entity = Entity(entity_id_counter++);
			entities.insert(new_entity);
			
			ComponentTypeIDSet c_id_set = ComponentTypeIDSet{ component_type_mgr.GetOrCreateComponentTypeID<Args>()...};
			ArchetypeID a_id = archetype_mgr.GetOrCreateArchetype(c_id_set);
			
			storage_mgr.AddEntity<Args...>(new_entity, a_id);

			return new_entity;
		}

		// Create an entity with no component type
		Entity CreateEntity() {
			Entity new_entity = Entity(entity_id_counter++);
			null_entities.insert(new_entity);

			return new_entity;
		}

		// Add a new component (or replace the old one) to an entity.
		template <typename T, typename... Args>
		void AddEntityComponent(const Entity& entity, const Args&... args) {

			// T is component type
			ComponentTypeID add_c_id = component_type_mgr.GetOrCreateComponentTypeID<T>();
			
			if (null_entities.count(entity) != 0) {
				// the entity has no component yet
				ArchetypeID a_id = archetype_mgr.GetOrCreateArchetype(ComponentTypeIDSet{ add_c_id });
				storage_mgr.AddEntity<T>(entity, a_id);
				
				null_entities.erase(entity);
				entities.insert(entity);
			}
			else {
				if (!storage_mgr.HasComponentType(entity, add_c_id)) {  
					// the entity doesn't have this component yet
					ArchetypeID old_a_id = storage_mgr.GetEntityArchetypeID(entity);
					ComponentTypeIDSet c_id_set = archetype_mgr.GetArchtype(old_a_id).GetComponentTypeIDs();
					c_id_set.insert(add_c_id);
					ArchetypeID new_a_id = archetype_mgr.GetOrCreateArchetype(c_id_set);

					storage_mgr.MigrateEntity(entity, new_a_id);
				}
			}
			// the entity already has this component, then just emplace it with new value. 
			storage_mgr.SetEntityComponent<T, Args...>(entity, args...);
		}

		// Set a new component value fpr an entity.
		template <typename T, typename... Args>
		void SetEntityComponent(const Entity& entity, const Args&... args) {
			storage_mgr.SetEntityComponent<T, Args...>(entity, args...);
		}

		// Get the pointer to the component type T of an entity.
		template <typename T>
		T* GetEntityComponent(const Entity& entity) {
			return storage_mgr.GetEntityComponent<T>(entity);
		}

		// Whether an entity has component T.
		template <typename T>
		bool HasComponent(const Entity& entity) 
		{
			ComponentTypeID c_id = component_type_mgr.GetOrCreateComponentTypeID<T>();
			return storage_mgr.HasComponentType(entity, c_id);
		}

		// Whether an entity has the list of components.
		template <typename T, typename V, typename... Args>
		bool HasComponent(const Entity& entity) 
		{
			ComponentTypeIDSet c_id_set = ComponentTypeIDSet{ component_type_mgr.GetOrCreateComponentTypeID<T>(),
				component_type_mgr.GetOrCreateComponentTypeID<V>(),
				component_type_mgr.GetOrCreateComponentTypeID<Args>()... };

			for (auto const& c_id : c_id_set) {
				if (!storage_mgr.HasComponentType(entity, c_id)) {
					return false;
				}
			}

			return true;
		}

		// Remove the component T from an entity.
		template <typename T>
		void RemoveEntityComponent(const Entity& entity) {
			ComponentTypeID remove_c_id = component_type_mgr.GetOrCreateComponentTypeID<T>();

			if (null_entities.count(entity) != 0 || !storage_mgr.HasComponentType(entity, remove_c_id)) {
				// Nothing to remove;
				return;
			}
			
			// the entity doesn't have this component yet
			ArchetypeID old_a_id = storage_mgr.GetEntityArchetypeID(entity);
			ComponentTypeIDSet c_id_set = archetype_mgr.GetArchtype(old_a_id).GetComponentTypeIDs();
			c_id_set.erase(remove_c_id);

			if (c_id_set.size() == 0) {
				this->RemoveEntityAllComponents(entity);
			}
			else {
				ArchetypeID new_a_id = archetype_mgr.GetOrCreateArchetype(c_id_set);
				storage_mgr.MigrateEntity(entity, new_a_id);
			}
		}

		// Remove all components from an entity
		void RemoveEntityAllComponents(const Entity& entity) {
			storage_mgr.RemoveEntity(entity);
			null_entities.insert(entity);
			entities.erase(entity);
		}

		std::unordered_set<Entity>& GetEntities() {
			return entities;
		}

		// For debug
		void PrintEntitiesInfo() const {
			cout << "World has " << entities.size() << " entities: ID ";
			for (auto const& entity : entities) {
				cout << entity.id << ", ";
			}
			cout << endl;
		}

		void PrintComponentTypesInfo() const {
			this->component_type_mgr.PrintComponentTypesInfo();
		}

		void PrintArchetypesInfo() const {
			this->archetype_mgr.PrintArchetypesInfo();
		}

		void PrintComponentStorageInfo() const{
			this->storage_mgr.PrintComponentStorageInfo();
		}

		template <typename F, typename... Args>
		void ForEach(F func) {
			// 要考虑遍历时对 entity 的增删的问题。
			ComponentTypeIDSet c_id_set = { component_type_mgr.GetOrCreateComponentTypeID<Args>()... };

			for (const auto& a_id : archetype_mgr.GetArchetypeContains(c_id_set)) {
				std::vector<Entity> entities = storage_mgr.GetEntities(a_id);
				for (const auto& entity : entities) {
					func(&entity, this->GetEntityComponent<Args>(entity)...);
				}
			}
		}

	private:
		// Store entities by a hash map since we may create and delete entities frequently.
		std::unordered_set<Entity> entities;
		size_t entity_id_counter = 1;  // Grows from 1; 0 is the invalid entity's ID.
		std::unordered_set<Entity> null_entities;

		// manage component types
		ComponentTypeManager component_type_mgr;

		// manage all archetypes
		ArchetypeManager archetype_mgr;

		// manage the archetype of each entity, entity's components storage, orgainzed by archetype in chunks
		ComponentStorageManager storage_mgr;
	};
}
