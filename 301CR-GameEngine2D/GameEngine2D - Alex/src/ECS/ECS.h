#ifndef ECS_H
#define ECS_H

#include "../Logger/Logger.h"
#include <bitset>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <set>
#include <deque>
#include <memory>
//#include <string>
//#include <iostream>

const unsigned int MAX_COMPONENTS = 32;

//SIGNATURE 
//Use a bit  to keep track of which compenents  an entity has.
//And also to keep track of which entites a system is interested in.
typedef std::bitset<MAX_COMPONENTS> Signature;

struct BaseComponent {
protected:
	static int nextId;
};

//Used to assign a unique id to a component type
template  <typename T>
class Component : public BaseComponent{
	public:
		//Returns the unique id of Component<T>
		static int GetId() {
			static auto id = nextId++;
			return id;
		}
};

class Entity {
	private:
		int id;

public:
	Entity(int id) : id(id) {};
	Entity(const Entity& entity) = default;
	void Kill();
	int GetId() const;

	//Manage entity tags  and groups
	void Tag(const std::string& tag);
	bool HasTag(const std::string& tag) const;
	void Group(const std::string& group);
	bool BelongsToGroup(const std::string& group) const;


	//Manage entity components
	template <typename TComponent, typename...TArgs> void AddComponent(TArgs&& ...args);
	template <typename TComponent> void RemoveComponent();
	template <typename TComponent> bool HasComponent() const;
	template <typename TComponent> TComponent& GetComponent() const;

	//Operator overloading  for entity objects
	Entity& operator =(const Entity& other) = default;
	bool operator ==(const Entity & other) const { return id == other.id; }
	bool operator !=(const Entity & other) const { return id != other.id; }
	bool operator >(const Entity & other) const { return id > other.id; }
	bool operator <(const Entity & other) const { return id < other.id; }

	//Holds a apointer to the entity's owner registry
	class Registry* registry;

};

//System
//The system processes entities that contain a specific signature
class System 
{
private:
	Signature componentSignature;
	std::vector<Entity> entities;
public:
	System() = default;
	~System() = default;

	void AddEntityToSystem(Entity entity);
	void RemoveEntityFromSystem(Entity entity);
	std::vector <Entity> GetSystemEntities() const;
	const Signature& GetComponentSignature() const;

	//Defines the component type that entities must have to be considered by the system.
	template <typename TComponent> void RequireComponent();

};

//Pool
//A pool is just a vector of objects of typeT (continous data)
class IPool {
	public:
		virtual ~IPool() = default;
		virtual void RemoveEntityFromPool(int entityId) = 0;
};

template<typename T>
class Pool : public IPool {
	private:

		//Keeps track of the vector of objects an dthe current number of elements
		std::vector<T> data;
		int size;

		//Helper maps to keep track of the entity index, so the vector is always packed
		std::unordered_map<int, int> entityIdToIndex;
		std::unordered_map<int, int> indexToEntityId;


	public:
		Pool(int capacity = 100) {
			size = 0;
			data.resize(capacity);
		}
		virtual ~Pool() = default;

		bool IsEmpty() const {
			return size == 0;
		}

		int GetSize() const {
			return size;
		}

		void Resize(int n) {
			data.resize(n);
		}

		void Clear() {
			data.clear();
			size = 0;
		}

		void Add(T object) {
			data.push_back(object);
		}

		void Set(int entityId, T object) {
			if (entityIdToIndex.find(entityId) != entityIdToIndex.end()) {

				//If the element already exists, simply replace the component object
				int index = entityIdToIndex[entityId];
				data[index] = object;
			}
			else {
				//When adding a new object, keep track of the entity ids and their vector index
				int index = size;
				entityIdToIndex.emplace(entityId, index);
				indexToEntityId.emplace(index, entityId);
				if (index >= data.capacity()) {
					//If necessary, resize by always doubling the current capacity
					data.resize(size * 2);
				}
				data[index] = object;
				size++;
			}
		}

