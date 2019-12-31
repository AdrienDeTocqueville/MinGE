#include "Systems/GraphicEngine.h"

#include "Renderer/RenderContext.inl"
#include "Renderer/CommandKey.h"
#include "Renderer/Commands.h"

#include "Utility/Debug.h"
#include "Utility/JobSystem/JobSystem.inl"
#include "Profiler/profiler.h"

#include "Assets/Shader.h"

#include "Components/Animator.h"
#include "Components/Graphic.h"
#include "Components/Camera.h"
#include "Components/Skybox.h"
#include "Components/Light.h"

#include <cstring>
#include <algorithm>

struct create_cmd_data
{
	RenderContext *contexts;
	const View *views;
	uint32_t view_count;
};

void create_cmd(const void *_data)
{
	auto *data = static_cast<const JobSystem::ParallelFor<Graphic*, create_cmd_data>*>(_data);

	auto *ctx = data->user_data.contexts + JobSystem::worker_id();
	auto *views = data->user_data.views;
	auto view_count = data->user_data.view_count;

	auto **it = (const Graphic**)data->start;
	auto **last = (const Graphic**)data->end;
	while (it != last)
		(*it++)->render(ctx, view_count, views);
}

struct merge_cmd_data
{
	RenderContext::CommandPair *pairs;
	RenderContext *ctx;
};

void merge_cmd(const void *_data)
{
	auto *data = static_cast<const merge_cmd_data*>(_data);
	auto *pairs = data->pairs;

	const auto &pool = data->ctx->commands;
	for (size_t b(0); b < pool.block; b++)
	{
		memcpy(pairs, pool.blocks[b], pool.block_bytes);
		pairs += pool.block_bytes;
	}
	memcpy(pairs, pool.blocks[pool.block], pool.index);
	data->ctx->clear();
}

void GraphicEngine::render()
{
	MICROPROFILE_SCOPEI("SYSTEM_GRAPHIC", "render");

	// Animate skeletal meshes
	{ MICROPROFILE_SCOPEI("SYSTEM_GRAPHIC", "animate");
	for (Animator* animator: animators)
		animator->animate();
	}


	//glEnable(GL_SCISSOR_TEST);

	// TODO : find a solution for that
	{
		Light *source = GraphicEngine::get()->getLight();
		Shader::setBuiltin("lightPosition", source->getPosition());
		Shader::setBuiltin("lightColor", source->getColor());
	}

	// Lights and cameras
	uint32_t view_count = 0;
	{ MICROPROFILE_SCOPEI("SYSTEM_GRAPHIC", "views");

	for (size_t i(0); i < lights.size(); i++)
	{
		if (lights[i]->target == NULL)
			continue;
		View *view = views + view_count;
		lights[i]->update(view);

		auto *cmd = contexts[0].create<SetupView>(); cmd->view = view;
		contexts[0].add(CommandKey::encode(view_count, view->pass, 0), cmd);

		view_count++;
	}

	for (size_t i(0); i < cameras.size(); i++)
	{
		View *view = views + view_count;
		cameras[i]->update(view);

		auto *cmd = contexts[0].create<SetupView>(); cmd->view = view;
		contexts[0].add(CommandKey::encode(view_count, view->pass, 0), cmd);

		if (Skybox* sky = cameras[i]->find<Skybox>())
		{
			contexts[0].add(CommandKey::encode(view_count, RenderPass::Skybox), contexts[0].create<SetupSkybox>());
			sky->render(contexts + 0, i);
		}

		view_count++;
	}
	}

	unsigned worker_count = JobSystem::worker_count();

	// Create commands
	{ MICROPROFILE_SCOPEI("SYSTEM_GRAPHIC", "create commands");

	JobSystem::ParallelFor<Graphic*, create_cmd_data> data{
		graphics.data(), (unsigned)graphics.size(),
		contexts, views, view_count
	};

	std::atomic<int> counter(0);
	int jobs = JobSystem::parallel_for(
		create_cmd, &data, &counter
	);

	JobSystem::wait(&counter, jobs);
	}

	// Merge
	RenderContext::CommandPair *pairs;
	size_t cmd_count = 0;

	{ MICROPROFILE_SCOPEI("SYSTEM_GRAPHIC", "merge contexts");
	for (unsigned i(0); i < worker_count; i++)
		cmd_count += contexts[i].cmd_count();
	pairs = new RenderContext::CommandPair[cmd_count];

	cmd_count = 0;
	std::atomic<int> counter(0);
	for (unsigned i(0); i < worker_count; i++)
	{
		merge_cmd_data data = {pairs + cmd_count, contexts + i};
		JobSystem::run(merge_cmd, &data, &counter);
		cmd_count += contexts[i].cmd_count();
	}
	JobSystem::wait(&counter, worker_count);
	}

	// Sort
	{ MICROPROFILE_SCOPEI("SYSTEM_GRAPHIC", "sort commands");
	std::sort(pairs, pairs + cmd_count);
	}

	// Submit to backend
	{ MICROPROFILE_SCOPEI("SYSTEM_GRAPHIC", "submit commands");
	Material::bound = NULL;
	auto *last = pairs + cmd_count;
	for (auto *pair = pairs; pair < last; pair++)
		CommandPacket::submit(pair->key, pair->packet);
	}

	delete[] pairs;

#ifdef DRAWAABB
	for(Graphic* g: graphics)
		g->getAABB().prepare();

	AABB::draw();
#endif

#ifdef DEBUG
	Debug::update();
#endif

	//glDisable(GL_SCISSOR_TEST);
}
