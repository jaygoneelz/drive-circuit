/*
OpenGL Template for INM376 / IN3005
City University London, School of Mathematics, Computer Science and Engineering
Source code drawn from a number of sources and examples, including contributions from
 - Ben Humphrey (gametutorials.com), Michal Bubner (mbsoftworks.sk), Christophe Riccio (glm.g-truc.net)
 - Christy Quinn, Sam Kellett and others

 For educational use by Department of Computer Science, City University London UK.

 This template contains a skybox, simple terrain, camera, lighting, shaders, texturing

 Potential ways to modify the code:  Add new geometry types, shaders, change the terrain, load new meshes, change the lighting,
 different camera controls, different shaders, etc.

 Template version 5.0a 29/01/2017
 Dr Greg Slabaugh (gregory.slabaugh.1@city.ac.uk)

 version 6.0a 29/01/2019
 Dr Eddie Edwards (Philip.Edwards@city.ac.uk)

 version 6.1a 13/02/2022 - Sorted out Release mode and a few small compiler warnings
 Dr Eddie Edwards (Philip.Edwards@city.ac.uk)

*/

#include "Game.h"
#include "HighResolutionTimer.h"
#include "GameWindow.h"
#include "Camera.h"
#include "Skybox.h"
#include "Plane.h"
#include "Shaders.h"
#include "FreeTypeFont.h"
#include "Sphere.h"
#include "MatrixStack.h"
#include "OpenAssetImportMesh.h"
#include "Audio.h"
#include "Diamond.h"
#include "Track.h"
#include "CatmullRom.h"
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

Game::Game()
{
    m_pSkybox = NULL;
    m_pCamera = NULL;
    m_pShaderPrograms = NULL;
    m_pPlanarTerrain = NULL;
    m_pFtFont = NULL;
    m_pBarrelMesh = NULL;
    m_pHorseMesh = NULL;
    m_pSphere = NULL;
    m_pHighResolutionTimer = NULL;
    m_pAudio = NULL;
    m_pDiamond = NULL;
    m_pDiamondShaderProgram = NULL;
    m_pSpline = NULL;
    m_pTrack = NULL;
    m_pTreeMesh = NULL;
    m_pLampMesh = NULL;
    m_pRockMesh = NULL;
    m_pCarMesh = NULL;

    m_playerT = 0.0f;
    m_playerSpeed = 0.0f;
    m_wheelAngle = 0.0f;
    m_cameraMode = 0;
    m_darkMode = false;
    m_stormMode = false;
    m_gameTime = 0.0f;
    m_score = 0;
    m_lapTime = 0.0f;
    m_gameOver = false;
    m_lastLapT = 0.0f;

    m_centrelineVAO = 0;
    m_centrelineVerts = 0;
    m_edgeVAO[0] = 0;
    m_edgeVAO[1] = 0;
    m_edgeVerts[0] = 0;
    m_edgeVerts[1] = 0;

    for (int i = 0; i < 5; i++) m_collected[i] = false;

    m_dt = 0.0;
    m_framesPerSecond = 0;
    m_frameCount = 0;
    m_elapsedTime = 0.0;
    m_appActive = false;
    m_hInstance = NULL;
}

Game::~Game()
{
    delete m_pCamera;
    delete m_pSkybox;
    delete m_pPlanarTerrain;
    delete m_pFtFont;
    delete m_pBarrelMesh;
    delete m_pHorseMesh;
    delete m_pSphere;
    delete m_pAudio;
    delete m_pDiamond;
    delete m_pDiamondShaderProgram;
    delete m_pSpline;
    delete m_pTrack;
    delete m_pTreeMesh;
    delete m_pLampMesh;
    delete m_pRockMesh;
    delete m_pCarMesh;

    if (m_pShaderPrograms) {
        for (unsigned int i = 0; i < m_pShaderPrograms->size(); i++)
            delete (*m_pShaderPrograms)[i];
        delete m_pShaderPrograms;
    }
    delete m_pHighResolutionTimer;
}

