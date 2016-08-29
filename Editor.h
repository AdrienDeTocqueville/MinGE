#ifndef EDITOR_H
#define EDITOR_H

#include <Minge.h>


class Editor : public Script
{
    public:
        void update() override
        {
            float speed = 5.0f * Component::deltaTime;

            if (Input::getKeyDown(sf::Keyboard::LControl))
            {
                if (Input::getKeyDown(sf::Keyboard::Up))
                    tr->position.z += speed;
                if (Input::getKeyDown(sf::Keyboard::Down))
                    tr->position.z -= speed;
            }
            else
            {
                if (Input::getKeyDown(sf::Keyboard::Up))
                    tr->position.x += speed;
                if (Input::getKeyDown(sf::Keyboard::Down))
                    tr->position.x -= speed;
            }

            if (Input::getKeyDown(sf::Keyboard::Left))
                tr->position.y += speed;
            if (Input::getKeyDown(sf::Keyboard::Right))
                tr->position.y -= speed;

            tr->toMatrix();
        }
};

class AABBEditor: public Script
{
    public:
        void start()
        {
            box = getComponents<Box>();
            i = box.size()-1;
        }

        void update() override
        {
            if (!box.size())
                return;

            float speed = 0.01f * Component::deltaTime * 5.0f*Input::getKeyDown(sf::Keyboard::RShift);

            if (Input::getKeyDown(sf::Keyboard::LControl))
            {
                if (Input::getKeyDown(sf::Keyboard::LShift))
                {
                    if (Input::getKeyDown(sf::Keyboard::Up))
                        box[i]->halfExtent.z += speed;
                    if (Input::getKeyDown(sf::Keyboard::Down))
                        box[i]->halfExtent.z -= speed;
                }
                else
                {
                    if (Input::getKeyDown(sf::Keyboard::Up))
                        box[i]->halfExtent.x += speed;
                    if (Input::getKeyDown(sf::Keyboard::Down))
                        box[i]->halfExtent.x -= speed;

                    if (Input::getKeyDown(sf::Keyboard::Left))
                        box[i]->halfExtent.y += speed;
                    if (Input::getKeyDown(sf::Keyboard::Right))
                        box[i]->halfExtent.y -= speed;
                }
            }
            else
            {
                if (Input::getKeyDown(sf::Keyboard::LShift))
                {
                    if (Input::getKeyDown(sf::Keyboard::Up))
                        box[i]->center.z += speed;
                    if (Input::getKeyDown(sf::Keyboard::Down))
                        box[i]->center.z -= speed;
                }
                else
                {
                    if (Input::getKeyDown(sf::Keyboard::Up))
                        box[i]->center.x += speed;
                    if (Input::getKeyDown(sf::Keyboard::Down))
                        box[i]->center.x -= speed;

                    if (Input::getKeyDown(sf::Keyboard::Left))
                        box[i]->center.y += speed;
                    if (Input::getKeyDown(sf::Keyboard::Right))
                        box[i]->center.y -= speed;
                }
            }

            if (Input::getKeyReleased(sf::Keyboard::Tab))
                if (++i == box.size()) i = 0;

            if (Input::getKeyReleased(sf::Keyboard::M))
            {
                for (unsigned j(0) ; j < box.size() ; j++)
                {
                    std::cout << "            new Box(vec3(";
                write(2.0f*box[j]->halfExtent, false);
                    std::cout << "), vec3(";
                write(box[j]->getCenter(), false);
                    std::cout << "), tapisMat)," << std::endl;
                }
            }
        }

    std::vector<Box*> box;
    unsigned i = 0;
};

#endif // EDITOR_H
