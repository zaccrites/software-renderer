
#include <stdint.h>
#include <iostream>

#include <SDL2/SDL.h>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

#include "SoftwareRenderer.hpp"
#include "Vertex.hpp"


const uint32_t FRAME_SCALING = 1;
const uint32_t FRAME_WIDTH = 640 * FRAME_SCALING;
const uint32_t FRAME_HEIGHT = 480 * FRAME_SCALING;

const uint32_t DISPLAY_SCALING = 2;
const uint32_t DISPLAY_WIDTH = FRAME_WIDTH * DISPLAY_SCALING;
const uint32_t DISPLAY_HEIGHT = FRAME_HEIGHT * DISPLAY_SCALING;




// TODO: Make a cylinder-like mesh.
// std::vector<Vertex> MakeMesh(uint32_t sides)
std::vector<Vertex> MakeMesh()
{
    // "VBO" (TODO: Use indexing to render, rather than duplicating vertices)
    std::vector<Vertex> mesh;

    auto makeSide = [&mesh](const glm::vec3& axis, float angle, const glm::vec3& color) {
        glm::mat4 transform { 1.0 };
        transform = glm::rotate(transform, angle, axis);

        glm::vec4 top_upLeft    = transform * glm::vec4 {-1.0, 1.0,  1.0, 1.0};
        glm::vec4 top_upRight   = transform * glm::vec4 { 1.0, 1.0,  1.0, 1.0};
        glm::vec4 top_downLeft  = transform * glm::vec4 {-1.0, 1.0, -1.0, 1.0};
        glm::vec4 top_downRight = transform * glm::vec4 { 1.0, 1.0, -1.0, 1.0};

        glm::vec2 tex_upLeft = {0.0, 1.0};
        glm::vec2 tex_upRight = {1.0, 1.0};
        glm::vec2 tex_downLeft = {0.0, 0.0};
        glm::vec2 tex_downRight = {1.0, 0.0};

        glm::vec3 red     = {1.0, 0.0, 0.0};
        glm::vec3 green   = {0.0, 1.0, 0.0};
        glm::vec3 blue    = {0.0, 0.0, 1.0};


        mesh.push_back({top_downLeft, color, tex_downLeft});
        mesh.push_back({top_downRight, color, tex_downRight});
        mesh.push_back({top_upLeft, color, tex_upLeft});

        mesh.push_back({top_downRight, color, tex_downRight});
        mesh.push_back({top_upRight, color, tex_upRight});
        mesh.push_back({top_upLeft, color, tex_upLeft});

    };

    glm::vec3 red     = {1.0, 0.0, 0.0};
    glm::vec3 green   = {0.0, 1.0, 0.0};
    glm::vec3 blue    = {0.0, 0.0, 1.0};
    glm::vec3 cyan    = {0.0, 1.0, 1.0};
    glm::vec3 magenta = {1.0, 0.0, 1.0};
    glm::vec3 yellow  = {1.0, 1.0, 0.0};

    glm::vec3 forward = {0.0, 0.0, 1.0};
    glm::vec3 left = {-1.0, 0.0, 0.0};
    makeSide(forward, static_cast<float>(glm::radians(0.0f)), red);  // TOP
    makeSide(forward, static_cast<float>(glm::radians(90.0f)), green);  // RIGHT
    makeSide(forward, static_cast<float>(glm::radians(180.0f)), magenta);  // BOTTOM
    makeSide(forward, static_cast<float>(glm::radians(270.0f)), cyan);  // LEFT
    makeSide(left, static_cast<float>(glm::radians(90.0f)), blue);  // BACKWARD)
    makeSide(left, static_cast<float>(glm::radians(270.0f)), yellow);  // FORWARD)

    return mesh;
}