void Game::Initialise()
{
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glClearDepth(1.0f);

    m_pCamera = new CCamera;
    m_pSkybox = new CSkybox;
    m_pShaderPrograms = new vector<CShaderProgram*>;
    m_pPlanarTerrain = new CPlane;
    m_pFtFont = new CFreeTypeFont;
    m_pBarrelMesh = new COpenAssetImportMesh;
    m_pHorseMesh = new COpenAssetImportMesh;
    m_pSphere = new CSphere;
    m_pAudio = new CAudio;
    m_pDiamond = new CDiamond;
    m_pDiamondShaderProgram = new CShaderProgram;
    m_pSpline = new CCatmullRom;
    m_pTrack = new CTrack;
    m_pTreeMesh = new COpenAssetImportMesh;
    m_pLampMesh = new COpenAssetImportMesh;
    m_pRockMesh = new COpenAssetImportMesh;
    m_pCarMesh = new COpenAssetImportMesh;

    RECT dimensions = m_gameWindow.GetDimensions();
    int width = dimensions.right - dimensions.left;
    int height = dimensions.bottom - dimensions.top;
    m_pCamera->SetOrthographicProjectionMatrix(width, height);
    m_pCamera->SetPerspectiveProjectionMatrix(45.0f, (float)width / (float)height, 0.5f, 5000.0f);

    // Load and compile shaders
    vector<CShader> shaders;
    vector<string>  shaderFiles = {
        "mainShader.vert", "mainShader.frag",
        "textShader.vert", "textShader.frag"
    };

    for (int i = 0; i < (int)shaderFiles.size(); i++) {
        string ext = shaderFiles[i].substr(shaderFiles[i].size() - 4, 4);
        int    type = (ext == "vert") ? GL_VERTEX_SHADER
            : (ext == "frag") ? GL_FRAGMENT_SHADER
            : (ext == "geom") ? GL_GEOMETRY_SHADER
            : GL_VERTEX_SHADER;
        CShader s;
        s.LoadShader("resources\\shaders\\" + shaderFiles[i], type);
        shaders.push_back(s);
    }

    CShaderProgram* pMainProgram = new CShaderProgram;
    pMainProgram->CreateProgram();
    pMainProgram->AddShaderToProgram(&shaders[0]);
    pMainProgram->AddShaderToProgram(&shaders[1]);
    pMainProgram->LinkProgram();
    m_pShaderPrograms->push_back(pMainProgram);

    CShaderProgram* pFontProgram = new CShaderProgram;
    pFontProgram->CreateProgram();
    pFontProgram->AddShaderToProgram(&shaders[2]);
    pFontProgram->AddShaderToProgram(&shaders[3]);
    pFontProgram->LinkProgram();
    m_pShaderPrograms->push_back(pFontProgram);

    {
        CShader dv, df;
        dv.LoadShader("resources\\shaders\\diamond.vert", GL_VERTEX_SHADER);
        df.LoadShader("resources\\shaders\\diamond.frag", GL_FRAGMENT_SHADER);
        m_pDiamondShaderProgram->CreateProgram();
        m_pDiamondShaderProgram->AddShaderToProgram(&dv);
        m_pDiamondShaderProgram->AddShaderToProgram(&df);
        m_pDiamondShaderProgram->LinkProgram();
    }

    m_pSkybox->Create(3500.0f);
    m_pPlanarTerrain->Create("resources\\textures\\", "grassfloor01.jpg", 2000.0f, 2000.0f, 50.0f);
    m_pFtFont->LoadSystemFont("arial.ttf", 32);
    m_pFtFont->SetShaderProgram(pFontProgram);

    m_pBarrelMesh->Load("resources\\models\\Barrel\\Barrel02.obj");
    m_pHorseMesh->Load("resources\\models\\Horse\\Horse2.obj");
    m_pSphere->Create("resources\\textures\\", "dirtpile01.jpg", 25, 25);

    m_pTreeMesh->Load("resources\\models\\Tree\\Tree.obj");
    m_pLampMesh->Load("resources\\models\\Statue\\statue.obj");
    m_pRockMesh->Load("resources\\models\\Stone\\Stone.obj");
    m_pCarMesh->Load("resources\\models\\Car\\Car.obj");

    m_pDiamond->Create(
        "resources\\textures\\Tile41a.jpg",
        "resources\\textures\\Tile41a.jpg"
    );

    // Detail texture on unit 1 -- blended with road base in fragment shader (multi-texturing)
    m_detailTexture.Load("resources\\textures\\Tile41a.jpg");
    m_detailTexture.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    m_detailTexture.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_detailTexture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    m_detailTexture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

    // F1-style circuit -- 16 control points forming hairpins, S-curves, chicane
    m_pSpline->AddControlPoint(glm::vec3(0.0f, 0.0f, 400.0f)); // start/finish
    m_pSpline->AddControlPoint(glm::vec3(150.0f, 0.0f, 380.0f));
    m_pSpline->AddControlPoint(glm::vec3(350.0f, 0.0f, 300.0f));
    m_pSpline->AddControlPoint(glm::vec3(450.0f, 0.0f, 100.0f)); // hairpin entry
    m_pSpline->AddControlPoint(glm::vec3(420.0f, 0.0f, -100.0f)); // hairpin exit
    m_pSpline->AddControlPoint(glm::vec3(300.0f, 0.0f, -200.0f)); // S-curve
    m_pSpline->AddControlPoint(glm::vec3(100.0f, 0.0f, -150.0f));
    m_pSpline->AddControlPoint(glm::vec3(0.0f, 0.0f, -350.0f));
    m_pSpline->AddControlPoint(glm::vec3(-150.0f, 0.0f, -400.0f));
    m_pSpline->AddControlPoint(glm::vec3(-350.0f, 0.0f, -300.0f));
    m_pSpline->AddControlPoint(glm::vec3(-450.0f, 0.0f, -100.0f)); // hairpin
    m_pSpline->AddControlPoint(glm::vec3(-400.0f, 0.0f, 100.0f));
    m_pSpline->AddControlPoint(glm::vec3(-300.0f, 0.0f, 250.0f)); // chicane
    m_pSpline->AddControlPoint(glm::vec3(-150.0f, 0.0f, 200.0f));
    m_pSpline->AddControlPoint(glm::vec3(-50.0f, 0.0f, 300.0f));
    m_pSpline->AddControlPoint(glm::vec3(-20.0f, 0.0f, 380.0f));
    m_pSpline->CreatePath(100); // 100 samples/segment = 1600 centreline points total

    m_pTrack->Create(m_pSpline, "resources\\textures\\dirtpile01.jpg", 18.0f, 15.0f);

    // Centreline strip and edge glow strips -- built as inline VAOs
    // CVert declared at function scope so both blocks can use it
    struct CVert { glm::vec3 pos; glm::vec2 uv; glm::vec3 normal; };

    {
        std::vector<CVert> verts;
        int n = m_pSpline->PathSize();
        for (int i = 0; i <= n; i++) {
            float t = (float)i / (float)n;
            glm::vec3 pos = m_pSpline->GetPosition(t);
            glm::vec3 T, N, B;
            m_pSpline->GetTNB(t, T, N, B);
            glm::vec3 Nf = glm::normalize(glm::vec3(N.x, 0.0f, N.z));
            float u = t * 80.0f;
            CVert lv, rv;
            lv.pos = glm::vec3(pos.x + 1.5f * Nf.x, 0.25f, pos.z + 1.5f * Nf.z);
            lv.uv = glm::vec2(0.0f, u);  lv.normal = glm::vec3(0, 1, 0);
            rv.pos = glm::vec3(pos.x - 1.5f * Nf.x, 0.25f, pos.z - 1.5f * Nf.z);
            rv.uv = glm::vec2(1.0f, u);  rv.normal = glm::vec3(0, 1, 0);
            verts.push_back(lv);
            verts.push_back(rv);
        }
        m_centrelineVerts = (int)verts.size();
        glGenVertexArrays(1, &m_centrelineVAO);
        glBindVertexArray(m_centrelineVAO);
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(CVert), verts.data(), GL_STATIC_DRAW);
        GLsizei stride = sizeof(CVert);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
        glBindVertexArray(0);
    }

    for (int side = 0; side < 2; side++) {
        std::vector<CVert> ev;
        int n = m_pSpline->PathSize();
        float offset = (side == 0) ? 24.0f : -24.0f;
        for (int i = 0; i <= n; i++) {
            float t = (float)i / (float)n;
            glm::vec3 pos = m_pSpline->GetPosition(t);
            glm::vec3 T, N, B;
            m_pSpline->GetTNB(t, T, N, B);
            glm::vec3 Nf = glm::normalize(glm::vec3(N.x, 0.0f, N.z));
            float u = t * 80.0f;
            CVert lv, rv;
            lv.pos = glm::vec3(pos.x + (offset + 1.0f) * Nf.x, 0.22f, pos.z + (offset + 1.0f) * Nf.z);
            lv.uv = glm::vec2(0.0f, u);  lv.normal = glm::vec3(0, 1, 0);
            rv.pos = glm::vec3(pos.x + (offset - 1.0f) * Nf.x, 0.22f, pos.z + (offset - 1.0f) * Nf.z);
            rv.uv = glm::vec2(1.0f, u);  rv.normal = glm::vec3(0, 1, 0);
            ev.push_back(lv);
            ev.push_back(rv);
        }
        m_edgeVerts[side] = (int)ev.size();
        glGenVertexArrays(1, &m_edgeVAO[side]);
        glBindVertexArray(m_edgeVAO[side]);
        GLuint evbo;
        glGenBuffers(1, &evbo);
        glBindBuffer(GL_ARRAY_BUFFER, evbo);
        glBufferData(GL_ARRAY_BUFFER, ev.size() * sizeof(CVert), ev.data(), GL_STATIC_DRAW);
        GLsizei es = sizeof(CVert);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, es, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, es, (void*)sizeof(glm::vec3));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, es, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
        glBindVertexArray(0);
    }

    glEnable(GL_CULL_FACE);

    m_pAudio->Initialise();
    m_pAudio->LoadMusicStream("resources\\Audio\\DST-Garote.mp3");
    m_pAudio->PlayMusicStream();

    m_pCamera->Set(
        glm::vec3(0.0f, 600.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));
}

