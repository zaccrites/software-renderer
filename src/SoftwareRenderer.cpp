
#include <algorithm>
#include <cmath>
#include <stdio.h>

#include <glm/gtc/matrix_transform.hpp>

#include "SoftwareRenderer.hpp"


static int frameTriangleCounter = 0;


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
    }

    printf("Drew %d triangles last frame! \n", frameTriangleCounter);
    frameTriangleCounter = 0;
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


void SoftwareRenderer::DrawTriangleList(const std::vector<Vertex>& vertices)
{
    // Model Space -> World Space -> Camera Space -> [Clip Space] -> NDC Space -> Raster Space
    glm::mat4 transformMatrix = m_ProjectionMatrix * m_ViewModelMatrix;
    auto moveToClipSpace = [transformMatrix](Vertex& v) {
        v.position = transformMatrix * v.position;
    };

    enum class Direction { X, Y, Z };

    // For now, only clipping against the "right" (+X) plane.
    auto clipLineSegment = [](const Vertex& a, const Vertex& b, Direction direction, bool positive) -> Vertex {
        // a inside, b outside

        // TODO: Extract math functions for later (when giving up GLM)
        auto interpolate = [](float a, float b, float weight) {
            return a * weight + b * (1.0f - weight);
        };

        float coordinateA;
        float coordinateB;
        switch (direction)
        {
        case Direction::X:
            coordinateA = a.position.x;
            coordinateB = b.position.x;
            break;
        case Direction::Y:
            coordinateA = a.position.y;
            coordinateB = b.position.y;
            break;
        case Direction::Z:
            coordinateA = a.position.z;
            coordinateB = b.position.z;
            break;
        }

        // TODO: Invariant: 0 <= *.W
        // If it's not, what do we do?
        if (a.position.w < 0 || b.position.w < 0)
        {
            printf("WARNING: a.position.w == %02f, b.position.w == %02f \n", a.position.w, b.position.w);
        }

        // TODO: Different for Z near/far clipping?
        const float wClipA = positive ? a.position.w : -a.position.w;
        const float wClipB = positive ? b.position.w : -b.position.w;
        const float n = (wClipB - coordinateB);
        const float t = n / (n + coordinateA - wClipA);

        const float newX = interpolate(a.position.x, b.position.x, t);
        const float newY = interpolate(a.position.y, b.position.y, t);
        const float newZ = interpolate(a.position.z, b.position.z, t);
        float newW;
        switch (direction)
        {
        case Direction::X: newW = positive ? newX : -newX; break;
        case Direction::Y: newW = positive ? newY : -newY; break;
        case Direction::Z: newW = positive ? newZ : -newZ; break;
        }

        const float newR = interpolate(a.color.r, b.color.r, t);
        const float newG = interpolate(a.color.g, b.color.g, t);
        const float newB = interpolate(a.color.b, b.color.b, t);

        const float newU = interpolate(a.texcoords.x, b.texcoords.x, t);
        const float newV = interpolate(a.texcoords.y, b.texcoords.y, t);

        return {
            { newX, newY, newZ, newW },
            { newR, newG, newB },
            { newU, newV }
        };
    };

    // TODO: Avoid copying vertices? Make local VBOs to use instead?
    // This algorithm modifies the vertices, so it might be unavoidable to
    // make a copy to clip in.
    std::vector<Vertex> clipVertices;
    for (auto v : vertices)
    {
        moveToClipSpace(v);
        clipVertices.push_back(v);
    }

    std::vector<Vertex> newClipVertices;

    // TODO: Clean this up
    for (int planeNumber = 0; planeNumber < 6; planeNumber++)
    {
        // if (planeNumber > 1) break;
        for (size_t i = 0; i < clipVertices.size(); i += 3)
        {
            Vertex v0 = clipVertices[i + 0];
            Vertex v1 = clipVertices[i + 1];
            Vertex v2 = clipVertices[i + 2];

            // TODO: Move this out. This doesn't need to be a lambda.
            auto clipTriangle = [&newClipVertices, clipLineSegment, v0, v1, v2](Direction direction, bool positive) {

                bool v0_out;
                bool v1_out;
                bool v2_out;
                switch (direction)
                {
                case Direction::X:
                    v0_out = positive ? v0.position.x > v0.position.w : v0.position.x < -v0.position.w;
                    v1_out = positive ? v1.position.x > v1.position.w : v1.position.x < -v1.position.w;
                    v2_out = positive ? v2.position.x > v2.position.w : v2.position.x < -v2.position.w;
                    break;

                case Direction::Y:
                    v0_out = positive ? v0.position.y > v0.position.w : v0.position.y < -v0.position.w;
                    v1_out = positive ? v1.position.y > v1.position.w : v1.position.y < -v1.position.w;
                    v2_out = positive ? v2.position.y > v2.position.w : v2.position.y < -v2.position.w;
                    break;

                case Direction::Z:
                    v0_out = positive ? v0.position.z > v0.position.w : v0.position.z < -v0.position.w;
                    v1_out = positive ? v1.position.z > v1.position.w : v1.position.z < -v1.position.w;
                    v2_out = positive ? v2.position.z > v2.position.w : v2.position.z < -v2.position.w;
                    break;
                }

                // TODO: How to optimize this?
                // (?) https://en.wikipedia.org/wiki/Sutherland%E2%80%93Hodgman_algorithm

                // If the bit is set, then the vertex is inside the clipping plane.
                // const uint8_t pattern =
                uint8_t pattern =
                    (v0_out ? 0 : 0b100) |
                    (v1_out ? 0 : 0b010) |
                    (v2_out ? 0 : 0b001);

                switch (pattern)
                {
                    case 0b000:
                        // All outside, so skip the triangle.
                        break;

                    case 0b011:
                    {
                        // Just v0 out
                        const auto v1_prime = clipLineSegment(v1, v0, direction, positive);
                        const auto v2_prime = clipLineSegment(v2, v0, direction, positive);
                        newClipVertices.push_back(v2_prime);
                        newClipVertices.push_back(v1_prime);
                        newClipVertices.push_back(v1);
                        newClipVertices.push_back(v1);
                        newClipVertices.push_back(v2);
                        newClipVertices.push_back(v2_prime);
                        break;
                    }

                    case 0b101:
                    {
                        // Just v1 out
                        const auto v0_prime = clipLineSegment(v0, v1, direction, positive);
                        const auto v2_prime = clipLineSegment(v2, v1, direction, positive);
                        newClipVertices.push_back(v0_prime);
                        newClipVertices.push_back(v2_prime);
                        newClipVertices.push_back(v2);
                        newClipVertices.push_back(v2);
                        newClipVertices.push_back(v0);
                        newClipVertices.push_back(v0_prime);
                        break;
                    }

                    case 0b110:
                    {
                        // Just v2 out
                        const auto v0_prime = clipLineSegment(v0, v2, direction, positive);
                        const auto v1_prime = clipLineSegment(v1, v2, direction, positive);
                        newClipVertices.push_back(v1_prime);
                        newClipVertices.push_back(v0_prime);
                        newClipVertices.push_back(v0);
                        newClipVertices.push_back(v0);
                        newClipVertices.push_back(v1);
                        newClipVertices.push_back(v1_prime);
                        break;
                    }

                    case 0b001:
                    {
                        // v0 and v1 out
                        const auto v0_prime = clipLineSegment(v2, v0, direction, positive);
                        const auto v1_prime = clipLineSegment(v2, v1, direction, positive);
                        newClipVertices.push_back(v0_prime);
                        newClipVertices.push_back(v1_prime);
                        newClipVertices.push_back(v2);
                        break;
                    }

                    case 0b010:
                    {
                        // v0 and v2 out
                        const auto v0_prime = clipLineSegment(v1, v0, direction, positive);
                        const auto v2_prime = clipLineSegment(v1, v2, direction, positive);
                        newClipVertices.push_back(v2_prime);
                        newClipVertices.push_back(v0_prime);
                        newClipVertices.push_back(v1);
                        break;
                    }

                    case 0b100:
                    {
                        // v1 and v2 out
                        const auto v1_prime = clipLineSegment(v0, v1, direction, positive);
                        const auto v2_prime = clipLineSegment(v0, v2, direction, positive);
                        newClipVertices.push_back(v1_prime);
                        newClipVertices.push_back(v2_prime);
                        newClipVertices.push_back(v0);
                        break;
                    }

                    case 0b111:
                        // All inside, so just draw as-is.
                        newClipVertices.push_back(v0);
                        newClipVertices.push_back(v1);
                        newClipVertices.push_back(v2);
                        break;

                    default:
                        assert(false);
                        break;
                }
            };  // clipTriangle

            switch (planeNumber)
            {
            case 0:
                clipTriangle(Direction::X, true);
                break;
            case 1:
                clipTriangle(Direction::X, false);
                break;
            case 2:
                clipTriangle(Direction::Y, true);
                break;
            case 3:
                clipTriangle(Direction::Y, false);
                break;
            case 4:
                clipTriangle(Direction::Z, true);
                break;
            case 5:
                clipTriangle(Direction::Z, false);
                break;
            default:
                assert(false);
                break;
            }

        }

        // After clipping each triangle against a plane,
        // move the new vertices back into the original
        // list to be processed again against a different plane.
        newClipVertices.swap(clipVertices);
        newClipVertices.clear();

    }

    printf("Ready to draw %d triangles! \n", clipVertices.size() / 3);
    for (size_t i = 0; i < clipVertices.size(); i += 3)
    {
        Vertex v0 = clipVertices[i + 0];
        Vertex v1 = clipVertices[i + 1];
        Vertex v2 = clipVertices[i + 2];

        RenderTriangle(v0, v1, v2);
    }

}


