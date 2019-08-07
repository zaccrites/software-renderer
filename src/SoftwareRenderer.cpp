
#include <algorithm>

#include "SoftwareRenderer.hpp"

#include <glm/gtc/matrix_transform.hpp>


// struct InternalVertex
// {
//     // glm::vec4 position;
//     // glm::vec
// };




SoftwareRenderer::SoftwareRenderer(uint32_t frameWidth, uint32_t frameHeight) :
    m_FrameWidth { frameWidth },
    m_FrameHeight { frameHeight },
    m_Framebuffer {},
    m_DepthBuffer {},
    m_ProjectionMatrix { 1.0 },
    m_ViewModelMatrix { 1.0 }
{
    m_Framebuffer.resize(m_FrameWidth * m_FrameHeight * 4);
    m_DepthBuffer.resize(m_FrameWidth * m_FrameHeight);
}


void SoftwareRenderer::Clear(uint8_t r, uint8_t g, uint8_t b)
{
    // TODO: Use flags to clear color buffer, depth buffer, etc. separately
    for (uint32_t i = 0; i < m_Framebuffer.size(); i += 4)
    {
        m_Framebuffer[i + 0] = b;
        m_Framebuffer[i + 1] = g;
        m_Framebuffer[i + 2] = r;
        m_Framebuffer[i + 3] = 0xff;
    }

    for (uint32_t i = 0; i < m_DepthBuffer.size(); i++)
    {
        m_DepthBuffer[i] = std::numeric_limits<float>::infinity();
        // m_DepthBuffer[i] = 0;
    }
}


void SoftwareRenderer::SetProjectionMatrix(const glm::mat4& value)
{
    m_ProjectionMatrix = value;
}

void SoftwareRenderer::SetViewModelMatrix(const glm::mat4& value)
{
    m_ViewModelMatrix = value;
}



// TODO: Use fixed point simulation instead of float for fragment shading?


void SoftwareRenderer::DrawTriangleList(const std::vector<Vertex>& vertices)
{
    for (size_t i = 0; i < vertices.size(); i += 3)
    {
        DrawTriangle(vertices[i + 0], vertices[i + 1], vertices[i + 2]);
    }
}


#include <iostream>