void Game::Update()
{
    float dt_s = (float)m_dt * 0.001f;

    if (!m_gameOver) {
        m_gameTime += dt_s;
        m_lapTime += dt_s;
        m_playerT += m_playerSpeed * dt_s;
        if (m_playerT >= 1.0f) { m_playerT -= 1.0f; m_lapTime = 0.0f; }
        if (m_playerT < 0.0f)   m_playerT += 1.0f;
    }

    glm::vec3 playerPos = m_pSpline->GetPosition(m_playerT);
    glm::vec3 pT, pN, pB;
    m_pSpline->GetTNB(m_playerT, pT, pN, pB);

    switch (m_cameraMode)
    {
    case 0:
        m_pCamera->Update(m_dt * 5.0);
        break;
    case 1: // First person -- above and ahead of car
    {
        glm::vec3 camPos = playerPos + glm::vec3(0.0f, 14.0f, 0.0f) + pT * 5.0f;
        m_pCamera->Set(camPos, camPos + pT * 30.0f, glm::vec3(0, 1, 0));
        break;
    }
    case 2: // Third person -- behind and above
    {
        glm::vec3 camPos = playerPos - pT * 50.0f + glm::vec3(0.0f, 20.0f, 0.0f);
        m_pCamera->Set(camPos, playerPos + glm::vec3(0, 4, 0), glm::vec3(0, 1, 0));
        break;
    }
    case 3: // Top view -- tangent as up so road reads in direction of travel
        m_pCamera->Set(playerPos + glm::vec3(0, 250, 0), playerPos, pT);
        break;
    }

    // Barrel pickup collision -- distance check against 12-unit radius
    float pickupTs[5] = { 0.1f, 0.3f, 0.5f, 0.7f, 0.9f };

    if (m_playerT < m_lastLapT)
        for (int i = 0; i < 5; i++) m_collected[i] = false;
    m_lastLapT = m_playerT;

    for (int i = 0; i < 5; i++) {
        if (!m_collected[i]) {
            glm::vec3 pPos = m_pSpline->GetPosition(pickupTs[i]) + glm::vec3(0, 1, 0);
            if (glm::length(playerPos - pPos) < 12.0f) {
                m_score++;
                m_collected[i] = true;
            }
        }
    }

    m_wheelAngle += m_playerSpeed * 1600.0f * dt_s * 30.0f;
    if (m_wheelAngle > 360.0f) m_wheelAngle -= 360.0f;

    m_pAudio->Update();
}

