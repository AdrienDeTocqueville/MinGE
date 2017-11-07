#include "Components/Component.h"

int Component::instances = 0;

Component::Component():
    entity(nullptr), tr(nullptr)
{
    instances++;
}

Component::~Component()
{
    instances--;
}

/// Methods (private)
void Component::onRegister() {}

void Component::onDeregister() {}
