
#include <stdint.h>
#include <iostream>

#include <SDL2/SDL.h>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

#include "SoftwareRenderer.hpp"
#include "Vertex.hpp"


const uint32_t FRAME_SCALING = 1;
const uint32_t FRAME_WIDTH = 320 * FRAME_SCALING;
const uint32_t FRAME_HEIGHT = 240 * FRAME_SCALING;

const uint32_t DISPLAY_SCALING = 4;
const uint32_t DISPLAY_WIDTH = FRAME_WIDTH * DISPLAY_SCALING;
const uint32_t DISPLAY_HEIGHT = FRAME_HEIGHT * DISPLAY_SCALING;




// Make a cylinder-like mesh.
// std::vector<Vertex> MakeMesh(uint32_t sides)
std::vector<Vertex> MakeMesh()
{
    // "VBO" (TODO: Use indexing to render, rather than duplicating vertices)
    std::vector<Vertex> mesh;

    // // Make top and bottom
    // for (uint32_t i = 0; i < sides - 1; i++)
    // {
    //     // TOP


    // }

    // Create a cube
    glm::vec3 top_upLeft = {-1.0, 1.0, 1.0};
    glm::vec3 top_upRight = {1.0, 1.0, 1.0};
    glm::vec3 top_downLeft = {-1.0, 1.0, -1.0};
    glm::vec3 top_downRight = {1.0, 1.0, -1.0};
    //
    glm::vec3 bottom_upLeft = {-1.0, -1.0, 1.0};
    glm::vec3 bottom_upRight = {1.0, -1.0, 1.0};
    glm::vec3 bottom_downLeft = {-1.0, -1.0, -1.0};
    glm::vec3 bottom_downRight = {1.0, -1.0, -1.0};
    //
    glm::vec3 red     = {1.0, 0.0, 0.0};
    glm::vec3 green   = {0.0, 1.0, 0.0};
    glm::vec3 blue    = {0.0, 0.0, 1.0};
    glm::vec3 cyan    = {0.0, 1.0, 1.0};
    glm::vec3 magenta = {1.0, 0.0, 1.0};
    glm::vec3 yellow  = {1.0, 1.0, 0.0};
    //
    //
    glm::vec2 tex_upLeft = {0.0, 1.0};
    glm::vec2 tex_upRight = {1.0, 1.0};
    glm::vec2 tex_downLeft = {0.0, 0.0};
    glm::vec2 tex_downRight = {1.0, 0.0};
    //
    //
    // TOP
    mesh.push_back({top_downLeft, red, tex_downLeft});
    mesh.push_back({top_downRight, red, tex_downRight});
    mesh.push_back({top_upLeft, red, tex_upLeft});
    //
    mesh.push_back({top_downRight, red, tex_downRight});
    mesh.push_back({top_upRight, red, tex_upRight});
    mesh.push_back({top_upLeft, red, tex_upLeft});
    //
    //
    // BOTTOM
    mesh.push_back({bottom_upLeft, cyan, tex_downLeft});
    mesh.push_back({bottom_downRight, cyan, tex_downRight});
    mesh.push_back({bottom_downLeft, cyan, tex_upLeft});
    //
    mesh.push_back({bottom_upLeft, cyan, tex_downRight});
    mesh.push_back({bottom_upRight, cyan, tex_upRight});
    mesh.push_back({bottom_downRight, cyan, tex_upLeft});
    //
    //
    // RIGHT
    mesh.push_back({bottom_downRight, green, tex_downLeft});
    mesh.push_back({top_upRight, green, tex_downRight});
    mesh.push_back({top_downRight, green, tex_upLeft});
    //
    mesh.push_back({bottom_downRight, green, tex_downRight});
    mesh.push_back({bottom_upRight, green, tex_upRight});
    mesh.push_back({top_upRight, green, tex_upLeft});
    //
    //
    // LEFT
    mesh.push_back({bottom_upLeft, magenta, tex_downLeft});
    mesh.push_back({top_downLeft, magenta, tex_downRight});
    mesh.push_back({top_upLeft, magenta, tex_upLeft});
    //
    mesh.push_back({bottom_upLeft, magenta, tex_downRight});
    mesh.push_back({bottom_downLeft, magenta, tex_upRight});
    mesh.push_back({top_downLeft, magenta, tex_upLeft});
    //
    //
    // FRONT
    mesh.push_back({bottom_downRight, blue, tex_downLeft});
    mesh.push_back({top_downRight, blue, tex_downRight});
    mesh.push_back({top_downLeft, blue, tex_upLeft});
    //
    mesh.push_back({bottom_downLeft, blue, tex_downRight});
    mesh.push_back({bottom_downRight, blue, tex_upRight});
    mesh.push_back({top_downLeft, blue, tex_upLeft});
    //
    //
    // BACK
    mesh.push_back({top_upLeft, yellow, tex_downLeft});
    mesh.push_back({top_upRight, yellow, tex_downRight});
    mesh.push_back({bottom_upLeft, yellow, tex_upLeft});
    //
    mesh.push_back({top_upRight, yellow, tex_downRight});
    mesh.push_back({bottom_upRight, yellow, tex_upRight});
    mesh.push_back({bottom_upLeft, yellow, tex_upLeft});

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
    auto oliveTexture = MakeTextureFromFile(context, "/home/zac/gpu/assets/olive1.png");

    auto cube1 = MakeMesh();
    auto cube2 = MakeMesh();


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
                    if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q)
                    {
                        isRunning = false;
                    }
                    break;
                }
            }
        }


        glm::mat4 model1 {1.0};
        model1 = glm::scale(model1, glm::vec3 {3.0, 3.0, 3.0});
        model1 = glm::rotate(model1, static_cast<float>(glm::radians(45.0 * t)), glm::vec3 {0.0, 1.0, 0.0});

        glm::mat4 model2 {1.0};
        model2 = glm::rotate(model2, static_cast<float>(glm::radians(45.0 * t)), glm::vec3 {0.0, 1.0, 0.0});
        model2 = glm::translate(model2, glm::vec3 {5.0, 0.0, 0.0});
        model2 = glm::rotate(model2, static_cast<float>(glm::radians(180.0)), glm::vec3 {0.0, 1.0, 0.0});

        context.SetProjectionMatrix(glm::perspective(
            glm::radians(45.0f),
            static_cast<float>(FRAME_WIDTH) / static_cast<float>(FRAME_HEIGHT),
            5.0f,
            20.0f
        ));

        float cameraRadius = 20.0;
        float cameraAngle = 45.0;
        // cameraAngle *= std::sin(glm::radians(t * 90.0));
        glm::vec3 eye = {
            0.0,
            cameraRadius * std::sin(glm::radians(cameraAngle)),
            cameraRadius * std::cos(glm::radians(cameraAngle))
        };


        glm::vec3 target = {0.0, 0.0, 0.0};
        glm::vec3 up = {0.0, 1.0, 0.0};
        glm::mat4 view = glm::lookAt(eye, target, up);


        context.Clear(0x64, 0x95, 0xed);



        context.UseTexture(oliveTexture);
        context.SetViewModelMatrix(view * model1);
        context.DrawTriangleList(cube1);

        context.UseTexture(texture);
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
    context.DestroyTexture(oliveTexture);

    SDL_DestroyTexture(pDisplayTexture);
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_Quit();
}
