#ifndef AUDIOLISTENER_H
#define AUDIOLISTENER_H

#include "Components/Component.h"

class AudioListener : public Component
{
    public:
        AudioListener();
        virtual ~AudioListener();

        /// Methods (public)
            Component* clone() override;

            void registerComponent() override;
            void deregisterComponent() override;

            void update();

        static AudioListener* main;
};

#endif // AUDIOLISTENER_H
