#include "Core/Entity.h"
#include "Core/Engine.h"
#include "IO/DataModel.h"

const Entity Entity::none;
array_list_t<Entity::entity_t> Entity::entities;

static void destroy_entity(UID32 object)
{
	Entity entity(object);
	ASSERT(entity.is_valid(), "Trying to destroy an invalid Entity handle");

	Entity::entity_t &data = Entity::entities[object.id()];
	for (system_t *system : systems)
	{
		if (auto on_destroy = system_types[system->type_index].on_destroy_entity)
			on_destroy(system->instance(), entity);
	}

	data.destroy();
	Entity::entities.remove(object.id());
}

void Entity::destroy()
{
	destroy_entity(*this);
}

Entity Entity::create(const char *name)
{
	uint32_t i = entities.add();
	Entity::entity_t &entity = entities[i];

	entity.create();
	entity.name = name ? strdup(name) : NULL;

	return Entity::get(i);
}

Entity Entity::get(const char *name)
{
	for (uint32_t i = 0; i < entities.size; i++)
	{
		Entity::entity_t &entity = entities[i];
		if (entity.alive() && entity.name != NULL)
		{
			if (name == entity.name || strcmp(name, entity.name) == 0)
				return Entity::get(i);
		}
	}
	return Entity::none;
}

static UID32 transfer(UID32 object, DataModel *model)
{
	if (!object.valid())
		object = Entity::create(NULL);
	Entity entity = Entity(object);

	Entity::entity_t &data = Entity::entities[entity.id()];
	model->transfer("name", data.name);

	for (system_t *system : systems)
	{
		const system_type_t &sys = system_types[system->type_index];
		if (sys.transfer)
		{
			if (model->begin_system(sys))
			{
				sys.transfer(system->instance(), entity, model);
				model->end_system();
			}
		}
	}

	return object;
}

const object_type_t Entity::type = []() {
	object_type_t t = INIT_ASSET_TYPE(Entity);

	t.destroy = destroy_entity;
	t.data = [](UID32 obj) { return (object_data_t*)&(Entity::entities[obj.id()]); };
	t.transfer = transfer;
	return t;
}();
