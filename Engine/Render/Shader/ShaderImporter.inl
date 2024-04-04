#pragma once

#include "Core/AssetDatabase.h"

void shader_importer(import_context_t &ctx)
{
	Shader shader = Shader::create(ctx.path);

	if (shader.valid())
		ctx.register_object(Shader::type, shader);
}

const asset_importer_t Shader::importer = []() {
	asset_importer_t i = {};

	i.import = shader_importer;
	return i;
}();