void Game::Render()
{
    if (m_darkMode)
        glClearColor(m_stormMode ? 0.02f : 0.05f,
            m_stormMode ? 0.03f : 0.05f,
            m_stormMode ? 0.05f : 0.12f, 1.0f);
    else
        glClearColor(0.4f, 0.6f, 0.8f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glutil::MatrixStack modelViewMatrixStack;
    modelViewMatrixStack.SetIdentity();

    CShaderProgram* pMainProgram = (*m_pShaderPrograms)[0];
    pMainProgram->UseProgram();
    pMainProgram->SetUniform("bUseTexture", true);
    pMainProgram->SetUniform("sampler0", 0);
    pMainProgram->SetUniform("sampler1", 1);
    pMainProgram->SetUniform("CubeMapTex", 10);
    pMainProgram->SetUniform("bMultiTex", false);
    pMainProgram->SetUniform("bDarkMode", m_darkMode);

    float fogDens = m_stormMode ? 0.006f : (m_darkMode ? 0.0008f : 0.0f);
    pMainProgram->SetUniform("fogDensity", fogDens);

    glm::vec3 fogCol = m_stormMode
        ? glm::vec3(0.04f, 0.06f, 0.08f)
        : (m_darkMode ? glm::vec3(0.01f, 0.01f, 0.04f) : glm::vec3(0.5f, 0.6f, 0.7f));
    pMainProgram->SetUniform("fogColour", fogCol);
    pMainProgram->SetUniform("bStormMode", m_stormMode);
    pMainProgram->SetUniform("uTime", (float)m_gameTime);
    pMainProgram->SetUniform("matrices.projMatrix", m_pCamera->GetPerspectiveProjectionMatrix());

    modelViewMatrixStack.LookAt(
        m_pCamera->GetPosition(),
        m_pCamera->GetView(),
        m_pCamera->GetUpVector());
    glm::mat4 viewMatrix = modelViewMatrixStack.Top();

    // Directional sunlight -- disabled in dark mode (spotlights take over)
    pMainProgram->SetUniform("light1.position", viewMatrix * glm::vec4(-100, 200, -100, 1));
    if (m_darkMode) {
        pMainProgram->SetUniform("light1.La", glm::vec3(0.25f, 0.25f, 0.30f));
        pMainProgram->SetUniform("light1.Ld", glm::vec3(0.0f));
        pMainProgram->SetUniform("light1.Ls", glm::vec3(0.0f));
    }
    else {
        pMainProgram->SetUniform("light1.La", glm::vec3(0.3f));
        pMainProgram->SetUniform("light1.Ld", glm::vec3(1.0f));
        pMainProgram->SetUniform("light1.Ls", glm::vec3(1.0f));
    }

    // Spotlight 1 -- white headlight, follows player and aims forward and slightly down
    glm::vec3 playerPos = m_pSpline->GetPosition(m_playerT);
    glm::vec3 spT, spN, spB;
    m_pSpline->GetTNB(m_playerT, spT, spN, spB);

    pMainProgram->SetUniform("spotlight1.position", viewMatrix * glm::vec4(playerPos + glm::vec3(0, 12, 0), 1));
    pMainProgram->SetUniform("spotlight1.La", glm::vec3(0.3f));
    pMainProgram->SetUniform("spotlight1.Ld", glm::vec3(2.0f, 2.0f, 1.9f));
    pMainProgram->SetUniform("spotlight1.Ls", glm::vec3(1.0f));
    pMainProgram->SetUniform("spotlight1.direction", glm::normalize(glm::vec3(viewMatrix * glm::vec4(glm::normalize(spT + glm::vec3(0, -0.4f, 0)), 0))));
    pMainProgram->SetUniform("spotlight1.cutoff", cosf(glm::radians(m_stormMode ? 30.0f : 55.0f)));
    pMainProgram->SetUniform("spotlight1.exponent", 3.0f);

    // Spotlight 2 -- orange, fixed at the far side of the circuit
    glm::vec3 sl2World = m_pSpline->GetPosition(0.5f) + glm::vec3(0, 60, 0);
    pMainProgram->SetUniform("spotlight2.position", viewMatrix * glm::vec4(sl2World, 1));
    pMainProgram->SetUniform("spotlight2.La", glm::vec3(0.05f, 0.03f, 0.0f));
    pMainProgram->SetUniform("spotlight2.Ld", glm::vec3(1.0f, 0.5f, 0.1f));
    pMainProgram->SetUniform("spotlight2.Ls", glm::vec3(1.0f, 0.6f, 0.2f));
    pMainProgram->SetUniform("spotlight2.direction", glm::normalize(glm::vec3(viewMatrix * glm::vec4(0, -1, 0, 0))));
    pMainProgram->SetUniform("spotlight2.cutoff", cosf(glm::radians(60.0f)));
    pMainProgram->SetUniform("spotlight2.exponent", 2.0f);

    pMainProgram->SetUniform("material1.Ma", glm::vec3(1.0f));
    pMainProgram->SetUniform("material1.Md", m_darkMode ? glm::vec3(1.0f) : glm::vec3(0.5f));
    pMainProgram->SetUniform("material1.Ms", glm::vec3(0.3f));
    pMainProgram->SetUniform("material1.shininess", 15.0f);

    // Skybox
    modelViewMatrixStack.Push();
    pMainProgram->SetUniform("renderSkybox", true);
    modelViewMatrixStack.Translate(m_pCamera->GetPosition());
    pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
    pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
    m_pSkybox->Render(10);
    pMainProgram->SetUniform("renderSkybox", false);
    modelViewMatrixStack.Pop();

    // Terrain
    modelViewMatrixStack.Push();
    pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
    pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
    m_pPlanarTerrain->Render();
    modelViewMatrixStack.Pop();

    // Road -- polygon offset pulls it in front of terrain, eliminating Z-fighting
    glDisable(GL_CULL_FACE);
    pMainProgram->SetUniform("material1.Ma", glm::vec3(1.0f));
    pMainProgram->SetUniform("material1.Md", glm::vec3(1.0f));
    pMainProgram->SetUniform("material1.Ms", glm::vec3(0.2f));
    pMainProgram->SetUniform("material1.shininess", 8.0f);
    pMainProgram->SetUniform("bMultiTex", false);
    pMainProgram->SetUniform("bUseTexture", true);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-4.0f, -4.0f);
    glDepthFunc(GL_LEQUAL);
    glActiveTexture(GL_TEXTURE0);
    modelViewMatrixStack.Push();
    pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
    pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
    m_pTrack->Render();
    modelViewMatrixStack.Pop();
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDepthFunc(GL_LESS);

    pMainProgram->SetUniform("material1.Ma", glm::vec3(1.0f));
    pMainProgram->SetUniform("material1.Md", m_darkMode ? glm::vec3(1.0f) : glm::vec3(0.5f));
    pMainProgram->SetUniform("material1.Ms", glm::vec3(0.3f));
    pMainProgram->SetUniform("material1.shininess", 15.0f);

    // Centreline strip -- white in day, orange glow at night
    pMainProgram->SetUniform("bUseTexture", false);
    if (m_darkMode) {
        pMainProgram->SetUniform("material1.Ma", glm::vec3(1.0f, 0.6f, 0.0f));
        pMainProgram->SetUniform("material1.Md", glm::vec3(1.0f, 0.8f, 0.0f));
        pMainProgram->SetUniform("material1.Ms", glm::vec3(1.0f, 0.9f, 0.5f));
        pMainProgram->SetUniform("material1.shininess", 128.0f);
    }
    else {
        pMainProgram->SetUniform("material1.Ma", glm::vec3(1.0f));
        pMainProgram->SetUniform("material1.Md", glm::vec3(1.0f));
        pMainProgram->SetUniform("material1.Ms", glm::vec3(0.5f));
        pMainProgram->SetUniform("material1.shininess", 32.0f);
    }
    modelViewMatrixStack.Push();
    pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
    pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-4.0f, -4.0f);
    glBindVertexArray(m_centrelineVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_centrelineVerts);
    glDisable(GL_POLYGON_OFFSET_FILL);
    modelViewMatrixStack.Pop();

    // Edge glow strips -- night/storm only
    if (m_darkMode) {
        pMainProgram->SetUniform("material1.Ma", glm::vec3(0.9f, 0.95f, 1.0f));
        pMainProgram->SetUniform("material1.Md", glm::vec3(1.0f));
        pMainProgram->SetUniform("material1.Ms", glm::vec3(1.0f));
        pMainProgram->SetUniform("material1.shininess", 128.0f);
        for (int side = 0; side < 2; side++) {
            modelViewMatrixStack.Push();
            pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
            pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(-4.0f, -4.0f);
            glBindVertexArray(m_edgeVAO[side]);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, m_edgeVerts[side]);
            glDisable(GL_POLYGON_OFFSET_FILL);
            modelViewMatrixStack.Pop();
        }
    }

    pMainProgram->SetUniform("bUseTexture", true);
    pMainProgram->SetUniform("material1.Ma", glm::vec3(1.0f));
    pMainProgram->SetUniform("material1.Md", glm::vec3(0.5f));
    pMainProgram->SetUniform("material1.Ms", glm::vec3(0.3f));
    pMainProgram->SetUniform("material1.shininess", 15.0f);
    glEnable(GL_CULL_FACE);

    // Lane marker diamonds -- 40 around the track, animated per-marker
    glDisable(GL_CULL_FACE);
    m_pDiamondShaderProgram->UseProgram();
    m_pDiamondShaderProgram->SetUniform("bUseTexture", false);
    m_pDiamondShaderProgram->SetUniform("sampler0", 0);
    m_pDiamondShaderProgram->SetUniform("matrices.projMatrix", m_pCamera->GetPerspectiveProjectionMatrix());
    m_pDiamondShaderProgram->SetUniform("light1.position", viewMatrix * glm::vec4(-100, 200, -100, 1));
    m_pDiamondShaderProgram->SetUniform("light1.La", glm::vec3(1.0f));
    m_pDiamondShaderProgram->SetUniform("light1.Ld", glm::vec3(1.0f));
    m_pDiamondShaderProgram->SetUniform("light1.Ls", glm::vec3(1.0f));
    m_pDiamondShaderProgram->SetUniform("material1.shininess", 64.0f);

    for (int mk = 0; mk < 40; mk++) {
        float t = (float)mk / 40.0f;
        glm::vec3 centre = m_pSpline->GetPosition(t);
        glm::vec3 mT, mN, mB;
        m_pSpline->GetTNB(t, mT, mN, mB);
        glm::vec3 Nf = glm::normalize(glm::cross(glm::vec3(0, 1, 0), mT));
        float side = (mk % 2 == 0) ? 21.0f : -21.0f;
        float spin = m_gameTime * 90.0f + mk * 9.0f;
        float bob = sinf(m_gameTime * 2.0f + mk * 0.5f) * 0.5f;
        glm::vec3 pos = glm::vec3(centre.x + Nf.x * side, 2.5f + bob, centre.z + Nf.z * side);

        if (m_darkMode) {
            m_pDiamondShaderProgram->SetUniform("material1.Ma", glm::vec3(1.0f, 0.3f, 0.0f));
            m_pDiamondShaderProgram->SetUniform("material1.Md", glm::vec3(1.0f, 0.5f, 0.0f));
            m_pDiamondShaderProgram->SetUniform("material1.Ms", glm::vec3(1.0f, 0.8f, 0.2f));
        }
        else {
            m_pDiamondShaderProgram->SetUniform("material1.Ma", glm::vec3(1.0f, 0.9f, 0.0f));
            m_pDiamondShaderProgram->SetUniform("material1.Md", glm::vec3(1.0f, 1.0f, 0.0f));
            m_pDiamondShaderProgram->SetUniform("material1.Ms", glm::vec3(0.5f));
        }
        modelViewMatrixStack.Push();
        modelViewMatrixStack.Translate(pos);
        modelViewMatrixStack.Rotate(glm::vec3(0, 1, 0), spin);
        modelViewMatrixStack.Scale(3.0f);
        m_pDiamondShaderProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
        m_pDiamondShaderProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
        m_pDiamond->Render();
        modelViewMatrixStack.Pop();
    }
    glEnable(GL_CULL_FACE);

    pMainProgram->UseProgram();
    pMainProgram->SetUniform("bUseTexture", true);
    pMainProgram->SetUniform("matrices.projMatrix", m_pCamera->GetPerspectiveProjectionMatrix());

    // Barrel pickups -- only render uncollected ones
    float pickupTs[5] = { 0.1f, 0.3f, 0.5f, 0.7f, 0.9f };
    for (int i = 0; i < 5; i++) {
        if (m_collected[i]) continue;
        glm::vec3 pos = m_pSpline->GetPosition(pickupTs[i]) + glm::vec3(0, 1, 0);
        modelViewMatrixStack.Push();
        modelViewMatrixStack.Translate(pos);
        modelViewMatrixStack.Scale(2.5f);
        pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
        pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
        m_pBarrelMesh->Render();
        modelViewMatrixStack.Pop();
    }

    // Horse -- template mesh, off-route
    modelViewMatrixStack.Push();
    modelViewMatrixStack.Translate(glm::vec3(350, 0, 0));
    modelViewMatrixStack.Scale(5.0f);
    pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
    pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
    m_pHorseMesh->Render();
    modelViewMatrixStack.Pop();

    // Trees -- 8 instances outside the circuit boundary
    glm::vec3 treePos[8] = {
        {  550, 0,  300 }, {  550, 0, -300 },
        { -550, 0,  300 }, { -550, 0, -300 },
        {  200, 0,  520 }, { -200, 0,  520 },
        {  200, 0, -520 }, { -200, 0, -520 },
    };
    for (int i = 0; i < 8; i++) {
        modelViewMatrixStack.Push();
        modelViewMatrixStack.Translate(treePos[i]);
        modelViewMatrixStack.Scale(10.0f);
        pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
        pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
        m_pTreeMesh->Render();
        modelViewMatrixStack.Pop();
    }

    // Statues -- 4 instances at cardinal positions, rotated upright
    glm::vec3 statuePos[4] = {
        {  520, 0,    0 }, { -520, 0,    0 },
        {    0, 0,  520 }, {    0, 0, -520 },
    };
    for (int i = 0; i < 4; i++) {
        modelViewMatrixStack.Push();
        modelViewMatrixStack.Translate(statuePos[i]);
        modelViewMatrixStack.Rotate(glm::vec3(1, 0, 0), -90.0f);
        modelViewMatrixStack.Scale(0.25f);
        pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
        pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
        m_pLampMesh->Render();
        modelViewMatrixStack.Pop();
    }

    // Stone arches -- 4 instances further out
    glm::vec3 archPos[4] = {
        {  600, 0,    0 }, { -600, 0,    0 },
        {    0, 0, -600 }, {    0, 0,  600 },
    };
    for (int i = 0; i < 4; i++) {
        modelViewMatrixStack.Push();
        modelViewMatrixStack.Translate(archPos[i]);
        modelViewMatrixStack.Scale(5.0f);
        pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
        pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
        m_pRockMesh->Render();
        modelViewMatrixStack.Pop();
    }

    // Player car -- rotation matrix built from TNB frame
    {
        glm::vec3 cT, cN, cB;
        m_pSpline->GetTNB(m_playerT, cT, cN, cB);
        glm::vec3 pos(playerPos.x, 0.2f, playerPos.z);

        glm::mat4 matT(1.0f);
        matT[3] = glm::vec4(pos, 1.0f);

        glm::vec3 carRight = glm::normalize(glm::cross(glm::vec3(0, 1, 0), cT));
        glm::mat4 matR(
            glm::vec4(carRight, 0.0f),
            glm::vec4(0, 1, 0, 0.0f),
            glm::vec4(cT, 0.0f),
            glm::vec4(0, 0, 0, 1.0f)
        );

        glm::mat4 matS(1.0f);
        matS[0][0] = matS[1][1] = matS[2][2] = 8.0f;

        glm::mat4 carMV = viewMatrix * matT * matR * matS;
        pMainProgram->SetUniform("matrices.modelViewMatrix", carMV);
        pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(carMV));
        m_pCarMesh->Render();
    }

    // Central diamond gem -- spins at 60 degrees/s and bobs on a sine wave
    glDisable(GL_CULL_FACE);
    m_pDiamondShaderProgram->UseProgram();
    m_pDiamondShaderProgram->SetUniform("bUseTexture", true);
    m_pDiamondShaderProgram->SetUniform("sampler0", 0);
    m_pDiamondShaderProgram->SetUniform("matrices.projMatrix", m_pCamera->GetPerspectiveProjectionMatrix());
    m_pDiamondShaderProgram->SetUniform("light1.position", viewMatrix * glm::vec4(-100, 100, -100, 1));
    m_pDiamondShaderProgram->SetUniform("light1.La", glm::vec3(1.0f));
    m_pDiamondShaderProgram->SetUniform("light1.Ld", glm::vec3(1.0f));
    m_pDiamondShaderProgram->SetUniform("light1.Ls", glm::vec3(1.0f));
    m_pDiamondShaderProgram->SetUniform("material1.Ma", glm::vec3(1.0f));
    m_pDiamondShaderProgram->SetUniform("material1.Md", glm::vec3(1.0f));
    m_pDiamondShaderProgram->SetUniform("material1.Ms", glm::vec3(1.0f));
    m_pDiamondShaderProgram->SetUniform("material1.shininess", 64.0f);

    modelViewMatrixStack.Push();
    modelViewMatrixStack.Translate(glm::vec3(0, 15.0f + sinf(m_gameTime * 2.0f) * 2.0f, 0));
    modelViewMatrixStack.Rotate(glm::vec3(0, 1, 0), m_gameTime * 60.0f);
    modelViewMatrixStack.Scale(5.0f);
    m_pDiamondShaderProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
    m_pDiamondShaderProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
    m_pDiamond->Render();
    modelViewMatrixStack.Pop();
    glEnable(GL_CULL_FACE);

    DisplayFrameRate();
    SwapBuffers(m_gameWindow.Hdc());
}

