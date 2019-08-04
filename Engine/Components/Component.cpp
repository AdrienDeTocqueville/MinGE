#include "Components/Component.h"

#ifdef DEBUG
int Component::instances = 0;
#endif

Component::Component():
    entity(nullptr), tr(nullptr)
{
#ifdef DEBUG
    instances++;
#endif
}

Component::~Component()
{
#ifdef DEBUG
    instances--;
#endif
}
