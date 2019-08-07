
#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <glm/glm.hpp>


struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texcoords;

    Vertex(glm::vec3 p, glm::vec3 c, glm::vec2 t) :
        position { p },
        color { c },
        texcoords { t }
    {
    }

};


#endif
