#include "Billard.h"

void Billard::update()
{
    if (Input::getKeyReleased(sf::Keyboard::V))
    {
        float h = -3.2f;    // hauteur du tapis
        float r = 0.6f;
        float hr = r * 0.5f;

        for (int m(0) ; m <= 2 ; m++)
            for (int x(-m); x <= m ; x++)
                sphere->clone(vec3(m*r*1.01f*2.0f + 5.0f, x*r*1.01f, h+hr), vec3(0), vec3(r));

        std::array<vec3, 6> positions
        {{
            vec3(1.5f*r*1.01f*2.0f + 5.0f, 0.5f*r*1.01f , h+hr),
            vec3(1.5f*r*1.01f*2.0f + 5.0f, 1.5f*r*1.01f , h+hr),
            vec3(1.5f*r*1.01f*2.0f + 5.0f, -0.5f*r*1.01f , h+hr),
            vec3(1.5f*r*1.01f*2.0f + 5.0f, -1.5f*r*1.01f , h+hr),

            vec3(0.5f*r*1.01f*2.0f + 5.0f, -0.5f*r*1.01f , h+hr),
            vec3(0.5f*r*1.01f*2.0f + 5.0f,  0.5f*r*1.01f , h+hr)
        }};
        for (vec3& pos: positions)
            sphere->clone(pos, vec3(0), vec3(r));
    }
}
