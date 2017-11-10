#ifndef AUDIOLISTENER_H
#define AUDIOLISTENER_H

#include "Components/Component.h"
#include <SFML/Audio.hpp>

class AudioListener : public Component
{
    public:
        AudioListener();
        virtual ~AudioListener();

        /// Methods (public)
            virtual AudioListener* clone() const override;

            void update();

        /// Attributes (static)
//            static AudioListener* main;

    private:
        /// Methods (private)
//            virtual void onRegister() override;
//            virtual void onDeregister() override;
};

#endif // AUDIOLISTENER_H
