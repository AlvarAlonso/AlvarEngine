#include <iostream>

#define VMA_IMPLEMENTATION

#include <core/logger.h>
#include <engine.hpp>

int main()
{
    Alvar::CEngine* App = Alvar::CEngine::Get();

    App->StartUp();

    App->Run();

    App->Shutdown();

    return 0;
}