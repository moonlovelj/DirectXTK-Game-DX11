#pragma once
class Camera
{
public:
    Camera();
    ~Camera();

    void Update(float elapsedTime);

    void SetPosition(float positionX, float positionY, float positionZ);
    void SetRotation(float rotationX, float rotationY, float rotationZ);

    void MoveForward(bool keydown);
    void MoveBackward(bool keydown);
    void MoveUpward(bool keydown);
    void MoveDownward(bool keydown);
    void TurnLeft(bool keydown);
    void TurnRight(bool keydown);
    void LookUpward(bool keydown);
    void LookDownward(bool keydown);

    const DirectX::SimpleMath::Matrix& GetViewMatrix() const;

private:
    void UpdateViewMatrix();

    float m_positionX, m_positionY, m_positionZ;
    float m_rotationX, m_rotationY, m_rotationZ;

    float m_frameTime;

    float m_forwardSpeed,   m_backwardSpeed;
    float m_upwardSpeed,    m_downwardSpeed;
    float m_leftTurnSpeed,  m_rightTurnSpeed;
    float m_lookUpSpeed,    m_lookDownSpeed;

    DirectX::SimpleMath::Matrix m_viewMatrix;
};