void Game::DisplayFrameRate()
{
    CShaderProgram* fontProgram = (*m_pShaderPrograms)[1];
    RECT dimensions = m_gameWindow.GetDimensions();
    int w = dimensions.right - dimensions.left;
    int h = dimensions.bottom - dimensions.top;

    m_elapsedTime += m_dt;
    m_frameCount++;
    if (m_elapsedTime > 1000.0) {
        m_elapsedTime = 0.0;
        m_framesPerSecond = m_frameCount;
        m_frameCount = 0;
    }

    fontProgram->UseProgram();
    glDisable(GL_DEPTH_TEST);
    fontProgram->SetUniform("matrices.modelViewMatrix", glm::mat4(1));
    fontProgram->SetUniform("matrices.projMatrix", m_pCamera->GetOrthographicProjectionMatrix());

    int   speedKmh = (int)(m_playerSpeed * 1500.0f);
    int   lapSec = (int)m_lapTime;
    int   lapMs = (int)((m_lapTime - (float)lapSec) * 100.0f);
    const char* modes[] = { "FREE", "1ST PERSON", "3RD PERSON", "TOP VIEW" };

    // Drop shadow technique: each element drawn twice -- dark offset then full colour
    glm::vec4 shadow(0.0f, 0.0f, 0.0f, 0.65f);
    int sx = 2, sy = -2;

    auto renderText = [&](glm::vec4 col, int x, int y, int size, const char* fmt, auto... args) {
        fontProgram->SetUniform("vColour", shadow);
        m_pFtFont->Render(x + sx, y + sy, size, fmt, args...);
        fontProgram->SetUniform("vColour", col);
        m_pFtFont->Render(x, y, size, fmt, args...);
        };

    renderText(glm::vec4(1, 1, 1, 1), 20, h - 20, 20, "FPS: %d", m_framesPerSecond);
    renderText(glm::vec4(1, 1, 0.2f, 1), 20, h - 50, 24, "SPEED: %d km/h", speedKmh);
    renderText(glm::vec4(0.2f, 1, 1, 1), 20, h - 80, 24, "SCORE: %d", m_score);
    renderText(glm::vec4(1, 0.6f, 0.1f, 1), 20, h - 110, 24, "LAP: %02d.%02d s", lapSec, lapMs);
    renderText(glm::vec4(1, 1, 1, 0.8f), w - 220, h - 20, 20, "CAM: %s", modes[m_cameraMode]);

    if (m_stormMode)
        renderText(glm::vec4(0.4f, 0.7f, 1, 1), w - 200, h - 50, 20, "STORM MODE");
    else if (m_darkMode)
        renderText(glm::vec4(0.8f, 0.4f, 1, 1), w - 180, h - 50, 20, "NIGHT MODE");

    renderText(glm::vec4(0.9f, 0.9f, 0.9f, 0.7f),
        20, 30, 16, "W/S: Speed  1/2/3: Camera  L: Night  K: Storm  F: Free");

    glEnable(GL_DEPTH_TEST);
}

