
#ifndef SOFTWARE_RENDERER_HPP
#define SOFTWARE_RENDERER_HPP


#include <stdint.h>
#include <glm/glm.hpp>

#include <vector>

#include "Vertex.hpp"



// TODO: Make abstract class above this one.
class SoftwareRenderer
{
public:

    SoftwareRenderer(uint32_t frameWidth, uint32_t frameHeight);


    // void render(uint8_t* pFramebuffer, uint32_t FRAME_WIDTH, uint32_t FRAME_HEIGHT,


    void Clear(uint8_t r, uint8_t g, uint8_t b);


    void DrawTriangleList(const std::vector<Vertex>& vertices);


    void SetProjectionMatrix(const glm::mat4& value);
    void SetViewModelMatrix(const glm::mat4& value);



    // TODO: Is this a part of the real API? Would be almost
    // impossible in hardware, but easy on any simulated version.
    const uint8_t* GetFramebufferPointer() const;


private:
    void DrawTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2);


    const uint32_t m_FrameWidth;
    const uint32_t m_FrameHeight;
    std::vector<uint8_t> m_Framebuffer;
    std::vector<float> m_DepthBuffer;

    glm::mat4 m_ProjectionMatrix;
    glm::mat4 m_ViewModelMatrix;

};




#endif
