#include "pch.h"
#include "Camera.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

Camera::Camera()
{
    m_positionX = 0.0f;
    m_positionY = 0.0f;
    m_positionZ = 0.0f;

    m_rotationX = 0.0f;
    m_rotationY = 0.0f;
    m_rotationZ = 0.0f;

    m_frameTime = 0.0f;

    m_forwardSpeed = 0.0f;
    m_backwardSpeed = 0.0f;
    m_upwardSpeed = 0.0f;
    m_downwardSpeed = 0.0f;
    m_leftTurnSpeed = 0.0f;
    m_rightTurnSpeed = 0.0f;
    m_lookUpSpeed = 0.0f;
    m_lookDownSpeed = 0.0f;

    UpdateViewMatrix();
}


Camera::~Camera()
{
}

void Camera::Update(float elapsedTime)
{
    m_frameTime = elapsedTime * 1000.f;
}

void Camera::SetPosition(float positionX, float positionY, float positionZ)
{
    m_positionX = positionX;
    m_positionY = positionY;
    m_positionZ = positionZ;
    UpdateViewMatrix();
}

void Camera::SetRotation(float rotationX, float rotationY, float rotationZ)
{
    m_rotationX = rotationX;
    m_rotationY = rotationY;
    m_rotationZ = rotationZ;
    UpdateViewMatrix();
}

void Camera::MoveForward(bool keydown)
{
    if (keydown)
    {
        m_forwardSpeed += m_frameTime * 0.001f;
        if (m_forwardSpeed > (m_frameTime * 0.03f))
            m_forwardSpeed = m_frameTime * 0.03f;
    }
    else
    {
        m_forwardSpeed -= m_frameTime * 0.0007f;
        if (m_forwardSpeed < 0.0f)
            m_forwardSpeed = 0.0f;
    }

    auto radians = XMConvertToRadians(m_rotationY);
    m_positionX += sinf(radians) * m_forwardSpeed;
    m_positionZ += cosf(radians) * m_forwardSpeed;

    UpdateViewMatrix();
}

void Camera::MoveBackward(bool keydown)
{
    if (keydown)
    {
        m_backwardSpeed += m_frameTime * 0.001f;
        if (m_backwardSpeed > (m_frameTime * 0.03f))
            m_backwardSpeed = m_frameTime * 0.03f;
    }
    else
    {
        m_backwardSpeed -= m_frameTime * 0.0007f;
        if (m_backwardSpeed < 0.0f)
            m_backwardSpeed = 0.0f;
    }

    auto radians = XMConvertToRadians(m_rotationY);
    m_positionX -= sinf(radians) * m_backwardSpeed;
    m_positionZ -= cosf(radians) * m_backwardSpeed;

    UpdateViewMatrix();
}

void Camera::MoveUpward(bool keydown)
{
    if (keydown)
    {
        m_upwardSpeed += m_frameTime * 0.003f;
        if (m_upwardSpeed > (m_frameTime * 0.03f))
            m_upwardSpeed = m_frameTime * 0.03f;
    }
    else
    {
        m_upwardSpeed -= m_frameTime * 0.002f;
        if (m_upwardSpeed < 0.0f)
            m_upwardSpeed = 0.0f;
    }

    m_positionY += m_upwardSpeed;

    UpdateViewMatrix();
}

void Camera::MoveDownward(bool keydown)
{
    if (keydown)
    {
        m_downwardSpeed += m_frameTime * 0.003f;
        if (m_downwardSpeed > (m_frameTime * 0.03f))
            m_downwardSpeed = m_frameTime * 0.03f;
    }
    else
    {
        m_downwardSpeed -= m_frameTime * 0.002f;
        if (m_downwardSpeed < 0.0f)
            m_downwardSpeed = 0.0f;
    }

    m_positionY -= m_downwardSpeed;

    UpdateViewMatrix();
}

void Camera::TurnLeft(bool keydown)
{
    if (keydown)
    {
        m_leftTurnSpeed += m_frameTime * 0.01f;
        if (m_leftTurnSpeed > (m_frameTime * 0.15f))
            m_leftTurnSpeed = m_frameTime * 0.15f;
    }
    else
    {
        m_leftTurnSpeed -= m_frameTime * 0.005f;
        if (m_leftTurnSpeed < 0.0f)
            m_leftTurnSpeed = 0.0f;
    }

    m_rotationY += m_leftTurnSpeed;
    if (m_rotationY > 360.0f)
        m_rotationY -= 360.0f;

    UpdateViewMatrix();
}

void Camera::TurnRight(bool keydown)
{
    if (keydown)
    {
        m_rightTurnSpeed += m_frameTime * 0.01f;
        if (m_rightTurnSpeed > (m_frameTime * 0.15f))
            m_rightTurnSpeed = m_frameTime * 0.15f;
    }
    else
    {
        m_rightTurnSpeed -= m_frameTime * 0.005f;
        if (m_rightTurnSpeed < 0.0f)
            m_rightTurnSpeed = 0.0f;
    }

    m_rotationY -= m_rightTurnSpeed;
    if (m_rotationY < 0.0f)
        m_rotationY += 360.0f;

    UpdateViewMatrix();
}

void Camera::LookUpward(bool keydown)
{
    if (keydown)
    {
        m_lookUpSpeed += m_frameTime * 0.01f;
        if (m_lookUpSpeed > (m_frameTime * 0.15f))
            m_lookUpSpeed = m_frameTime * 0.15f;
    }
    else
    {
        m_lookUpSpeed -= m_frameTime * 0.005f;
        if (m_lookUpSpeed < 0.0f)
            m_lookUpSpeed = 0.0f;
    }

    m_rotationX -= m_lookUpSpeed;
    if (m_rotationX < -90.0f)
        m_rotationX = -90.0f;

    UpdateViewMatrix();
}

void Camera::LookDownward(bool keydown)
{
    if (keydown)
    {
        m_lookDownSpeed += m_frameTime * 0.01f;
        if (m_lookDownSpeed > (m_frameTime * 0.15f))
            m_lookDownSpeed = m_frameTime * 0.15f;
    }
    else
    {
        m_lookDownSpeed -= m_frameTime * 0.005f;
        if (m_lookDownSpeed < 0.0f)
            m_lookDownSpeed = 0.0f;
    }

    m_rotationX += m_lookDownSpeed;
    if (m_rotationX > 90.0f)
        m_rotationX = 90.0f;

    UpdateViewMatrix();
}

const DirectX::SimpleMath::Matrix& Camera::GetViewMatrix() const
{
    return m_viewMatrix;
}

void Camera::UpdateViewMatrix()
{
    Vector3 up(0.f, 1.f, 0.f);
    Vector3 position(m_positionX, m_positionY, m_positionZ);
    Vector3 lookAt(0.f, 0.f, 1.f);

    auto pitch = XMConvertToRadians(m_rotationX);
    auto yaw = XMConvertToRadians(m_rotationY);
    auto roll = XMConvertToRadians(m_rotationZ);

    auto rotationMatrix = Matrix::CreateFromYawPitchRoll(yaw, pitch, roll);
    lookAt = Vector3::Transform(lookAt, rotationMatrix);
    up = Vector3::Transform(up, rotationMatrix);
    lookAt = position + lookAt;

    m_viewMatrix = Matrix::CreateLookAt(position, lookAt, up);
}
