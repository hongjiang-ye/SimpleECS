#pragma once
#include <cassert>
#include <typeindex>
#include <iostream>
using std::cout;
using std::endl;

#include "ComponentType.h"


namespace ECS {

	namespace Internal {
		// Extract the type information (an identifier and a name), through RTTI or compile-time processing;
		// Current version: generate an ID and name for type T by RTTI.
		template <typename T>
		std::pair<size_t, std::string> GetTypeInfo() {
			std::type_index c_rtti_type_index = std::type_index(typeid(T));
			size_t internal_id = c_rtti_type_index.hash_code();
			std::string name = c_rtti_type_index.name();

			return std::make_pair(internal_id, name);
		}
	}

	class ComponentTypeManager {
	public:

		ComponentTypeManager() {}

		// Avoid unintentional copy
		ComponentTypeManager(const ComponentTypeManager&) = delete;
		ComponentTypeManager operator=(const ComponentTypeManager&) = delete;

		void Init() {}

		const ComponentType& GetComponentType(ComponentTypeID c_id) const {
			return this->component_types[c_id];
		}

		template <typename T>
		ComponentTypeID GetComponentTypeID() const {
			auto type_info = Internal::GetTypeInfo<T>();
			size_t internal_id = type_info.first;

			return component_ids_by_internal_id.at(internal_id);
		}

		template <typename T>
		ComponentTypeID GetOrCreateComponentTypeID() {
			auto type_info = Internal::GetTypeInfo<T>();
			size_t internal_id = type_info.first;
			std::string name = type_info.second;

			if (component_ids_by_internal_id.count(internal_id) == 0) {
				// Create new component type
				size_t new_c_id = component_type_id_counter++;

				component_ids_by_internal_id.insert({ internal_id, new_c_id });
				component_types.emplace_back(new_c_id, sizeof(T), name);

				assert(component_type_id_counter == component_types.size());
			}

			return component_ids_by_internal_id.at(internal_id);
		}

		void PrintComponentTypesInfo() const {
			cout << "\n====== Component Type Info ======" << endl;

			cout << "World has " << this->component_types.size() << " component types:" << endl;
			for (const auto& c_type : this->component_types) {
				cout << "ComponentType ID " << c_type.id 
					<< ": Name: " << c_type.name << ", Size: " << c_type.size << endl;
			}
		}

	private:
		// Index by component type ID, which grows from 0;
		std::vector<ComponentType> component_types;
		ComponentTypeID component_type_id_counter = 0;

		// A mapping from internal type id (arbitrary size_t) to component type id (grows from 0)
		std::unordered_map<size_t, ComponentTypeID> component_ids_by_internal_id;
	};
}