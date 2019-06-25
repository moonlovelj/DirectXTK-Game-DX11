//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    
    m_keyboard = std::make_unique<Keyboard>();
    m_mouse = std::make_unique<Mouse>();
    m_mouse->SetWindow(window);
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.
    elapsedTime;
    auto kb = m_keyboard->GetState();
    m_camera->Update(elapsedTime);
    m_camera->TurnLeft(kb.Left);
    m_camera->TurnRight(kb.Right);
    m_camera->MoveForward(kb.Up);
    m_camera->MoveBackward(kb.Down);
    m_camera->MoveUpward(kb.W);
    m_camera->MoveDownward(kb.S);
    m_camera->LookUpward(kb.PageUp);
    m_camera->LookDownward(kb.PageDown);

    m_skyDome->Update(elapsedTime);
    m_water->Update(elapsedTime);
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    auto context = m_deviceResources->GetD3DDeviceContext();
    auto cameraPos = m_camera->GetPosition();
    auto skyTranslation = Matrix::CreateTranslation(cameraPos.x, cameraPos.y, cameraPos.z);
    auto skyReflectTranslation = Matrix::CreateTranslation(cameraPos.x, -cameraPos.y + 2.f * m_water->GetWaterPlaneHeight(), cameraPos.z);
    auto waterPlane = m_water->GetWaterPlane();
    Plane normalClipPlane{ 0.f, 1.f, 0.f, 100.f };

    ClearReflection();
    m_skyDome->Render(context, SimpleMath::operator *(m_world, skyReflectTranslation), m_camera->GetReflectionMatrix(m_water->GetWaterPlaneHeight()), m_proj);
    m_terrain->Render(context, m_world, m_camera->GetReflectionMatrix(m_water->GetWaterPlaneHeight()), m_proj, waterPlane);
    m_foliage->Render(context, m_world, m_camera->GetReflectionMatrix(m_water->GetWaterPlaneHeight()), m_proj, m_camera->GetPosition());

    ClearRefraction();
    m_terrain->Render(context, m_world, m_camera->GetViewMatrix(), m_proj, { 0.f, -1.f, 0.f, m_water->GetWaterPlaneHeight() + 0.1f });

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
   
    // TODO: Add your rendering code here.
    
    m_skyDome->Render(context, SimpleMath::operator *(m_world, skyTranslation), m_camera->GetViewMatrix(), m_proj);

    m_terrain->Render(context, m_world, m_camera->GetViewMatrix(), m_proj, normalClipPlane);

    m_water->Render(context, Matrix::CreateTranslation({128.f, 0 , 128.f}), m_camera->GetViewMatrix(), m_proj, 
        m_camera->GetReflectionMatrix(m_water->GetWaterPlaneHeight()),
        m_deviceResources->GetReflectionSRV(), m_deviceResources->GetRefractionSRV(),
        m_camera->GetPosition());
   
    m_foliage->Render(context, m_world, m_camera->GetViewMatrix(), m_proj, m_camera->GetPosition());

    ID3D11ShaderResourceView* nullSrv = nullptr;
    context->PSSetShaderResources(0, 1, &nullSrv);
    context->PSSetShaderResources(1, 1, &nullSrv);
    context->PSSetShaderResources(2, 1, &nullSrv);


    m_deviceResources->PIXEndEvent();

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}

void Game::ClearReflection()
{
    m_deviceResources->PIXBeginEvent(L"ClearReflection");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetReflectTextureRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::Black);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}

void Game::ClearRefraction()
{
    m_deviceResources->PIXBeginEvent(L"ClearRefraction");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRefractTextureRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::Black);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}

#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // TODO: Initialize device dependent objects here (independent of window size).
    device;
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto size = m_deviceResources->GetOutputSize();
    auto backBufferWidth = size.right - size.left;
    auto backBufferHeight = size.bottom - size.top;
    m_world = Matrix::Identity;
    m_view = Matrix::CreateLookAt(Vector3(-2.f, 2.f, -2.f),
        Vector3::Zero, Vector3::UnitY);
    m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f,
        float(backBufferWidth) / float(backBufferHeight), 0.1f, 1000.f);

    m_states = std::make_unique<CommonStates>(device);

    CD3D11_RASTERIZER_DESC rastDesc(D3D11_FILL_SOLID, D3D11_CULL_NONE, FALSE,
        D3D11_DEFAULT_DEPTH_BIAS, D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
        D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE, FALSE, TRUE);
    DX::ThrowIfFailed(device->CreateRasterizerState(&rastDesc,
        m_raster.ReleaseAndGetAddressOf()));

    m_skyDome = std::make_unique<SkyDome>(device, context);
    m_terrain = std::make_unique<Terrain>(device, context);
    m_camera = std::make_unique<Camera>();
    m_camera->SetPosition(50.f, 10.f, -4.f);
    m_water = std::make_unique<Water>(device, context, 3.75f, 128.f);
    m_foliage = std::make_unique<Foliage>(device, context);
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
    m_terrain.reset();
    m_camera.reset();
    m_states.reset();
    m_raster.Reset();
    m_skyDome.reset();
    m_water.reset();
    m_foliage.reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