uint32_t MakeCheckerboardTexture(SoftwareRenderer& rContext, bool hollowCenters = false)
{
    const uint32_t TEXTURE_WIDTH = 128;
    const uint32_t TEXTURE_HEIGHT = 128;
    const uint32_t SQUARE_WIDTH = 16;
    const uint32_t SQUARE_HEIGHT = 16;
    const uint32_t SQUARES_WIDE = TEXTURE_WIDTH / SQUARE_WIDTH;

    std::vector<uint8_t> data;
    for (size_t y = 0; y < TEXTURE_HEIGHT; y++)
    {
        const uint32_t squareY = y / SQUARE_HEIGHT;
        for (size_t x = 0; x < TEXTURE_WIDTH; x++)
        {
            const uint32_t squareX = x / SQUARE_WIDTH;
            const bool squareIsBlack = squareY % 2 == squareX % 2;
            const uint8_t colorValue = squareIsBlack ? 0x00 : 0xff;

            const uint32_t xInSquare = x % SQUARE_WIDTH;
            const uint32_t yInSquare = y % SQUARE_HEIGHT;
            const bool isCenter = 4 < xInSquare && xInSquare < 6 && 4 < yInSquare && yInSquare < 6;
            const uint8_t alphaValue = hollowCenters && isCenter ? 0x00 : 0xff;

            data.push_back(colorValue);  // B
            data.push_back(colorValue);  // G
            data.push_back(colorValue);  // R
            data.push_back(alphaValue);  // A
        }
    }

    uint32_t textureID = rContext.CreateTexture();
    rContext.UpdateTexture(textureID, TEXTURE_WIDTH, TEXTURE_HEIGHT, &data[0]);
    return textureID;
}


uint32_t MakeTextureFromFile(SoftwareRenderer& rContext, const char* filename)
{
    int width;
    int height;
    int comp;

    uint8_t* pRawData = stbi_load(filename, &width, &height, &comp, STBI_rgb_alpha);
    if (pRawData == nullptr)
    {
        return 0;
    }

    // TODO: assert(comp == 4)  ??

    std::vector<uint8_t> data;
    for (size_t i = 0; i < width * height * 4; i += 4)
    {
        data.push_back(pRawData[i + 2]);  // B
        data.push_back(pRawData[i + 1]);  // G
        data.push_back(pRawData[i + 0]);  // R
        data.push_back(pRawData[i + 3]);  // A
    }
    stbi_image_free(pRawData);

    uint32_t textureID = rContext.CreateTexture();
    rContext.UpdateTexture(textureID, width, height, &data[0]);
    return textureID;
}




