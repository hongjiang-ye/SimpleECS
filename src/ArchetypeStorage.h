#pragma once
#include <cassert>
#include <cstring>
#include <iostream>
using std::cout;
using std::endl;

#include "EntityManager.h"
#include "Chunk.h"
#include "ComponentTypeManager.h"
#include "ArchetypeManager.h"


namespace ECS
{
	struct EntityIndex
	{
		EntityIndex() : chunk_index(-1), col_index(-1) {}
		EntityIndex(size_t chunk_index, size_t col_index) : chunk_index(chunk_index), col_index(col_index) {}

		size_t chunk_index;
		size_t col_index;
	};

	inline bool operator==(const EntityIndex& lhs, const EntityIndex& rhs)
	{
		return lhs.chunk_index == rhs.chunk_index && lhs.col_index == rhs.col_index;
	}

	inline bool operator!=(const EntityIndex& lhs, const EntityIndex& rhs)
	{
		return !(lhs == rhs);
	}

	// Manage the archetype's storage-related information and chunk storage.
	struct ArchetypeStorage
	{

		// Avoid unintentional copy and default construction
		ArchetypeStorage() = delete;
		ArchetypeStorage(const ArchetypeStorage&) = delete;
		ArchetypeStorage operator=(const ArchetypeStorage&) = delete;

		ArchetypeStorage(const ComponentTypeManager* c_mgr_ptr, const ArchetypeManager* a_mgr_ptr, const ArchetypeID& a_id)
		{
			const Archetype& archetype = a_mgr_ptr->GetArchtype(a_id);
			const ComponentTypeIDSet& c_id_set = archetype.GetComponentTypeIDs();

			// Init component-related info
			size_t row_index = 0;
			size_t total_components_size = 0;

			for (const auto& c_id : c_id_set) {
				component_types.push_back(c_id);
				component_type_index_by_id.insert({ c_id, row_index });

				size_t c_size = c_mgr_ptr->GetComponentType(c_id).size;
				row_sizeofs.push_back(c_size);

				total_components_size += c_size;
				row_index++;
			}

			chunk_entity_capacity = chunk_size / total_components_size;
			this->CreateNewChunk();
		}

		void AddEntity(const Entity& new_entity)
		{
			EntityIndex new_e_index = this->GetEmptyEntityIndex();
			AddEntityToIndex(new_entity, new_e_index);
		}

		void* GetComponentDataAddress(const Entity& entity, const ComponentTypeID& c_id)
		{
			assert(component_type_index_by_id.count(c_id) != 0);
			size_t row_index = component_type_index_by_id[c_id];
			EntityIndex e_index = entity_indices[entity];

			return GetComponentDataAddress(e_index, row_index);
		}

		void MigrateEntity(const Entity& entity, ArchetypeStorage* const dest_a_storage_ptr)
		{
			EntityIndex src_e_index = entity_indices.at(entity);
			EntityIndex dest_e_index = dest_a_storage_ptr->GetEmptyEntityIndex();

			this->CopyEntityData(src_e_index, dest_e_index, dest_a_storage_ptr);
			this->RemoveEntityData(entity);
			dest_a_storage_ptr->AddEntityToIndex(entity, dest_e_index);
		}

		// Erase an entity's data, and fill the hole with the last entry of data at the current chunk
		// to keep the entity data continuous.
		void RemoveEntityData(const Entity& entity)
		{
			EntityIndex e_index = entity_indices.at(entity);

			EntityIndex last_e_index(e_index.chunk_index, cur_entity_count[e_index.chunk_index] - 1);
			Entity last_e_entity = archetype_entities[last_e_index.chunk_index][last_e_index.col_index];

			if (last_e_index != e_index) {  // if e_index is not the last entry in current chunk
				this->CopyEntityData(last_e_index, e_index, this);
			}

			cur_entity_count[e_index.chunk_index]--;
			archetype_entities[e_index.chunk_index][e_index.col_index] = last_e_entity;
			entity_indices.at(last_e_entity) = e_index;
			entity_indices.erase(entity);
		}

		// Copy all entities (potential performance issue to be addressed!)
		// Must copy the entities instead of using an indirect link, because the entity storage 
		// may be altered during ForEach update.
		std::vector<Entity> GetEntities()
		{
			std::vector<Entity> entities;
			for (size_t i = 0; i < chunks.size(); i++) {
				for (size_t j = 0; j < cur_entity_count[i]; j++) {
					entities.push_back(archetype_entities[i][j]);
				}
			}
			return entities;
		}

