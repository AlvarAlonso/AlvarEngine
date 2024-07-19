#include <iostream>

#define VMA_IMPLEMENTATION

#include <core/logger.h>
#include <engine.hpp>

int main()
{
    CEngine* App = CEngine::Get();

    App->StartUp();

    App->Run();

    App->Shutdown();

    return 0;
}