int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;


    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        std::cerr << "Failed to init SDL" << std::endl;
        return 1;
    }

    SDL_Window* pWindow = SDL_CreateWindow(
        "GPU: Software Renderer",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        DISPLAY_WIDTH,
        DISPLAY_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (pWindow == nullptr)
    {
        std::cerr << "Failed to create SDL window" << std::endl;
        return 1;
    }

    SDL_Renderer* pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);
    if (pRenderer == nullptr)
    {
        std::cerr << "Failed to create SDL renderer" << std::endl;
        return 1;
    }

    SDL_Texture* pDisplayTexture = SDL_CreateTexture(
        pRenderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        FRAME_WIDTH,
        FRAME_HEIGHT
    );


    SoftwareRenderer context {FRAME_WIDTH, FRAME_HEIGHT};


    auto texture = MakeCheckerboardTexture(context);
    auto texture2 = MakeTextureFromFile(context, "/home/zac/gpu/assets/textures/swipe.png");
    if ( ! texture2)
    {
        std::cerr << "Failed to read texture file!" << std::endl;
        return 1;
    }

    auto cube1 = MakeMesh();
    auto cube2 = MakeMesh();



    float cameraRoll = 0.0;
    float cameraPitch = 0.0;
    float cameraYaw = 0.0;

    // float cameraX = 0.0;
    // float cameraX = 13.7;
    float cameraX = 0;
    float cameraY = 0;
    float cameraZ = 10;


    float t = 0;
    bool isRunning = true;
    while (isRunning)
    {

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                {
                    isRunning = false;
                    break;
                }

                case SDL_KEYDOWN:
                {
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                        case SDLK_q:
                            isRunning = false;
                            break;


                        case SDLK_a:
                            cameraX -= 0.1f;
                            break;
                        case SDLK_d:
                            cameraX += 0.1f;
                            break;

                        case SDLK_w:
                            cameraZ -= 0.1f;
                            break;
                        case SDLK_s:
                            cameraZ += 0.1f;
                            break;

                        case SDLK_c:
                            cameraY += 0.1f;
                            break;
                        case SDLK_z:
                            cameraY -= 0.1f;
                            break;


                        // TODO: Use mouse, and move in direction of camera movement
                        // Extract camera class
                        case SDLK_UP:
                            cameraPitch += 0.1f;
                            break;
                        case SDLK_DOWN:
                            cameraPitch -= 0.1f;
                            break;

                        case SDLK_LEFT:
                            cameraYaw += 0.1f;
                            break;
                        case SDLK_RIGHT:
                            cameraYaw -= 0.1f;
                            break;


                        default:
                            break;
                    }
                    break;
                }
            }
        }


        glm::mat4 model1 {1.0};
        // model1 = glm::scale(model1, glm::vec3 {3.0, 3.0, 3.0});
        model1 = glm::scale(model1, glm::vec3 {0.5, 0.5, 0.5});
        // model1 = glm::rotate(model1, static_cast<float>(glm::radians(45.0)), glm::vec3 {0.0, 1.0, 0.0});
        model1 = glm::rotate(model1, static_cast<float>(glm::radians(45.0 * t)), glm::vec3 {0.0, 1.0, 0.0});
        // model1 = glm::rotate(model1, static_cast<float>(glm::radians(45.0)), glm::vec3 {0.0, 1.0, 0.0});

        glm::mat4 model2 {1.0};
        model1 = glm::scale(model1, glm::vec3 {3.0, 3.0, 3.0});
        model2 = glm::rotate(model2, static_cast<float>(glm::radians(90.0 * t)), glm::vec3 {0.0, 1.0, 0.0});
        model2 = glm::translate(model2, glm::vec3 {10.0, 0.0, 0.0});
        model2 = glm::rotate(model2, static_cast<float>(glm::radians(180.0)), glm::vec3 {0.0, 1.0, 0.0});

        context.SetProjectionMatrix(glm::perspective(
            glm::radians(45.0f),
            static_cast<float>(FRAME_WIDTH) / static_cast<float>(FRAME_HEIGHT),
            5.0f,
            100.0f
        ));

        // Take the inverse of the projection matrix to find clipping planes.
        //
        // Compare bounding spheres of potential objects to draw with frustum
        // planes to do coarse pruning of objects to draw to draw.


        float cameraRadius = 30.0;
        float cameraAngle = 45.0;
        // cameraAngle *= std::sin(glm::radians(t * 90.0));
        glm::vec3 eye = {
            -cameraX,
            cameraRadius * std::sin(glm::radians(cameraAngle)),
            cameraRadius * std::cos(glm::radians(cameraAngle))
        };
        std::cout << cameraX << std::endl;




        glm::vec3 target = {-cameraX, 0.0, 0.0};
        glm::vec3 up = {0.0, 1.0, 0.0};
        // glm::mat4 view = glm::lookAt(eye, target, up);


        glm::mat4 view {1.0};
        // view = glm::rotate(view, -cameraRoll, {0.0, 0.0, 1.0});
        view = glm::rotate(view, -cameraPitch, {1.0, 0.0, 0.0});
        view = glm::rotate(view, -cameraYaw, {0.0, 1.0, 0.0});
        view = glm::translate(view, {-cameraX, -cameraY, -cameraZ});

        // view = glm::lookAt(eye, target, up);


        context.Clear(0x64, 0x95, 0xed);



        context.UseTexture(texture);
        // context.UseTexture(texture2);
        context.SetViewModelMatrix(view * model1);
        context.DrawTriangleList(cube1);

        context.UseTexture(texture2);
        context.SetViewModelMatrix(view * model2);
        context.DrawTriangleList(cube2);


        // TODO: Use the SDL_PixelFormat struct to get rid of the 4 magic number
        SDL_UpdateTexture(pDisplayTexture, NULL, context.GetFramebufferPointer(), FRAME_WIDTH * 4);
        SDL_RenderCopy(pRenderer, pDisplayTexture, NULL, NULL);
        SDL_RenderPresent(pRenderer);

        // TODO: Use std::chrono
        SDL_Delay(16);
        t += 0.016;

    }

    context.DestroyTexture(texture);
    context.DestroyTexture(texture2);

    SDL_DestroyTexture(pDisplayTexture);
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_Quit();
}
