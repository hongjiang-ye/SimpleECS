#pragma once
#include <cassert>
#include <iostream>
using std::cout;
using std::endl;

#include "ArchetypeStorage.h"


namespace ECS
{
	// Manage the entity's archetype and data storage
	class ComponentStorageManager
	{
	public:

		ComponentStorageManager() {}

		// Avoid unintentional copy
		ComponentStorageManager(const ComponentStorageManager&) = delete;
		ComponentStorageManager operator=(const ComponentStorageManager&) = delete;

		void Init(const ComponentTypeManager* c_mgr_ptr, const ArchetypeManager* a_mgr_ptr)
		{
			this->c_mgr_ptr = c_mgr_ptr;
			this->a_mgr_ptr = a_mgr_ptr;
		}

		void AddArchetype(const ArchetypeID& a_id)
		{
			assert(archetype_storage_ptr_by_id.count(a_id) == 0);
			archetype_storage_ptr_by_id.insert({ a_id, new ArchetypeStorage(c_mgr_ptr, a_mgr_ptr, a_id) });
		}

		// Add entity with given archetype, and default construct all components
		template <typename... Args>
		void AddEntity(const Entity& new_entity, const ArchetypeID& a_id)
		{
			if (archetype_storage_ptr_by_id.count(a_id) == 0) {
				this->AddArchetype(a_id);
			}
			archetype_id_by_entity.insert({ new_entity, a_id });

			ArchetypeStorage* a_store_ptr = archetype_storage_ptr_by_id.at(a_id);
			a_store_ptr->AddEntity(new_entity);
			(this->DefaultConstructEntityComponent<Args>(new_entity), ...);
		}

		template <typename T>
		void DefaultConstructEntityComponent(const Entity& entity)
		{
			T* address = this->GetEntityComponent<T>(entity);
			new (address) T();
		}

		template <typename T, typename... Args>
		T* DirectConsturctEntityComponent(const Entity& entity, const Args&... args)
		{
			T* address = this->GetEntityComponent<T>(entity);
			return new (address) T(args...);
		}

		template <typename T>
		T* GetEntityComponent(const Entity& entity) const
		{
			ArchetypeStorage* a_store_ptr = GetEntityArchetypeStorage(entity);
			ComponentTypeID c_id = c_mgr_ptr->GetComponentTypeID<T>();
			void* address = a_store_ptr->GetComponentDataAddress(entity, c_id);
			return static_cast<T*>(address);
		}

		template <typename T, typename... Args>
		T* SetEntityComponent(const Entity& entity, const Args&... args)
		{
			T* address = this->GetEntityComponent<T>(entity);
			return new (address) T(args...);
		}

		void MigrateEntity(const Entity& entity, const ArchetypeID& new_a_id)
		{
			ArchetypeStorage* src_a_store_ptr = GetEntityArchetypeStorage(entity);

			if (archetype_storage_ptr_by_id.count(new_a_id) == 0) {
				this->AddArchetype(new_a_id);
			}
			ArchetypeStorage* dest_a_store_ptr = archetype_storage_ptr_by_id.at(new_a_id);

			src_a_store_ptr->MigrateEntity(entity, dest_a_store_ptr);
			archetype_id_by_entity.at(entity) = new_a_id;
		}

		template <typename T, typename... Args>
		T* AddEntityComponent(const Entity& entity, const ArchetypeID& new_a_id, const Args&... args)
		{
			this->MigrateEntity(entity, new_a_id);

			ArchetypeStorage* new_a_store_ptr = archetype_storage_ptr_by_id.at(new_a_id);
			ComponentTypeID c_id = c_mgr_ptr->GetComponentTypeID<T>();

			void* address = new_a_store_ptr->GetComponentDataAddress(entity, c_id);
			new (address) T(args...);
			return static_cast<T*>(address);
		}

		ArchetypeID GetEntityArchetypeID(const Entity& entity) const
		{
			return archetype_id_by_entity.at(entity);
		}

		void RemoveEntity(const Entity& entity)
		{
			assert(archetype_id_by_entity.count(entity) != 0);

			ArchetypeStorage* a_store_ptr = this->GetEntityArchetypeStorage(entity);
			archetype_id_by_entity.erase(entity);
			a_store_ptr->RemoveEntityData(entity);
		}

		bool HasComponentType(const Entity& entity, const ComponentTypeID& c_id) const
		{
			if (archetype_id_by_entity.count(entity) == 0) {
				// an empty entity with no component
				return false;
			}
			ArchetypeID a_id = archetype_id_by_entity.at(entity);
			return a_mgr_ptr->GetArchtype(a_id).hasComponentType(c_id);
		}

		std::vector<Entity> GetEntities(ArchetypeID a_id)
		{
			return archetype_storage_ptr_by_id.at(a_id)->GetEntities();
		}

		void PrintComponentStorageInfo() const
		{
			cout << "\n====== Component Storage Info ======" << endl;
			cout << endl;

			for (const auto& pair : archetype_storage_ptr_by_id) {
				cout << "\n=== Archetype ID: " << pair.first << "===" << endl;
				cout << "Entities: ";
				for (const auto& p : archetype_id_by_entity) {
					if (p.second == pair.first) {
						cout << p.first.id << ", ";
					}
				}
				cout << endl;

				cout << "Storage:" << endl;
				pair.second->PrintInfo();
			}
		}

	private:
		ArchetypeStorage* GetEntityArchetypeStorage(const Entity& entity) const
		{
			ArchetypeID a_id = archetype_id_by_entity.at(entity);
			return archetype_storage_ptr_by_id.at(a_id);
		}

		const ArchetypeManager* a_mgr_ptr = nullptr;
		const ComponentTypeManager* c_mgr_ptr = nullptr;

		std::unordered_map<ArchetypeID, ArchetypeStorage*> archetype_storage_ptr_by_id;

		// Store each entity's archetype
		std::unordered_map<Entity, ArchetypeID> archetype_id_by_entity;
	};
}
