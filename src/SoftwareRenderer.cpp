
#include <algorithm>
#include <cmath>

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
    m_Textures {},
    m_ActiveTextureID {0},
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



uint32_t SoftwareRenderer::CreateTexture()
{
    m_Textures.push_back({0, 0, {}});
    return m_Textures.size();
}

void SoftwareRenderer::UpdateTexture(uint32_t id, uint32_t width, uint32_t height, const uint8_t* pData)
{
    // TODO: Optimize this? Why bother inserting elements when all I want
    // is an empty buffer? A simple C implementation would just use malloc
    // and store the list of pointers.
    //
    auto& rTexture = m_Textures[id - 1];
    rTexture.width = width;
    rTexture.height = height;
    rTexture.data.resize(0);
    for (uint32_t i = 0; i < rTexture.width * rTexture.height * 4; i++)
    {
        rTexture.data.push_back(static_cast<float>(pData[i]) / static_cast<float>(0xff));
    }
}

void SoftwareRenderer::DestroyTexture(uint32_t id)
{
    auto& rTexture = m_Textures[id - 1];
    rTexture.width = 0;
    rTexture.height = 0;
    rTexture.data.resize(0);
}

void SoftwareRenderer::UseTexture(uint32_t id)
{
    m_ActiveTextureID = id;
}

SoftwareRenderer::Texture& SoftwareRenderer::GetActiveTexture()
{
    return m_Textures[m_ActiveTextureID - 1];
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

    // TODO: Go back and study the various pieces of this to figure
    // out EXACTLY what is going on, then reorganize the code
    // so that it fits the various pipeline stages better.

    // The "vertex shader", kind of.
    auto transformVertex = [this, transform](const Vertex& input) -> Vertex
    {
        // Model Space -> World Space -> Camera Space -> [Clip Space] -> NDC Space -> Raster Space
        glm::vec4 position = transform * glm::vec4 {input.position, 1.0};

        // TODO: CLip triangle, rasterize extra triangles as needed.
        // Must be done in clip space.
        // Take the inverse of the projection matrix to find clipping planes.

        // Model Space -> World Space -> Camera Space -> Clip Space -> [NDC Space] -> Raster Space
        position.x /= position.w;
        position.y /= position.w;
        position.z /= position.w;

        // Divide vertex attributes by Z (TODO: Should this be W ?)
        glm::vec3 color = input.color / position.z;
        glm::vec2 texcoords = input.texcoords / position.z;

        // ???
        position.z = 1.0 / position.z;

        // Model Space -> World Space -> Camera Space -> Clip Space -> NDC Space -> [Raster Space]
        position.x = (1.0f + position.x) * (m_FrameWidth / 2);
        position.y = (1.0f - position.y) * (m_FrameHeight / 2);

        return {position, color, texcoords};
    };

    Vertex v0_t = transformVertex(v0);
    Vertex v1_t = transformVertex(v1);
    Vertex v2_t = transformVertex(v2);

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
    uint32_t xmin = static_cast<uint32_t>(std::round(std::min({v0_t.position.x, v1_t.position.x, v2_t.position.x})));
    uint32_t xmax = static_cast<uint32_t>(std::round(std::max({v0_t.position.x, v1_t.position.x, v2_t.position.x})));
    uint32_t ymin = static_cast<uint32_t>(std::round(std::min({v0_t.position.y, v1_t.position.y, v2_t.position.y})));
    uint32_t ymax = static_cast<uint32_t>(std::round(std::max({v0_t.position.y, v1_t.position.y, v2_t.position.y})));

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

            const float area_v0_v1_p = edgeFunction({x, y}, {v0_t.position.x, v0_t.position.y}, {v1_t.position.x, v1_t.position.y});
            const float area_v1_v2_p = edgeFunction({x, y}, {v1_t.position.x, v1_t.position.y}, {v2_t.position.x, v2_t.position.y});
            const float area_v2_v0_p = edgeFunction({x, y}, {v2_t.position.x, v2_t.position.y}, {v0_t.position.x, v0_t.position.y});

            // TODO: Test for top and left edges to prevent drawing over the same edge of adjacent triangles
            // Assumes clockwise winding for front faces:
            if (area_v0_v1_p >= 0 && area_v1_v2_p >= 0 && area_v2_v0_p >= 0)
            {
                const float totalArea = edgeFunction({v0_t.position.x, v0_t.position.y}, {v1_t.position.x, v1_t.position.y}, {v2_t.position.x, v2_t.position.y});
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

                // Take the reciprocal for perspective correction
                float depth = 1.0 / mixBarycentric(v0_t.position.z, v1_t.position.z, v2_t.position.z);

                // std::cout << "depth = " << depth << std::endl;

                // Sample texture, if in use
                float textureColorR = 1.0;
                float textureColorG = 1.0;
                float textureColorB = 1.0;
                float textureColorA = 1.0;
                if (m_ActiveTextureID != 0)
                {
                    // TODO: Perspective correction
                    auto& rTexture = GetActiveTexture();
                    float mixedTexCoordU = std::fmod(mixBarycentric(v0_t.texcoords.x, v1_t.texcoords.x, v2_t.texcoords.x) * depth, 1.0f);
                    float mixedTexCoordV = std::fmod(mixBarycentric(v0_t.texcoords.y, v1_t.texcoords.y, v2_t.texcoords.y) * depth, 1.0f);
                    // TODO: Better filtering, mipmaps, texture repeating, clamping, etc.
                    uint32_t sampleXCoord = static_cast<uint32_t>(rTexture.width * mixedTexCoordU);
                    uint32_t sampleYCoord = static_cast<uint32_t>(rTexture.height * (1.0 - mixedTexCoordV));

                    // NOTE: When using the real 18-bit color, the texture
                    // data will (ideally) be stored
                    uint32_t sampleIndex = (sampleYCoord * rTexture.width + sampleXCoord) * 4;
                    textureColorB = rTexture.data[sampleIndex + 0];
                    textureColorG = rTexture.data[sampleIndex + 1];
                    textureColorR = rTexture.data[sampleIndex + 2];
                    textureColorA = rTexture.data[sampleIndex + 3];
                }

                // Alpha Test
                if (textureColorA < 0.01)
                {
                    continue;
                }

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


                float vertexColorR = mixBarycentric(v0_t.color.r, v1_t.color.r, v2_t.color.r) * depth;
                float vertexColorG = mixBarycentric(v0_t.color.g, v1_t.color.g, v2_t.color.g) * depth;
                float vertexColorB = mixBarycentric(v0_t.color.b, v1_t.color.b, v2_t.color.b) * depth;


                float pixelColorR = vertexColorR;
                float pixelColorG = vertexColorG;
                float pixelColorB = vertexColorB;

                if (m_ActiveTextureID != 0)
                {
                    float percentTexture = 1.0;
                    pixelColorR = (1.0 - percentTexture) * pixelColorR + percentTexture * textureColorR;
                    pixelColorG = (1.0 - percentTexture) * pixelColorG + percentTexture * textureColorG;
                    pixelColorB = (1.0 - percentTexture) * pixelColorB + percentTexture * textureColorB;
                }


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
