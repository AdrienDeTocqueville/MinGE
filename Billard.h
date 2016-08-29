#ifndef BILLARD_H
#define BILLARD_H

#include <MinGE.h>

class Billard : public Script
{
    public:
        Billard(Entity* _sphere): sphere(_sphere) {}

        void update() override;

        Entity* sphere;
};

#endif // BILLARD_H
