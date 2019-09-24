#pragma once

#include <MinGE.h>


void test_bvh();

class MoveScript : public Script
{
	public: MoveScript(float _base, int _axis):
		base(_base), axis(_axis) { }

	private:
		void update() override;

		float base;
		int axis;
};