		void Remove(int entityId) {
			//Copy the last element  to the deleted position to keep the array packed
			int indexOfRemoved = entityIdToIndex[entityId];
			int indexOfLast = size - 1;
			data[indexOfRemoved] = data[indexOfLast];

			//Update the index-entity maps to point to the correct elements
			int entityOfLastElement = indexToEntityId[indexOfLast];
			entityIdToIndex[entityOfLastElement] = indexOfRemoved;
			indexToEntityId[indexOfRemoved] = entityOfLastElement;

			entityIdToIndex.erase(entityId);
			indexToEntityId.erase(indexOfLast);
			size--;
		}

		void RemoveEntityFromPool(int entityId) override {
			if (entityIdToIndex.find(entityId) != entityIdToIndex.end()) {
				Remove(entityId);
			}
		}

		T& Get(int entityId) {
			int index = entityIdToIndex[entityId];
			return static_cast<T&>(data[index]);
		}

		T& operator[](unsigned int index) {
			return data[index];
		}
};

//Registry
//The registry manages the creation and destruction of entities, add systems,
//and components.
class Registry
{
private:
	int numEntities = 0;

	//Vector of component pools, each pool contains all the data for a certain compoenent type
	//[Vector index = component id]
	//[Pool index = entity id]
	std::vector<std::shared_ptr<IPool>> componentPools;

	//Vector of component signatures per entity, saying which comp is "on" for a given entity.
	//[Vector index = entity id]
	std::vector<Signature> entityComponentSignatures;

	//Map of active systems
	//map keys = system type id
	//https://www.cplusplus.com/reference/unordered_map/unordered_map/
	std::unordered_map<std::type_index, std::shared_ptr<System>> systems;

	//Set of entities thare  need to be added or removed in the next registry update()
	std::set<Entity> entitiesToBeAdded;
	std::set<Entity> entitiesToBeKilled;

	//Entity tags (one tag name per entity)
	std::unordered_map<std::string, Entity> entityPerTag;
	std::unordered_map<int, std::string> tagPerEntity;

	//Entity groups (a set of entities per group name)
	std::unordered_map<std::string, std::set<Entity>> entitiesPerGroup;
	std::unordered_map<int, std::string> groupPerEntity;

	//List of free entities ids that were previously removed
	std::deque<int> freeIds;

public:
	Registry() {
		Logger::Log("Registry constructor called");
	}

	~Registry() {
		Logger::Log("Registry destructor called");
	}


	//The registry Update() processes the entities that are waiting to be added/destroyed.
	void Update();

	//----Entity management----
	Entity CreateEntity();
	void KillEntity(Entity entity);

	//----Tag management----
	void TagEntity(Entity entity, const std::string& tag);
	bool EntityHasTag(Entity entity, const std::string& tag) const;
	Entity GetEntityByTag(const std::string& tag) const;
	void RemoveEntityTag(Entity entity);

	//----Tag management----
	void GroupEntity(Entity entity, const std::string& group);
	bool EntityBelongsToGroup(Entity entity, const std::string& group) const;
	std::vector<Entity> GetEntitiesByGroup(const std::string& group) const;
	void RemoveEntityGroup(Entity entity);


	//----Component Manager----
	template <typename TComponent, typename ...TArgs> void AddComponent(Entity entity, TArgs&& ...args);
	//Removecomponent (Entity entity)
	template <typename TComponent> void RemoveComponent(Entity entity);
	//Hascomponent (Entity entity)
	template <typename TComponent> bool HasComponent(Entity entity) const;
	//Getcomponent
	template<typename TComponent> TComponent& GetComponent(Entity entity) const;

	//----System Manager----
	//AddSystem()
	template <typename TSystem, typename ...TArgs> void AddSystem(TArgs&& ...args);
	//RemoveSystem()
	template <typename TSystem> void RemoveSystem();
	//HasSystem()
	template <typename TSystem> bool HasSystem() const;
	//Getsystem()
	template <typename TSystem> TSystem& GetSystem() const;

