#include <gtest/gtest.h>

#include "ECS/ECS.h"

TEST(Chunk, AddressComputation)
{

	size_t chunk_size = 16384 * 4;  // 64K
	ECS::Chunk chunk(chunk_size);

	std::vector<size_t> row_sizeofs{ sizeof(int), sizeof(short), sizeof(bool), sizeof(double) };  // int, short, char, double

	size_t total_row_sizeof = sizeof(int) + sizeof(short) + sizeof(bool) + sizeof(double);
	size_t col_num = chunk_size / total_row_sizeof;

	char* address = static_cast<char*>(chunk.chunk_ptr);

	int* int_array = new (address) int[col_num];
	address += col_num * sizeof(int);

	short* short_array = new (address) short[col_num];
	address += col_num * sizeof(short);

	bool* bool_array = new (address) bool[col_num];
	address += col_num * sizeof(bool);

	double* double_array = new (address) double[col_num];
	address += col_num * sizeof(double);

	for (size_t i = 0; i < col_num; i++) {
		int_array[i] = (int)i * 2;
		short_array[i] = (short)i;
		bool_array[i] = ((i % 2) == 0);
		double_array[i] = (double)i / 3;
	}

	EXPECT_EQ(114 * 2, *(static_cast<int*>(chunk.GetAddress(0, 114, row_sizeofs, col_num))));
	EXPECT_EQ(514, *(static_cast<short*>(chunk.GetAddress(1, 514, row_sizeofs, col_num))));
	EXPECT_EQ(1919 % 2 == 0, *(static_cast<bool*>(chunk.GetAddress(2, 1919, row_sizeofs, col_num))));
	EXPECT_DOUBLE_EQ(double(810) / 3, *(static_cast<double*>(chunk.GetAddress(3, 810, row_sizeofs, col_num))));
}

struct PositionComponent
{
	PositionComponent(float x, float y) : x(x), y(y) {}
	PositionComponent() : x(0.2f), y(0.2f) {}

	float x;
	float y;
};

struct IntComponent
{
	IntComponent() : num(99) {}
	IntComponent(int i) : num(i) {}

	int num;
};

struct MixComponent
{

	MixComponent() : i(1), b(false), d(3.14), s(666) {}
	MixComponent(int i, bool b, double d, short s) : i(i), b(b), d(d), s(s) {}

	int i;
	bool b;
	double d;
	short s;
};

