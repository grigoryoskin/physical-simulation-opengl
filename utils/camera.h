#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <Eigen/Dense>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN,
    NONE
};

// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.7f;
const float ZOOM        =  45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // camera Attributes
    Eigen::Vector3f Position;
    Eigen::Vector3f Front;
    Eigen::Vector3f Up;
    Eigen::Vector3f Right;
    Eigen::Vector3f WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // constructor with vectors
    Camera(Eigen::Vector3f position = Eigen::Vector3f(0.0f, 0.0f, 0.0f), Eigen::Vector3f up = Eigen::Vector3f(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(Eigen::Vector3f(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(Eigen::Vector3f(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = Eigen::Vector3f(posX, posY, posZ);
        WorldUp = Eigen::Vector3f(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    Eigen::Matrix4f GetViewMatrix()
    {
        Eigen::Vector3f f = Front.normalized();
        Eigen::Vector3f u = Right.normalized();
        Eigen::Vector3f s = f.cross(u).normalized();
        u = s.cross(f);
        Eigen::Matrix4f mat = Eigen::Matrix4f::Zero();
        mat(0,0) = s.x();
        mat(0,1) = s.y();
        mat(0,2) = s.z();
        mat(0,3) = -s.dot(Position);
        mat(1,0) = u.x();
        mat(1,1) = u.y();
        mat(1,2) = u.z();
        mat(1,3) = -u.dot(Position);
        mat(2,0) = -f.x();
        mat(2,1) = -f.y();
        mat(2,2) = -f.z();
        mat(2,3) = f.dot(Position);
        mat.row(3) << 0,0,0,1;
        return mat;
       // return glm::lookAt(Position, Position + Front, Up);
    }

    Eigen::Matrix4f GetPerspectiveMatrix
    (
        float aspect,
        float zNear,
        float zFar
    )
    {
        float yScale = 1.0f/tan(Zoom*0.0174533/2);
        float xScale = yScale/aspect;
        Eigen::Matrix4f pmat;
        pmat << xScale, 0, 0, 0,
        0, yScale, 0, 0,
        0, 0, -(zFar+zNear)/(zFar-zNear), -1,
        0, 0, -2*zNear*zFar/(zFar-zNear), 0;
        
        return pmat.transpose();
    }
    
    
    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Up * velocity;
        if (direction == RIGHT)
            Position += Up * velocity;
        if (direction == UP)
            Position += Right * velocity;
        if (direction == DOWN)
            Position -= Right * velocity;
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += yoffset;
        Pitch -= xoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }
    
private:
    /*
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up    = glm::normalize(glm::cross(Right, Front));
    }
    */
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        
        // calculate the new Front vector
        Eigen::Vector3f front;
        front[0] = cos(Yaw*0.0174533) * cos(Pitch*0.0174533);
        front[1] = sin(Pitch*0.0174533);
        front[2] = sin(Yaw*0.0174533) * cos(Pitch*0.0174533);
        Front = front.normalized();
        // also re-calculate the Right and Up vector
        Right = Front.cross(WorldUp).normalized(); // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up    = Right.cross(Front).normalized();
        
    }
};
#endif
