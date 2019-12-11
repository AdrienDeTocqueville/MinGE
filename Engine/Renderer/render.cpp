#include "Systems/GraphicEngine.h"

#include "Renderer/RenderContext.inl"
#include "Renderer/CommandKey.h"
#include "Renderer/Commands.h"

#include "Utility/Debug.h"
#include "Utility/JobSystem.h"
#include "Profiler/profiler.h"

#include "Assets/Program.h"

#include "Components/Animator.h"
#include "Components/Graphic.h"
#include "Components/Camera.h"
#include "Components/Skybox.h"
#include "Components/Light.h"

#include <cstring>
#include <algorithm>

struct create_cmd_data
{
	RenderContext *ctx;
	Graphic **first, **last;
	const View *views;
	size_t view_count;
};

void create_cmd(const void *_data)
{
	auto *data = static_cast<const create_cmd_data*>(_data);

	Graphic **it = data->first;
	Graphic **last = data->last;
	RenderContext *ctx = data->ctx;
	const View *views = data->views;
	size_t view_count = data->view_count;

	while (it != last)
		(*it++)->render(ctx, view_count, views);
}

inline unsigned div_ceil(unsigned a, unsigned b)
{
	return (a + b - 1) / b;	
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
		Program::setBuiltin("lightPosition", source->getPosition());
		Program::setBuiltin("lightColor", source->getColor());
	}

	{ MICROPROFILE_SCOPEI("SYSTEM_GRAPHIC", "cameras");
	for (size_t i(0); i < cameras.size(); i++)
	{
		cameras[i]->update(views + i);

		auto *cmd = contexts[0].create<SetupView>(); cmd->view = views + i;
		contexts[0].add(CommandKey::encode(i, views[i].pass, 0), cmd);

		if (Skybox* sky = cameras[i]->find<Skybox>())
		{
			contexts[0].add(CommandKey::encode(i, RenderPass::Skybox), contexts[0].create<SetupSkybox>());
			sky->render(contexts + 0, i);
		}
	}
	}

	unsigned worker_count = JobSystem::worker_count();

	// Create commands
	std::atomic<int> counter(0);

	{ MICROPROFILE_SCOPEI("SYSTEM_GRAPHIC", "create commands");
	unsigned i, graphic_count = graphics.size();
	unsigned load = std::max(32u, div_ceil(graphic_count, worker_count));
	create_cmd_data datas[worker_count];

	for (i = 0; i < worker_count; i++)
	{
		unsigned last = min((i + 1) * load, graphic_count);
		datas[i].ctx = contexts + i;
		datas[i].first = graphics.data() + i * load;
		datas[i].last = graphics.data() + last;
		datas[i].views = views;
		datas[i].view_count = cameras.size();
		JobSystem::run(create_cmd, datas + i, &counter);
		if (last == graphic_count)
			break;
	}
	worker_count = i + 1;
	JobSystem::wait(&counter, worker_count);
	}

	// Merge
	RenderContext::CommandPair *pairs;
	size_t cmd_count = 0;

	{ MICROPROFILE_SCOPEI("SYSTEM_GRAPHIC", "merge contexts");
	for (int i(0); i < worker_count; i++)
		cmd_count += contexts[i].cmd_count();

	pairs = new RenderContext::CommandPair[cmd_count];
	auto *dest = (uint8_t*)pairs;
	for (int i(0); i < worker_count; i++)
	{
		const auto &pool = contexts[i].commands;
		for (int b(0); b < pool.block; b++)
		{
			memcpy(dest, pool.blocks[b], pool.block_bytes);
			dest += pool.block_bytes;
		}
		memcpy(dest, pool.blocks[pool.block], pool.index);
		dest += pool.index;
		contexts[i].clear();
	}
	}
	
	// Sort
	{ MICROPROFILE_SCOPEI("SYSTEM_GRAPHIC", "sort commands");
	std::sort(pairs, pairs + cmd_count);
	}

	// Submit to backend
	{ MICROPROFILE_SCOPEI("SYSTEM_GRAPHIC", "submit commands");
	auto *count = pairs + cmd_count;
	for (auto *pair = pairs; pair < count; pair++)
		CommandPacket::submit(pair->key, pair->packet);
	}

	delete pairs;

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
