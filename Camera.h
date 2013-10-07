#pragma once

#include "mathlib.h"

#define CAMERA_MOVE_SENS 5.0f
#define CAMERA_LOOK_SENS 0.15f

/**
 * Camera singleton.
 */
class Camera
{
public:
    /**
     * Gets the instance of the singleton.
     */
    static Camera& GetInstance();

    /**
     * Returns the current position of the camera.
     */
    Vector3F GetPosition();

    /**
     * Sets the position of the camera.
     */
    void SetPosition(float x, float y, float z);

    /**
     * Sets the position of the camera.
     */
    void SetPosition(Vector3F v);

    /**
     * Returns a vector containing the current view angles.
     */
    Vector2F GetViewAngles();

    /**
     * Returns a vector pointing representing the current view direction.
     */
    Vector3F GetViewVector();

    /**
     * Sets the current view angles.
     */
    void SetViewAngles(float pitch, float yaw);

    /**
     * Sets the current view angles.
     */
    void SetPosition(Vector2F v);

    /**
     * Returns the current movement sensitivity.
     */
    float GetMoveSens();

    /**
     * Sets the current movement sensitivity.
     */
    void SetMoveSens(float moveSens);

    /**
     * Sets whether or not the mouse should be captured to determine the current view angles.
     */
    void SetCaptureMouse(bool captureMouse);

    /**
     * Updates the position of the coord system according to user input and time passed since the last update.
     *
     * @param interval Time passed since the last update in seconds.
     */
    void Update(double interval);

    /**
     * Sets the modelview matrix according to the current position and view angles.
     */
    void Look();

private:
    /**
     * Private constructor.
     */
    Camera();

    Camera(Camera const&); // No implementation, prevent copying singleton
    void operator=(Camera const&); // No implementation, prevent copying singleton

    /** The current position */
    Vector3F position;

    /** If true, the mouse movement is captured to determine the view angles. */
    bool captureMouse;

    /** Angle of up down rotation. */
    float pitch;

    /** Angle of side to side rotation. */
    float yaw;

    /** Movement sensitivity */
    float moveSens;

    /** Look sensitivity */
    float lookSens;

    /** Pixel coords of the origin of the mouse. */
    Vector2I mouseOrigin;
};