TEST(EntityManager, AllCases)
{

	ECS::World world;
	ECS::EntityManager& entity_mgr = world.GetEntityManager();

	// Create entities, with and without initial archetype
	ECS::Entity entity_1 = entity_mgr.CreateEntity();
	ECS::Entity entity_2 = entity_mgr.CreateEntity<PositionComponent>();
	ECS::Entity entity_3 = entity_mgr.CreateEntity<PositionComponent, IntComponent>();
	ECS::Entity entity_4 = entity_mgr.CreateEntity<IntComponent, MixComponent>();

	PositionComponent* pos_ptr;
	IntComponent* i_ptr;
	MixComponent* mix_ptr;

	pos_ptr = entity_mgr.GetEntityComponent<PositionComponent>(entity_2);
	EXPECT_FLOAT_EQ(0.2f, pos_ptr->x);
	EXPECT_FLOAT_EQ(0.2f, pos_ptr->y);

	pos_ptr = entity_mgr.GetEntityComponent<PositionComponent>(entity_3);
	EXPECT_FLOAT_EQ(0.2f, pos_ptr->x);
	EXPECT_FLOAT_EQ(0.2f, pos_ptr->y);

	i_ptr = entity_mgr.GetEntityComponent<IntComponent>(entity_3);
	EXPECT_EQ(99, i_ptr->num);

	mix_ptr = entity_mgr.GetEntityComponent<MixComponent>(entity_4);
	EXPECT_EQ(1, mix_ptr->i);
	EXPECT_EQ(false, mix_ptr->b);
	EXPECT_DOUBLE_EQ(3.14, mix_ptr->d);
	EXPECT_EQ(666, mix_ptr->s);

	// Add Components
	entity_mgr.AddEntityComponent<IntComponent>(entity_2, 20);
	i_ptr = entity_mgr.GetEntityComponent<IntComponent>(entity_2);
	EXPECT_EQ(20, i_ptr->num);

	entity_mgr.AddEntityComponent<PositionComponent>(entity_1, 0.9f, 0.8f);
	entity_mgr.AddEntityComponent<IntComponent>(entity_1, 6);
	entity_mgr.AddEntityComponent<MixComponent>(entity_1, 2, true, 3.0, 4);

	pos_ptr = entity_mgr.GetEntityComponent<PositionComponent>(entity_1);
	EXPECT_FLOAT_EQ(0.9f, pos_ptr->x);
	EXPECT_FLOAT_EQ(0.8f, pos_ptr->y);

	i_ptr = entity_mgr.GetEntityComponent<IntComponent>(entity_1);
	EXPECT_EQ(6, i_ptr->num);

	mix_ptr = entity_mgr.GetEntityComponent<MixComponent>(entity_1);
	EXPECT_EQ(2, mix_ptr->i);
	EXPECT_EQ(true, mix_ptr->b);
	EXPECT_DOUBLE_EQ(3.0, mix_ptr->d);
	EXPECT_EQ(4, mix_ptr->s);

	pos_ptr = entity_mgr.GetEntityComponent<PositionComponent>(entity_3);
	EXPECT_FLOAT_EQ(0.2f, pos_ptr->x);
	EXPECT_FLOAT_EQ(0.2f, pos_ptr->y);

	i_ptr = entity_mgr.GetEntityComponent<IntComponent>(entity_3);
	EXPECT_EQ(99, i_ptr->num);

	entity_mgr.AddEntityComponent<MixComponent>(entity_1, 3, false, 4.0, 5);
	mix_ptr = entity_mgr.GetEntityComponent<MixComponent>(entity_1);
	EXPECT_EQ(3, mix_ptr->i);
	EXPECT_EQ(false, mix_ptr->b);
	EXPECT_DOUBLE_EQ(4.0, mix_ptr->d);
	EXPECT_EQ(5, mix_ptr->s);

	// Replace existing componet value
	entity_mgr.AddEntityComponent<MixComponent>(entity_1, 4, true, 5.0, 6);
	mix_ptr = entity_mgr.GetEntityComponent<MixComponent>(entity_1);
	EXPECT_EQ(4, mix_ptr->i);
	EXPECT_EQ(true, mix_ptr->b);
	EXPECT_DOUBLE_EQ(5.0, mix_ptr->d);
	EXPECT_EQ(6, mix_ptr->s);

	// Has component type
	bool res = entity_mgr.HasComponent<PositionComponent, IntComponent>(entity_2);
	ASSERT_TRUE(res);
	ASSERT_FALSE(entity_mgr.HasComponent<MixComponent>(entity_2));

	// Remove component
	entity_mgr.RemoveEntityComponent<PositionComponent>(entity_1);
	entity_mgr.RemoveEntityComponent<MixComponent>(entity_1);
	i_ptr = entity_mgr.GetEntityComponent<IntComponent>(entity_1);
	EXPECT_EQ(6, i_ptr->num);
	ASSERT_FALSE(entity_mgr.HasComponent<PositionComponent>(entity_1));
	ASSERT_FALSE(entity_mgr.HasComponent<MixComponent>(entity_1));
	ASSERT_TRUE(entity_mgr.HasComponent<IntComponent>(entity_1));

	// Two entities become null
	entity_mgr.RemoveEntityComponent<IntComponent>(entity_1);
	entity_mgr.RemoveEntityAllComponents(entity_4);
	ASSERT_FALSE(entity_mgr.HasComponent<IntComponent>(entity_1));
	ASSERT_FALSE(entity_mgr.HasComponent<MixComponent>(entity_4));
	entity_mgr.AddEntityComponent<MixComponent>(entity_4, 6, false, 7.0, 8);
	mix_ptr = entity_mgr.GetEntityComponent<MixComponent>(entity_4);
	EXPECT_EQ(6, mix_ptr->i);
	EXPECT_EQ(false, mix_ptr->b);
	EXPECT_DOUBLE_EQ(7.0, mix_ptr->d);
	EXPECT_EQ(8, mix_ptr->s);

	// For each
	std::unordered_set<ECS::Entity> entities = entity_mgr.GetEntities();
	for (const auto& entity : entities) {
		entity_mgr.RemoveEntityAllComponents(entity);
	}

	size_t repeat_num = 100;
	for (size_t i = 0; i < repeat_num; i++) {
		entity_mgr.CreateEntity<PositionComponent, IntComponent, MixComponent>();
	}
	size_t count = 0;
	entity_mgr.AddEntityComponent<PositionComponent>(entity_1);
	entity_mgr.AddEntityComponent<IntComponent>(entity_1);
	world.ForEach<PositionComponent, IntComponent>(
		[&](const ECS::Entity*, PositionComponent*, IntComponent*) -> void {
		count++;
	});
	EXPECT_EQ(repeat_num + 1, count);

	count = 0;
	world.ForEach<IntComponent, MixComponent>(
		[&](const ECS::Entity*, IntComponent*, MixComponent*) -> void {
		count++;
	});
	EXPECT_EQ(repeat_num, count);
}



class CopyIntSystem : public ECS::System
{
public:
	CopyIntSystem() {}

	virtual void Init() override {}
	virtual void Update(double delta_time) override
	{
		world_ptr->ForEach<IntComponent>([&](const ECS::Entity*, IntComponent* i_ptr) -> void {
			i_copy = i_ptr->num;
			i_ptr->num += 10 * int(delta_time);
		});
	}

	float i_copy;
};

TEST(System, AllCases)
{
	ECS::World world;
	ECS::EntityManager& entity_mgr = world.GetEntityManager();

	ECS::Entity entity = entity_mgr.CreateEntity();
	entity_mgr.AddEntityComponent<IntComponent>(entity, 0);

	CopyIntSystem* s1 = new CopyIntSystem();
	CopyIntSystem* s2 = new CopyIntSystem();
	CopyIntSystem* s3 = new CopyIntSystem();

	world.AddSystem(s1, 0);
	world.AddSystem(s2, 6);
	world.AddSystem(s3, 4);

	world.Update(1);
	EXPECT_EQ(0, s2->i_copy);
	EXPECT_EQ(10, s3->i_copy);
	EXPECT_EQ(20, s1->i_copy);

	world.Update(2);
	EXPECT_EQ(30, s2->i_copy);
	EXPECT_EQ(50, s3->i_copy);
	EXPECT_EQ(70, s1->i_copy);
}