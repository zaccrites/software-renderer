
#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <glm/glm.hpp>


struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;

    Vertex(glm::vec3 p, glm::vec3 c) :
        position { p },
        color { c }
    {
    }

};

#endif
