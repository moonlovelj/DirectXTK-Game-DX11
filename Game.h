//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"

#include "Terrain.h"
#include "Camera.h"
#include "SkyDome.h"
#include "Water.h"
#include "Foliage.h"

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();
    void ClearReflection();
    void ClearRefraction();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

    std::unique_ptr<DirectX::Keyboard>      m_keyboard;
    std::unique_ptr<DirectX::Mouse>         m_mouse;

    DirectX::SimpleMath::Matrix             m_world;
    DirectX::SimpleMath::Matrix             m_view;
    DirectX::SimpleMath::Matrix             m_proj;

    std::unique_ptr<DirectX::CommonStates>  m_states;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_raster;

    std::unique_ptr<Terrain>                m_terrain;
    std::unique_ptr<Camera>                 m_camera;
    std::unique_ptr<SkyDome>                m_skyDome;
    std::unique_ptr<Water>                  m_water;
    std::unique_ptr<Foliage>                m_foliage;
};