void Game::GameLoop()
{
    m_pHighResolutionTimer->Start();
    Update();
    Render();
    m_dt = m_pHighResolutionTimer->Elapsed();
}

WPARAM Game::Execute()
{
    m_pHighResolutionTimer = new CHighResolutionTimer;
    m_gameWindow.Init(m_hInstance);
    if (!m_gameWindow.Hdc()) return 1;
    Initialise();
    m_pHighResolutionTimer->Start();

    MSG msg;
    while (1) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else if (m_appActive) {
            GameLoop();
        }
        else Sleep(200);
    }
    m_gameWindow.Deinit();
    return msg.wParam;
}

LRESULT Game::ProcessEvents(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    LRESULT result = 0;
    switch (message)
    {
    case WM_ACTIVATE:
        switch (LOWORD(w_param)) {
        case WA_ACTIVE: case WA_CLICKACTIVE:
            m_appActive = true;
            m_pHighResolutionTimer->Start();
            break;
        case WA_INACTIVE:
            m_appActive = false;
            break;
        }
        break;

    case WM_SIZE:
    {
        RECT d;
        GetClientRect(window, &d);
        m_gameWindow.SetDimensions(d);
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(window, &ps);
        EndPaint(window, &ps);
        break;
    }
    case WM_KEYDOWN:
        switch (w_param)
        {
        case VK_ESCAPE: PostQuitMessage(0); break;
        case 'F': m_cameraMode = 0; break;
        case '1': m_cameraMode = 1; break;
        case '2': m_cameraMode = 2; break;
        case '3': m_cameraMode = 3; break;
        case 'L': m_darkMode = !m_darkMode; break;
        case 'K':
            m_stormMode = !m_stormMode;
            m_darkMode = m_stormMode;
            break;
        case 'W':
            if (m_cameraMode != 0)
                m_playerSpeed = glm::min(m_playerSpeed + 0.002f, 0.08f);
            break;
        case 'S':
            if (m_cameraMode != 0)
                m_playerSpeed = glm::max(m_playerSpeed - 0.002f, 0.0f);
            break;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        result = DefWindowProc(window, message, w_param, l_param);
        break;
    }
    return result;
}

Game& Game::GetInstance()
{
    static Game instance;
    return instance;
}

void Game::SetHinstance(HINSTANCE hinstance)
{
    m_hInstance = hinstance;
}

LRESULT CALLBACK WinProc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    return Game::GetInstance().ProcessEvents(window, message, w_param, l_param);
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, PSTR, int)
{
    Game& game = Game::GetInstance();
    game.SetHinstance(hinstance);
    return int(game.Execute());
}