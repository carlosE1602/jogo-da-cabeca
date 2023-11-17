// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include "SDL_image.h"
#include "Random.h"
#include "Game.h"
#include "Actors/Actor.h"
#include "Actors/Pacman.h"
#include "Actors/Item.h"
#include "Actors/Ghost/Ghost.h"
#include "Actors/Wall.h"
#include "Components/DrawComponents/DrawComponent.h"
#include "Components/DrawComponents/DrawSpriteComponent.h"
#include "Components/AIComponents/FSMComponent.h"
#include "Actors/PathNode.h"

Game::Game(int windowWidth, int windowHeight)
        :mWindow(nullptr)
        ,mRenderer(nullptr)
        ,mTicksCount(0)
        ,mIsRunning(true)
        ,mUpdatingActors(false)
        ,mWindowWidth(windowWidth)
        ,mWindowHeight(windowHeight)
        ,mRespawnTimer(RESPAWN_TIME)
{

}

bool Game::Initialize()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    mWindow = SDL_CreateWindow("P4: Super Pacman Bros", 0, 0, mWindowWidth, mWindowHeight, 0);
    if (!mWindow)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!mRenderer)
    {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        return false;
    }

    Random::Init();

    mTicksCount = SDL_GetTicks();

    // Init all game actors
    InitializeActors();

    return true;
}

void Game::InitializeActors()
{
    // Background
    auto* background = new Actor(this);
    background->SetPosition(Vector2(234.0f, 258.0f));
    new DrawSpriteComponent(background, "../Assets/Sprites/Background.png", 448, 496);

    LoadLevel("../Assets/Levels/Level.txt");
    LoadPaths("../Assets/Levels/Paths.txt");

    SetGameState(State::Intro);
}

void Game::SetGameState(State gameState)
{
    if (gameState == Intro)
    {
        // Reset pacman
        mPacman->Start();

        // Reset ghosts
        for(auto *ghost : mGhosts) {
            ghost->Start();
        }
    }
    else if(gameState == Started)
    {
        for(auto *ghost : mGhosts) {
            ghost->GetComponent<FSMComponent>()->Start("scatter");
        }
    }

    mGameState = gameState;
}

void Game::LoadLevel(const std::string& levelPath)
{
    const float STARTX = 18.0f;
    const float STARTY = 18.0f;
    const float SPACING = 16.0f;

    std::ifstream file(levelPath);
    if (!file.is_open())
    {
        SDL_Log("Failed to load level: %s", levelPath.c_str());
    }

    size_t row = 0;
    std::string line;
    while (!file.eof())
    {
        std::getline(file, line);
        for (size_t col = 0; col < line.size(); col++)
        {
            Vector2 pos;
            pos.x = STARTX + SPACING * col;
            pos.y = STARTY + SPACING * row;

            char letter = line[col];
            if (letter == 'p')
            {
                Item* p = new Item(this, Item::Type::Pellet, 4, 4);
                p->SetPosition(pos);
            }
            else if (letter == 'P')
            {
                Item* p = new Item(this, Item::Type::PowerPellet, 16, 16);
                p->SetPosition(pos);
            }
            else if (letter == 'M')
            {
                pos.x -= SPACING / 2.0f;
                mPacman = new Pacman(this);
                mPacman->SetPosition(pos);
            }
            else if (letter == '-')
            {
                auto* wall = new Wall(this);
                wall->SetPosition(pos);
            }
            else if (letter >= '1' && letter <= '4')
            {
                Ghost::Type ghostType = static_cast<Ghost::Type>(letter - '1');
                if (ghostType < GHOST_COUNT)
                {
                    auto *ghost = new Ghost(this, ghostType);
                    ghost->SetPosition(pos);
                }
            }
        }
        row++;
    }
}

static bool IsPathNode(char adj)
{
    return adj == 'X' || adj == 'T' || adj == 'G' || adj == 'M' || adj == 'P' ||
           (adj >= '1' && adj <= '4') ||
           (adj >= 'A' && adj <= 'D');
}

static bool IsPath(char adj)
{
    return IsPathNode(adj) || adj == '*';
}