	//Add & remove entities from their systems
	void AddEntityToSystems(Entity entity);
	void RemoveEntityFromSystems(Entity entity);
};

template <typename TComponent>
void System::RequireComponent()
{
	const auto componentId = Component<TComponent>::GetId();
	componentSignature.set(componentId);
}

//Need smart pointers implementation later to get rid of raw pointers
//https://stackoverflow.com/questions/106508/what-is-a-smart-pointer-and-when-should-i-use-one
//https://docs.microsoft.com/en-us/cpp/cpp/smart-pointers-modern-cpp?view=msvc-160

template <typename TSystem, typename ...TArgs>
void Registry::AddSystem(TArgs&& ...args) {
	std::shared_ptr<TSystem> newSystem = std::make_shared<TSystem>(std::forward <TArgs>(args)...);
	//Adding the system to the unordered map by inserting a pair ,a key and a value
	systems.insert(std::make_pair(std::type_index(typeid(TSystem)), newSystem));
}

//Remove system
template <typename TSystem>
void Registry::RemoveSystem() {
	auto system = systems.find(std::type_index(typeid(TSystem)));
	systems.erase(system);
}

//Has system
template <typename TSystem>
bool Registry::HasSystem() const {
	return systems.find(std::type_index(typeid(TSystem))) != systems.end();
}

//http://www.cplusplus.com/reference/unordered_map/unordered_map/find/
//Get system 
//Get the itetrator point? and use it to ask for the second to get the value and convert to a pointer??cast of the Tsystem whatever the Tsystem is 
template <typename TSystem>
TSystem& Registry::GetSystem() const {
	auto system = systems.find(std::type_index(typeid(TSystem)));
	return  *(std::static_pointer_cast<TSystem>(system->second));
}


template <typename TComponent, typename ...TArgs>
void Registry::AddComponent(Entity entity, TArgs&& ...args) 
{
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();

	if (componentId >= componentPools.size()) {
		componentPools.resize(componentId + 1, nullptr);
	}

	if (!componentPools[componentId]) {
		std::shared_ptr<Pool<TComponent>>  newComponentPool (new Pool<TComponent>());
		componentPools[componentId] = newComponentPool;
	}

	std::shared_ptr <Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);

	TComponent newComponent(std::forward<TArgs>(args)...);

	componentPool->Set(entityId, newComponent);

	entityComponentSignatures[entityId].set(componentId);

	Logger::Log("Component ID = " + std::to_string(componentId) + "was added to the entity ID " + std::to_string(entityId));
	
}
template <typename TComponent>
void Registry::RemoveComponent(Entity entity) {
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();

	//Remove the component from the component list for that entity
	std::shared_ptr <Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);
	componentPool->Remove(entityId);

	//Set this component signature for that entity to false
	entityComponentSignatures[entityId].set(componentId, false);

	Logger::Log("Component ID = " + std::to_string(componentId) + "was removed from the entity ID " + std::to_string(entityId));

}

template <typename TComponent>
bool Registry::HasComponent(Entity entity) const{
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();
	return	entityComponentSignatures[entityId].test(componentId);

}

template <typename TComponent>
TComponent& Registry::GetComponent(Entity entity) const {
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();
	auto componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);
	return componentPool->Get(entityId);
}

template <typename TComponent, typename ...TArgs>
void Entity::AddComponent(TArgs&& ... args) {
	registry->AddComponent<TComponent>(*this, std::forward<TArgs>(args)...);
}

template <typename TComponent>
void Entity::RemoveComponent() {
	registry->RemoveComponent<TComponent>(*this);
}

template <typename TComponent>
bool Entity::HasComponent() const {
	return registry->HasComponent<TComponent>(*this);
}

template <typename TComponent>
TComponent& Entity::GetComponent() const {
	return registry->GetComponent<TComponent>(*this);
}

#endif // !ECS_H

