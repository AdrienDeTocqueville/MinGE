#pragma once

class Collider;
struct Manifold;

Manifold* detect_default(Collider* _a, Collider* _b);
Manifold* detect_SphereSphere(Collider* _a, Collider* _b);