void Game::LoadPaths(const std::string& fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open())
    {
        SDL_Log("Failed to load paths: %s", fileName.c_str());
    }

    std::vector<std::vector<char>> txtGrid;
    std::vector<std::vector<PathNode*>> nodeGrid;

    BuildPathGraphVertices(file, txtGrid, nodeGrid);
    BuildPathGraphEdges(txtGrid, nodeGrid);
}

void Game::BuildPathGraphVertices(std::ifstream& file,
                                  std::vector<std::vector<char>>& txtGrid,
                                  std::vector<std::vector<PathNode*>> &nodeGrid)
{
    const float STARTX = 34.0f;
    const float STARTY = 34.0f;
    const float SPACING = 16.0f;

    // TODO [Parte 1.1]: https://ufv-inf216.lucasnferreira.com/p5-pacman


    std::string line;
    while (std::getline(file, line)) {
        txtGrid.emplace_back(line.begin(), line.end());
    }


    nodeGrid.resize(txtGrid.size());

    for (int i = 0; i < txtGrid.size(); ++i) {
        nodeGrid[i].resize(txtGrid[i].size());

        for (int j = 0; j < txtGrid[i].size(); ++j) {

            char currentChar = txtGrid[i][j];

            PathNode::Type nodeType = PathNode::Type::Default;
            if(!IsPathNode(currentChar)) continue;
            if (currentChar == 'X' || currentChar == 'A' || currentChar == 'B' ||
                currentChar == 'C' || currentChar == 'D' || currentChar == '1' ||
                currentChar == '2' || currentChar == 'M') {
                nodeType = PathNode::Type::Default;
            } else if (currentChar == 'G' || currentChar == 'P' ||
                       currentChar == '3' || currentChar == '4') {
                nodeType = PathNode::Type::Ghost;
            } else if (currentChar == 'T') {
                nodeType = PathNode::Type::Tunnel;
            }

            nodeGrid[i][j] = new PathNode(this, nodeType);
            float x = STARTX + SPACING * j + ((currentChar == 'M' || currentChar == 'T') ? -SPACING / 2.0f : 0.0f);
            float y = STARTY + SPACING * i;

            nodeGrid[i][j]->SetPosition(Vector2(x, y));

            auto node = nodeGrid[i][j];
            if (currentChar >= 'A' && currentChar <= 'D') {
                mGhosts[currentChar - 'A']->SetScatterNode(node);
            } else if (currentChar >= '1' && currentChar <= '4') {
                mGhosts[currentChar - '1']->SetSpawnNode(node);
            } else if (currentChar == 'M') {
                mPacman->SetSpawnNode(node);
            } else if (currentChar == 'T') {
                (mTunnelLeft ? mTunnelRight : mTunnelLeft) = node;
            } else if (currentChar == 'G') {
                mGhostPen = node;
            }
        }
    }

}

bool ConnectIfPath(std::vector<std::vector<PathNode*>>& nodeGrid, std::vector<std::vector<char>>& txtGrid, int i, int j, int k, int l) {
    if (nodeGrid[k][l] != nullptr && IsPath(txtGrid[k][l])) {
        nodeGrid[i][j]->AddAdjacent(nodeGrid[k][l]);
        nodeGrid[k][l]->AddAdjacent(nodeGrid[i][j]);
        return true;
    } else if (nodeGrid[k][l] != nullptr && !IsPath(txtGrid[k][l])) {
        return true;
    }
    return false;
}

