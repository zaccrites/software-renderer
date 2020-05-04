
#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <glm/glm.hpp>


struct Vertex
{
    glm::vec4 position;
    glm::vec3 color;
    glm::vec2 texcoords;

    // TODO: This is only on the internal vertex, and should be
    // cached instead of computed every time.
    float oneOverW()
    {
        return 1.0 / position.w;
    }

    Vertex(glm::vec3 p, glm::vec3 c, glm::vec2 t) :
        position { p, 1.0 },
        color { c },
        texcoords { t }
    {
    }

    // TODO: This class is ugly. Fix it.
    // There are user-facing vertexes, and ones used inside the driver.
    // Maybe structures-of-arrays of vertex attributes make more sense than
    // an arrays of Vertex structures anyway.
    Vertex(glm::vec4 p, glm::vec3 c, glm::vec2 t) :
        position { p },
        color { c },
        texcoords { t }
    {
    }

};


#endif
