#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include "mathlib.h"

#define CAMERA_MOVE_SENS 50
#define CAMERA_LOOK_SENS 0.15

class Camera
{
public:
    /**
     * Constructor.
     */
    Camera();

    /**
     * Returns the current position of the camera.
     */
    Vector3D GetPosition();

    /**
     * Sets the position of the camera.
     */
    void SetPosition(float x, float y, float z);

    /**
     * Sets the position of the camera.
     */
    void SetPosition(Vector3D v);

    /**
     * Returns a vector containing the current view angles.
     */
    Vector2D GetViewAngles();

    /**
     * Returns a vector pointing representing the current view direction.
     */
    Vector3D GetViewVector();

    void SetViewAngles(float pitch, float yaw); // Sets the current view angles
    void SetPosition(Vector2D v); // Sets the current view angles

    float GetMoveSens();               // Returns the current movement sensitivity
    void SetMoveSens(float moveSens); // Sets the current movement sensitivity

    void SetCaptureMouse(bool captureMouse);

    void Update(double interval); // Updates the position of the coord system according to user input and passed time since the last frame
    void Look();

private:
    /** The current position */
    Vector3D position;

    bool captureMouse;

    float pitch; // Angle of up down rotation
    float yaw; // Angle of side to side rotation

    float moveSens; // Movement sensitivity
    float lookSens; // Look sensitivity

    Vector2D mouseOrigin; // Pixel coords of the origin of the mouse
};

#endif // CAMERA_H_INCLUDED