void Game::BuildPathGraphEdges(std::vector<std::vector<char>>& txtGrid,
                               std::vector<std::vector<PathNode*>> &nodeGrid)
{
    // Now hook up paths
    // size_t numRows = nodeGrid.size();
    // size_t numCols = nodeGrid[0].size();

    // TODO [Parte 1.2]: https://ufv-inf216.lucasnferreira.com/p5-pacman
    int tunnelX = -1, tunnelY = -1;

    for (int i = 0; i < txtGrid.size(); ++i) {
        for (int j = 0; j < txtGrid[i].size(); ++j) {
            if(!IsPathNode(txtGrid[i][j])) continue;

            if (txtGrid[i][j] == 'T') {
                if (tunnelX == -1) {
                    tunnelX = j;
                    tunnelY = i;
                } else {

                    nodeGrid[i][j]->AddAdjacent(nodeGrid[tunnelY][tunnelX]);
                    nodeGrid[tunnelY][tunnelX]->AddAdjacent(nodeGrid[i][j]);
                }
            }

            if (IsPath(txtGrid[i][j])) {
                auto row = txtGrid[i];
                for (int k = j + 1; k < row.size(); ++k) {
                    if(!IsPath(txtGrid[i][k])) break;
                    if (ConnectIfPath(nodeGrid, txtGrid, i, j, i, k)) {
                        break;
                    }
                }

                for (int k = i + 1; k < txtGrid.size(); ++k) {
                    if(!IsPath(txtGrid[k][j])) break;
                    if (ConnectIfPath(nodeGrid, txtGrid, i, j, k, j)) {
                        break;
                    }
                }
            }
        }
    }

}

void Game::RunLoop()
{
    while (mIsRunning)
    {
        ProcessInput();
        UpdateGame();
        GenerateOutput();
    }
}

void Game::ProcessInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                Quit();
                break;
        }
    }

    const Uint8* state = SDL_GetKeyboardState(nullptr);

    for (auto actor : mActors)
    {
        actor->ProcessInput(state);
    }

    if (state[SDL_SCANCODE_D] || state[SDL_SCANCODE_A] ||
        state[SDL_SCANCODE_W] || state[SDL_SCANCODE_S])
    {
        if(mGameState == State::Intro) {
            SetGameState(State::Started);
        }
    }

    // Toggles for debug path views
    if (!mPrev1Input && state[SDL_SCANCODE_1])
    {
        mShowGhostPaths = !mShowGhostPaths;
    }

    if (!mPrev2Input && state[SDL_SCANCODE_2])
    {
        mShowGraph = !mShowGraph;
        for (auto p : mPathNodes)
        {
            p->GetComponent<DrawSpriteComponent>()->SetIsVisible(mShowGraph);
        }
    }

    mPrev1Input = static_cast<bool>(state[SDL_SCANCODE_1]);
    mPrev2Input = static_cast<bool>(state[SDL_SCANCODE_2]);
}

void Game::UpdateGame()
{
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16));

    float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
    if (deltaTime > 0.05f)
    {
        deltaTime = 0.05f;
    }

    mTicksCount = SDL_GetTicks();

    // Update all actors and pending actors
    UpdateActors(deltaTime);

    // Update camera position
    UpdateCamera();

    // Update game state
    UpdateState(deltaTime);
}

void Game::UpdateCamera()
{

}

void Game::UpdateActors(float deltaTime)
{
    mUpdatingActors = true;
    for (auto actor : mActors)
    {
        actor->Update(deltaTime);
    }
    mUpdatingActors = false;

    for (auto pending : mPendingActors)
    {
        mActors.emplace_back(pending);
    }
    mPendingActors.clear();

    std::vector<Actor*> deadActors;
    for (auto actor : mActors)
    {
        if (actor->GetState() == ActorState::Destroy)
        {
            deadActors.emplace_back(actor);
        }
    }

    for (auto actor : deadActors)
    {
        delete actor;
    }
}

void Game::UpdateState(float deltaTime)
{
    // If pacman is dead, respawn after some time
    if(mPacman->IsDead())
    {
        mGameState = State::Over;
        for(auto *ghost : mGhosts) {
            ghost->Pause();
        }

        mRespawnTimer -= deltaTime;
        if(mRespawnTimer <= .0f)
        {
            mRespawnTimer = RESPAWN_TIME;
            SetGameState(State::Intro);
        }
    }

    // Count number of pellets in game
    int pellets_count = 0;
    for(auto *item : mItems) {
        if(item->GetType() == Item::Type::Pellet) {
            pellets_count += 1;
        }
    }

    // If there are no more pellets, player won
    // For now, just pause the game
    if(pellets_count == 0)
    {
        mGameState = State::Won;
        for(auto *ghost : mGhosts) {
            ghost->Pause();
        }
    }
}

void Game::AddActor(Actor* actor)
{
    if (mUpdatingActors)
    {
        mPendingActors.emplace_back(actor);
    }
    else
    {
        mActors.emplace_back(actor);
    }
}

