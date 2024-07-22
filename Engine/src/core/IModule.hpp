#pragma once

/**
 * @brief 
 * Interface for module classes.
 */
class IModule
{
public:
    /**
     * @brief Initializes the module, settings all the necessary variables. A module should not be used before calling Initialize().
     * @return Returns true if initialized correctly.
     */
    virtual bool Initialize() = 0;

    /**
     * @brief Updates the module with the engine framerate.
     */
    virtual void Update() = 0;

    /**
     * @brief Shutdowns the module, cleaning up all resources. Before being used again, Initialize() should be called.
     * @return Returns true if shutdown correctly.
     */
    virtual bool Shutdown() = 0;
};