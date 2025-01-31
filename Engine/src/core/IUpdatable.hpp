#pragma once

namespace Alvar
{
    class IUpdatable
    {
    public:
        virtual void Update(float aDeltaTime) = 0;
    };
}