void SoftwareRenderer::DrawTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
{

    glm::mat4 transform = m_ProjectionMatrix * m_ViewModelMatrix;


    // The "vertex shader", kind of.
    auto transformVertexPosition = [this, transform](const glm::vec3& inputPosition)
    {
        glm::vec4 outputPosition = {inputPosition, 1.0};

        // Model Space -> World Space -> Camera Space -> [Clip Space] -> NDC Space -> Raster Space
        outputPosition = transform * outputPosition;

        // TODO: CLip triangle, rasterize extra triangles as needed.
        // Must be done in clip space.
        // Take the inverse of the projection matrix to find clipping planes.

        // Model Space -> World Space -> Camera Space -> Clip Space -> [NDC Space] -> Raster Space
        outputPosition.x /= outputPosition.w;
        outputPosition.y /= outputPosition.w;
        outputPosition.z /= outputPosition.w;

        // Model Space -> World Space -> Camera Space -> Clip Space -> NDC Space -> [Raster Space]
        outputPosition.x = (1.0f + outputPosition.x) * (m_FrameWidth / 2);
        outputPosition.y = (1.0f - outputPosition.y) * (m_FrameHeight / 2);

        return outputPosition;
    };

    glm::vec4 v0_position = transformVertexPosition(v0.position);
    glm::vec4 v1_position = transformVertexPosition(v1.position);
    glm::vec4 v2_position = transformVertexPosition(v2.position);

    // std::cout <<
    //     "\nv0: (" << v0_position.x <<
    //     ", " << v0_position.y <<
    //     ", " << v0_position.z <<
    //     ", " << v0_position.w <<
    //     ")" << std::endl;

    // std::cout <<
    //     "v1: (" << v1_position.x <<
    //     ", " << v1_position.y <<
    //     ", " << v1_position.z <<
    //     ", " << v1_position.w <<
    //     ")" << std::endl;

    // std::cout <<
    //     "v2: (" << v2_position.x <<
    //     ", " << v2_position.y <<
    //     ", " << v2_position.z <<
    //     ", " << v2_position.w <<
    //     ")\n" << std::endl;


    auto edgeFunction = [](const glm::vec2& p, const glm::vec2& p1, const glm::vec2& p2)
    {
        const glm::vec2 a { p2 - p1 };
        const glm::vec2 b { p - p1 };
        return a.x * b.y - a.y * b.x;
    };


    // TODO: Use a smarter algorithm than a bounding box.
    uint32_t xmin = static_cast<uint32_t>(std::round(std::min({v0_position.x, v1_position.x, v2_position.x})));
    uint32_t xmax = static_cast<uint32_t>(std::round(std::max({v0_position.x, v1_position.x, v2_position.x})));
    uint32_t ymin = static_cast<uint32_t>(std::round(std::min({v0_position.y, v1_position.y, v2_position.y})));
    uint32_t ymax = static_cast<uint32_t>(std::round(std::max({v0_position.y, v1_position.y, v2_position.y})));

    // // TODO: Remove this once clipping is implemented
    // xmin = std::max(xmin, 0U);
    // xmax = std::min(xmax, m_FrameWidth);
    // ymin = std::max(ymin, 0U);
    // ymax = std::min(ymax, m_FrameHeight);

    // ymin = 0;
    // xmin = 0;
    // ymax = m_FrameHeight;
    // xmax = m_FrameWidth;

    // std::cout << "x on " << ymin << " to " << ymax << "\n" <<
    //              "y on " << xmin << " to " << xmax << "\n" << std::endl;


    for (uint32_t y = ymin; y <= ymax; y++)
    // for (uint32_t y = 0; y < m_FrameHeight; y++)
    {
        for (uint32_t x = xmin; x <= xmax; x++)
        // for (uint32_t x = 0; x < m_FrameWidth; x++)
        {

            const float area_v0_v1_p = edgeFunction({x, y}, {v0_position.x, v0_position.y}, {v1_position.x, v1_position.y});
            const float area_v1_v2_p = edgeFunction({x, y}, {v1_position.x, v1_position.y}, {v2_position.x, v2_position.y});
            const float area_v2_v0_p = edgeFunction({x, y}, {v2_position.x, v2_position.y}, {v0_position.x, v0_position.y});

            // TODO: Test for top and left edges to prevent drawing over the same edge of adjacent triangles
            // Assumes clockwise winding for front faces:
            if (area_v0_v1_p >= 0 && area_v1_v2_p >= 0 && area_v2_v0_p >= 0)
            {
                const float totalArea = edgeFunction({v0_position.x, v0_position.y}, {v1_position.x, v1_position.y}, {v2_position.x, v2_position.y});
                // w0 not needed due to optimization (see below)
                const float w1 = area_v2_v0_p / totalArea;
                const float w2 = area_v0_v1_p / totalArea;

                auto mixBarycentric = [w1, w2](float z0, float z1, float z2)
                {
                    // Barycentric coordinates optimization:
                    // removes a multiplication operation
                    //
                    // w0 + w1 + w2 = 1
                    // w0 = 1 - w1 - w2
                    // Z = (w0 * Z0) + (w1 * Z1) + (w2 * Z2)
                    // Z = ((1 - w1 - w2) * Z0) + (w1 * Z1) + (w2 * Z2)
                    // Z = Z0 + w1(Z1 - Z0) + w2(Z2 - Z0)
                    return z0 + w1 * (z1 - z0) + w2 * (z2 - z0);
                };




                uint32_t pixelIndex = (y * m_FrameWidth + x);

                // Depth Test
                // TODO: Use 1/z instead, will need to init depth buffer
                // to 0 instead of infinity
                float lastDepth = m_DepthBuffer[pixelIndex];
                float depth = 1.0/mixBarycentric(1.0/v0_position.z, 1.0/v1_position.z, 1.0/v2_position.z);

                // std::cout << "depth = " << depth << std::endl;

                if (depth < lastDepth)
                {
                    // Update depth buffer
                    // TODO: Make optional
                    m_DepthBuffer[pixelIndex] = depth;
                }
                else
                {
                    // Discard fragment
                    continue;
                }


                // TODO: Alpha Test



                float vertexColorR = mixBarycentric(v0.color.r, v1.color.r, v2.color.r);
                float vertexColorG = mixBarycentric(v0.color.g, v1.color.g, v2.color.g);
                float vertexColorB = mixBarycentric(v0.color.b, v1.color.b, v2.color.b);


                float pixelColorR = vertexColorR;
                float pixelColorG = vertexColorG;
                float pixelColorB = vertexColorB;


                m_Framebuffer[pixelIndex * 4 + 0] = static_cast<uint8_t>(0xff * pixelColorB);
                m_Framebuffer[pixelIndex * 4 + 1] = static_cast<uint8_t>(0xff * pixelColorG);
                m_Framebuffer[pixelIndex * 4 + 2] = static_cast<uint8_t>(0xff * pixelColorR);
                m_Framebuffer[pixelIndex * 4 + 3] = 0xff;  // TODO: Alpha Blending
            }

        }
    }


}


const uint8_t* SoftwareRenderer::GetFramebufferPointer() const
{
    return &m_Framebuffer[0];
}
