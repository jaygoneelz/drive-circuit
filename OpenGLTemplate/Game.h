#pragma once

#include "Common.h"
#include "GameWindow.h"
#include "Texture.h"

class CCamera;
class CSkybox;
class CShader;
class CShaderProgram;
class CPlane;
class CFreeTypeFont;
class CHighResolutionTimer;
class CSphere;
class COpenAssetImportMesh;
class CAudio;
class CDiamond;
class CTrack;
class CCatmullRom;

class Game
{
public:
    Game();
    ~Game();

    static Game& GetInstance();
    WPARAM Execute();
    void SetHinstance(HINSTANCE hinstance);
    LRESULT ProcessEvents(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

private:
    void Initialise();
    void Update();
    void Render();
    void DisplayFrameRate();
    void GameLoop();

    // Template objects
    CSkybox* m_pSkybox;
    CCamera* m_pCamera;
    vector<CShaderProgram*>* m_pShaderPrograms;
    CPlane* m_pPlanarTerrain;
    CFreeTypeFont* m_pFtFont;
    COpenAssetImportMesh* m_pBarrelMesh;
    COpenAssetImportMesh* m_pHorseMesh;
    CSphere* m_pSphere;
    CHighResolutionTimer* m_pHighResolutionTimer;
    CAudio* m_pAudio;

    // Diamond -- reused from interim coursework
    CDiamond* m_pDiamond;
    CShaderProgram* m_pDiamondShaderProgram;

    // Spline and road
    CCatmullRom* m_pSpline;
    CTrack* m_pTrack;
    CTexture                 m_detailTexture;

    // Inline VAOs for centreline and edge glow strips
    UINT                     m_centrelineVAO;
    int                      m_centrelineVerts;
    UINT                     m_edgeVAO[2];
    int                      m_edgeVerts[2];

    // Four new meshes
    COpenAssetImportMesh* m_pTreeMesh;
    COpenAssetImportMesh* m_pLampMesh;
    COpenAssetImportMesh* m_pRockMesh;
    COpenAssetImportMesh* m_pCarMesh;

    // Player state
    float m_playerT;        // arc-length position on spline [0, 1]
    float m_playerSpeed;    // advance rate in t-units per second
    float m_wheelAngle;

    // Camera: 0=free, 1=first person, 2=third person, 3=top view
    int   m_cameraMode;

    // Lighting modes
    bool  m_darkMode;
    bool  m_stormMode;

    // Gameplay
    float m_gameTime;
    float m_lapTime;
    float m_lastLapT;
    int   m_score;
    bool  m_gameOver;
    bool  m_collected[5];

    // Timing / window
    GameWindow m_gameWindow;
    HINSTANCE  m_hInstance;
    double     m_dt;
    double     m_elapsedTime;
    int        m_framesPerSecond;
    int        m_frameCount;
    bool       m_appActive;
};