void Game::RemoveActor(Actor* actor)
{
    auto iter = std::find(mPendingActors.begin(), mPendingActors.end(), actor);
    if (iter != mPendingActors.end())
    {
        // Swap to end of vector and pop off (avoid erase copies)
        std::iter_swap(iter, mPendingActors.end() - 1);
        mPendingActors.pop_back();
    }

    iter = std::find(mActors.begin(), mActors.end(), actor);
    if (iter != mActors.end())
    {
        // Swap to end of vector and pop off (avoid erase copies)
        std::iter_swap(iter, mActors.end() - 1);
        mActors.pop_back();
    }
}

void Game::AddDrawable(class DrawComponent *drawable)
{
    mDrawables.emplace_back(drawable);

    std::sort(mDrawables.begin(), mDrawables.end(),[](DrawComponent* a, DrawComponent* b) {
        return a->GetDrawOrder() < b->GetDrawOrder();
    });
}

void Game::RemoveDrawable(class DrawComponent *drawable)
{
    auto iter = std::find(mDrawables.begin(), mDrawables.end(), drawable);
    mDrawables.erase(iter);
}

void Game::AddGhost(Ghost* ghost)
{
    mGhosts.emplace_back(ghost);
}

void Game::RemoveGhost(Ghost* ghost)
{
    auto iter = std::find(mGhosts.begin(), mGhosts.end(), ghost);
    mGhosts.erase(iter);
}

void Game::AddItem(Item* item)
{
    mItems.emplace_back(item);
}

void Game::RemoveItem(Item* item)
{
    auto iter = std::find(mItems.begin(), mItems.end(), item);
    mItems.erase(iter);
}

void Game::AddWall(Wall* wall)
{
    mWalls.emplace_back(wall);
}

void Game::RemoveWall(Wall* item)
{
    auto iter = std::find(mWalls.begin(), mWalls.end(), item);
    mWalls.erase(iter);
}

void Game::AddPathNode(class PathNode *node)
{
    mPathNodes.emplace_back(node);
}

void Game::RemovePathNode(class PathNode *node)
{
    auto iter = std::find(mPathNodes.begin(), mPathNodes.end(), node);
    mPathNodes.erase(iter);
}

void Game::GenerateOutput()
{
    // Set draw color to black
    SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);

    // Clear back buffer
    SDL_RenderClear(mRenderer);

    // Debug Ghost AI
    DebugDrawPaths();

    for (auto drawable : mDrawables)
    {
        if (drawable->IsVisible())
        {
            drawable->Draw(mRenderer);
        }
    }

    // Swap front buffer and back buffer
    SDL_RenderPresent(mRenderer);
}

void Game::DebugDrawPaths()
{
    if (mShowGraph)
    {
        SDL_SetRenderDrawColor(mRenderer, 127, 127, 127, 255);
        for (auto p : mPathNodes)
        {
            if (p->GetType() != PathNode::Tunnel)
            {
                for (auto n : p->GetAdjacents())
                {
                    SDL_RenderDrawLine(mRenderer, static_cast<int>(p->GetPosition().x),
                                       static_cast<int>(p->GetPosition().y),
                                       static_cast<int>(n->GetPosition().x),
                                       static_cast<int>(n->GetPosition().y));
                }
            }
        }
    }

    if (mShowGhostPaths)
    {
        // Now draw ghost paths
        for (auto g : mGhosts) {
            g->DebugDrawPath(mRenderer);
        }
    }
}

SDL_Texture* Game::LoadTexture(const std::string& texturePath) {
    // Load from file
    SDL_Surface* surf = IMG_Load(texturePath.c_str());
    if (!surf)
    {
        SDL_Log("Failed to load texture file %s", texturePath.c_str());
        return nullptr;
    }

    // Create texture from surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(mRenderer, surf);
    SDL_FreeSurface(surf);
    if (!texture)
    {
        SDL_Log("Failed to convert surface to texture for %s", texturePath.c_str());
        return nullptr;
    }

    return texture;
}

void Game::Shutdown()
{
    while (!mActors.empty())
    {
        delete mActors.back();
    }

    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}
