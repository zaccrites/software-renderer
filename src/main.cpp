
#include <stdint.h>
#include <iostream>

#include <SDL2/SDL.h>
#include <glm/gtc/matrix_transform.hpp>

#include "SoftwareRenderer.hpp"
#include "Vertex.hpp"


const uint32_t FRAME_SCALING = 1;
const uint32_t FRAME_WIDTH = 320 * FRAME_SCALING;
const uint32_t FRAME_HEIGHT = 240 * FRAME_SCALING;

const uint32_t DISPLAY_SCALING = 4;
const uint32_t DISPLAY_WIDTH = FRAME_WIDTH * DISPLAY_SCALING;
const uint32_t DISPLAY_HEIGHT = FRAME_HEIGHT * DISPLAY_SCALING;




std::vector<Vertex> make_cube()
{
    // "VBO"
    std::vector<Vertex> mesh;

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
    // TOP
    mesh.push_back({top_downLeft, red});
    mesh.push_back({top_downRight, red});
    mesh.push_back({top_upLeft, red});
    //
    mesh.push_back({top_downRight, red});
    mesh.push_back({top_upRight, red});
    mesh.push_back({top_upLeft, red});
    //
    //
    // BOTTOM
    mesh.push_back({bottom_upLeft, cyan});
    mesh.push_back({bottom_downRight, cyan});
    mesh.push_back({bottom_downLeft, cyan});
    //
    mesh.push_back({bottom_upLeft, cyan});
    mesh.push_back({bottom_upRight, cyan});
    mesh.push_back({bottom_downRight, cyan});
    //
    //
    // RIGHT
    mesh.push_back({bottom_downRight, green});
    mesh.push_back({top_upRight, green});
    mesh.push_back({top_downRight, green});
    //
    mesh.push_back({bottom_downRight, green});
    mesh.push_back({bottom_upRight, green});
    mesh.push_back({top_upRight, green});
    //
    //
    // LEFT
    mesh.push_back({bottom_upLeft, magenta});
    mesh.push_back({top_downLeft, magenta});
    mesh.push_back({top_upLeft, magenta});
    //
    mesh.push_back({bottom_upLeft, magenta});
    mesh.push_back({bottom_downLeft, magenta});
    mesh.push_back({top_downLeft, magenta});
    //
    //
    // FRONT
    mesh.push_back({bottom_downRight, blue});
    mesh.push_back({top_downRight, blue});
    mesh.push_back({top_downLeft, blue});
    //
    mesh.push_back({bottom_downLeft, blue});
    mesh.push_back({bottom_downRight, blue});
    mesh.push_back({top_downLeft, blue});
    //
    //
    // BACK
    mesh.push_back({top_upLeft, yellow});
    mesh.push_back({top_upRight, yellow});
    mesh.push_back({bottom_upLeft, yellow});
    //
    mesh.push_back({top_upRight, yellow});
    mesh.push_back({bottom_upRight, yellow});
    mesh.push_back({bottom_upLeft, yellow});

    return mesh;
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


    auto cube1 = make_cube();
    auto cube2 = make_cube();


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
        model1 = glm::rotate(model1, static_cast<float>(glm::radians(180.0 * t)), glm::vec3 {0.0, 1.0, 0.0});

        glm::mat4 model2 {1.0};
        model2 = glm::rotate(model2, static_cast<float>(glm::radians(180.0 * t)), glm::vec3 {0.0, 1.0, 0.0});
        model2 = glm::translate(model2, glm::vec3 {5.0, 0.0, 0.0});
        model2 = glm::rotate(model2, static_cast<float>(glm::radians(180.0)), glm::vec3 {0.0, 1.0, 0.0});

        context.SetProjectionMatrix(glm::perspective(
            glm::radians(45.0f),
            static_cast<float>(FRAME_WIDTH) / static_cast<float>(FRAME_HEIGHT),
            5.0f,
            20.0f
        ));

        float cameraRadius = 32.0;
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


        context.SetViewModelMatrix(view * model1);
        context.DrawTriangleList(cube1);

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

    SDL_DestroyTexture(pDisplayTexture);
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_Quit();
}