// Renders a triangle, assuming that it has already been transformed
// and clipped into the view port.
// void SoftwareRenderer::RenderTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
void SoftwareRenderer::RenderTriangle(Vertex& v0, Vertex& v1, Vertex& v2)
{

    // Model Space -> World Space -> Camera Space -> Clip Space -> [NDC Space] -> Raster Space
    auto perspectiveDivide = [](Vertex& v) {
        v.position.x /= v.position.w;
        v.position.y /= v.position.w;
        v.position.z /= v.position.w;

        v.color.r /= v.position.w;
        v.color.g /= v.position.w;
        v.color.b /= v.position.w;

        v.texcoords.x /= v.position.w;
        v.texcoords.y /= v.position.w;

        // (???) For perspective correction (1/w is linear in screen space)
        // v.oneOverW = 1.0f / v.position.w;

    };

    // Model Space -> World Space -> Camera Space -> Clip Space -> NDC Space -> [Raster Space]
    auto viewportTransform = [this](Vertex& v) {
        v.position.x = (1.0f + v.position.x) * (m_FrameWidth / 2);
        v.position.y = (1.0f - v.position.y) * (m_FrameHeight / 2);
    };

    Vertex v0_copy = v0;
    Vertex v1_copy = v1;
    Vertex v2_copy = v2;

    perspectiveDivide(v0);
    perspectiveDivide(v1);
    perspectiveDivide(v2);

    viewportTransform(v0);
    viewportTransform(v1);
    viewportTransform(v2);

    auto edgeFunction = [](const glm::vec2& p, const glm::vec2& p1, const glm::vec2& p2)
    {
        const glm::vec2 a { p2 - p1 };
        const glm::vec2 b { p - p1 };
        return a.x * b.y - a.y * b.x;
    };

    const float totalArea = edgeFunction({v0.position.x, v0.position.y}, {v1.position.x, v1.position.y}, {v2.position.x, v2.position.y});
    if (totalArea <= 0)
    {
        // Cull back facing triangles
	return;
    }

    // TODO: Use a smarter algorithm than a bounding box.
    uint32_t xmin = static_cast<uint32_t>(std::round(std::min({v0.position.x, v1.position.x, v2.position.x})));
    uint32_t xmax = static_cast<uint32_t>(std::round(std::max({v0.position.x, v1.position.x, v2.position.x})));
    uint32_t ymin = static_cast<uint32_t>(std::round(std::min({v0.position.y, v1.position.y, v2.position.y})));
    uint32_t ymax = static_cast<uint32_t>(std::round(std::max({v0.position.y, v1.position.y, v2.position.y})));

    glm::vec3 debugColor;
    switch (frameTriangleCounter % 12) {
        case 0: debugColor = {1.0, 0.0, 0.0}; break;
        case 1: debugColor = {0.0, 1.0, 0.0}; break;
        case 2: debugColor = {0.0, 0.0, 1.0}; break;
        case 3: debugColor = {1.0, 1.0, 0.0}; break;
        case 4: debugColor = {1.0, 0.0, 1.0}; break;
        case 5: debugColor = {0.0, 1.0, 1.0}; break;
        case 6: debugColor = {0.5, 0.0, 0.0}; break;
        case 7: debugColor = {0.0, 0.5, 0.0}; break;
        case 8: debugColor = {0.0, 0.0, 0.5}; break;
        case 9: debugColor = {0.5, 0.5, 0.0}; break;
        case 10: debugColor = {0.5, 0.0, 0.5}; break;
        case 11: debugColor = {0.0, 0.5, 0.5}; break;
    }
    frameTriangleCounter += 1;

    for (uint32_t y = ymin; y <= ymax; y++)
    {
        for (uint32_t x = xmin; x <= xmax; x++)
        {

            const float area_v0_v1_p = edgeFunction({x, y}, {v0.position.x, v0.position.y}, {v1.position.x, v1.position.y});
            const float area_v1_v2_p = edgeFunction({x, y}, {v1.position.x, v1.position.y}, {v2.position.x, v2.position.y});
            const float area_v2_v0_p = edgeFunction({x, y}, {v2.position.x, v2.position.y}, {v0.position.x, v0.position.y});

            // TODO: Test for top and left edges to prevent drawing over the same edge of adjacent triangles
            // Assumes clockwise winding for front faces:
            if (area_v0_v1_p > 0 && area_v1_v2_p > 0 && area_v2_v0_p > 0)
            {
                // w0 not needed due to optimization (see below)
                const float w1 = area_v2_v0_p / totalArea;
                const float w2 = area_v0_v1_p / totalArea;

                auto mixBarycentric = [w1, w2](float z0, float z1, float z2)
                {
                    // Barycentric coordinates optimization:
                    // removes a multiply
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
                float depth = 1.0 / mixBarycentric(v0.oneOverW(), v1.oneOverW(), v2.oneOverW());
                // float depth = 1.0 / mixBarycentric(v0.position.w, v1.position.w, v2.position.w);
                // printf("depth = %.4f  \n", depth);

                // Even though it's not clipped yet, z is less than 0.
                // That's messing everything up here.
                // std::cout << "depth = " << depth << std::endl;

                // Sample texture, if in use
                float textureColorR = 1.0;
                float textureColorG = 1.0;
                float textureColorB = 1.0;
                float textureColorA = 1.0;
                if (m_ActiveTextureID != 0)
                {
                    auto& rTexture = GetActiveTexture();

                    // TODO: Support negative tex coords better?
                    float mixedTexCoordU = std::fmod(mixBarycentric(v0.texcoords.x, v1.texcoords.x, v2.texcoords.x) * depth, 1.0f);
                    float mixedTexCoordV = std::fmod(mixBarycentric(v0.texcoords.y, v1.texcoords.y, v2.texcoords.y) * depth, 1.0f);
                    if (mixedTexCoordU < 0)
                    {
                        mixedTexCoordU += 1.0f;
                    }
                    if (mixedTexCoordV < 0)
                    {
                        mixedTexCoordV += 1.0f;
                    }
                    // printf("mixedTexCoord(U,V) = (%.4f, %.4f) \n", mixedTexCoordU, mixedTexCoordV);

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
                if (textureColorA < 0.5)
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

                float vertexColorR = mixBarycentric(v0.color.r, v1.color.r, v2.color.r) * depth;
                float vertexColorG = mixBarycentric(v0.color.g, v1.color.g, v2.color.g) * depth;
                float vertexColorB = mixBarycentric(v0.color.b, v1.color.b, v2.color.b) * depth;

                vertexColorR = debugColor.r;
                vertexColorG = debugColor.g;
                vertexColorB = debugColor.b;

                float pixelColorR = vertexColorR;
                float pixelColorG = vertexColorG;
                float pixelColorB = vertexColorB;
                if (m_ActiveTextureID != 0)
                {
                    float percentTexture = 0.5;
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
