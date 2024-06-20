#include <iostream>

#define VMA_IMPLEMENTATION

#include <core/logger.h>
#include <engine.hpp>

int main()
{
    Engine* App = Engine::Get();

    App->StartUp();

    App->Run();

    App->Shutdown();

    return 0;
}