		void PrintInfo() const
		{
			cout << "Has " << component_types.size() << " components (rows):" << endl;;
			for (size_t i = 0; i < component_types.size(); i++) {
				cout << "\tCID: " << component_types[i] << " Size: " << row_sizeofs[i] << endl;
			}

			cout << "Has " << chunks.size() << " chunks." << endl;
			for (size_t i = 0; i < chunks.size(); i++) {
				cout << "\tChunk " << i << ": contains " << cur_entity_count[i] << " entities: ";
				for (size_t j = 0; j < cur_entity_count[i]; j++) {
					cout << archetype_entities[i][j].id << ", ";
				}
				cout << endl;
			}
		}

	private:

		void CreateNewChunk()
		{
			chunks.push_back(new Chunk(chunk_size));
			cur_entity_count.push_back(0);
			archetype_entities.push_back(std::vector<Entity>(chunk_entity_capacity));
		}

		void AddEntityToIndex(const Entity& new_entity, const EntityIndex& e_index)
		{
			assert(entity_indices.count(new_entity) == 0);

			archetype_entities[e_index.chunk_index][e_index.col_index] = new_entity;
			entity_indices.insert({ new_entity, e_index });
			cur_entity_count[e_index.chunk_index]++;
		}

		// Computer the data component address by given chunk_index, column_index (entity) and row_index (component).
		void* GetComponentDataAddress(const EntityIndex& e_index, size_t row_index)
		{
			return chunks[e_index.chunk_index]->GetAddress(row_index, e_index.col_index, row_sizeofs, chunk_entity_capacity);
		}

		void CopyEntityData(const EntityIndex& src_e_index, const EntityIndex& dest_e_index, ArchetypeStorage* const dest_a_storage_ptr)
		{
			const std::vector<ComponentTypeID>& dest_component_types = dest_a_storage_ptr->component_types;

			for (const auto& c_id : dest_component_types) {
				if (component_type_index_by_id.count(c_id) == 0) {
					continue;  // only move component types that the destination archetype has.
				}
				size_t src_row_index = this->component_type_index_by_id.at(c_id);
				size_t dest_row_index = dest_a_storage_ptr->component_type_index_by_id.at(c_id);
				void* src_c_data_address = this->GetComponentDataAddress(src_e_index, src_row_index);
				void* dest_c_data_address = dest_a_storage_ptr->GetComponentDataAddress(dest_e_index, dest_row_index);
				std::memcpy(dest_c_data_address, src_c_data_address, row_sizeofs[component_type_index_by_id.at(c_id)]);
			}
		}

		EntityIndex GetEmptyEntityIndex()
		{
			for (size_t i = 0; i < cur_entity_count.size(); i++) {
				if (cur_entity_count[i] < chunk_entity_capacity) {
					return EntityIndex(i, cur_entity_count[i]);
				}
			}

			// If all chunks are full, allocate a new chunk.
			this->CreateNewChunk();
			return EntityIndex(chunks.size() - 1, 0);
		}

		// All chunks storing data for this archetype; use vector because we'll only add or delete chunks at the end
		std::vector<Chunk*> chunks;

		/**
		* Entity-related info (column index of chunk)
		*/
		// Column index: All entities having this archetype
		std::vector<std::vector<Entity>> archetype_entities;

		// The inverted column index [chunk_id][column_id] of each entity having this archetype
		std::unordered_map<Entity, EntityIndex> entity_indices;

		// 	The number of entities currently stored in the chunk
		std::vector<size_t> cur_entity_count;

		/**
		* Component-related info (row index of chunk)
		*/
		// The size of each component type
		std::vector<size_t> row_sizeofs;

		// Row index: All components in this archetype
		std::vector<ComponentTypeID> component_types;

		// The inverted row index for fast lookup
		std::unordered_map<ComponentTypeID, size_t> component_type_index_by_id;

		/**
		* Chunk properties, determined at construction
		*/
		// Default 16K chunk size
		const size_t chunk_size = 16384;

		// The number of entities having this archetype that can fit into a single chunk
		size_t chunk_entity_capacity;
	};

}
