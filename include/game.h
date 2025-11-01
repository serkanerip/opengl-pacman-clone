#ifndef GAME_H
#define GAME_H

#include <string.h>
#include "shader.h"
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "utils.h"
#include <queue>
#include <map>

enum GAME_STATE
{
  GAME_ACTIVE,
  GAME_MENU,
  GAME_WIN
};

enum GhostMode { SCATTER, CHASE, FRIGHTENED, EATEN };

struct Ghost
{
  glm::vec2 position;
  glm::vec2 direction;
  glm::vec2 velocity;
  float speed;
  unsigned int texture;
  GhostMode mode;
  glm::vec2 targetTile;
  glm::ivec2 scatterCorner;

  Ghost(std::string texturePath, glm::ivec2 scatterCorner) {
    texture = loadTexture(texturePath.c_str());
    this->scatterCorner = scatterCorner;
  }

  void updateGhostMode(float timer) {
    if (timer < 7)          mode = SCATTER;
    else if (timer < 27)    mode = CHASE;
    else if (timer < 34)    mode = SCATTER;
    else if (timer < 54)    mode = CHASE;
    else if (timer < 59)    mode = SCATTER;
    else                              mode = CHASE;
  }
};

struct Pacman
{
  glm::vec2 position = glm::vec2(0.0f, 0.0f);
  glm::vec2 direction = glm::vec2(0.0f, 0.0f);
  glm::vec2 velocity = glm::vec2(0.0f, 0.0f);
  float speed = 0.0f;
  float animationTime = 0.0f;
  unsigned int texture;
  glm::ivec2 currentTile;

  // textures for animation
  std::vector<unsigned int> upTextures;
  std::vector<unsigned int> downTextures;
  std::vector<unsigned int> leftTextures;
  std::vector<unsigned int> rightTextures;

  Pacman()
  {
    upTextures.push_back(loadTexture("pacman-art/pacman-up/1.png"));
    upTextures.push_back(loadTexture("pacman-art/pacman-up/2.png"));
    upTextures.push_back(loadTexture("pacman-art/pacman-up/3.png"));
    downTextures.push_back(loadTexture("pacman-art/pacman-down/1.png"));
    downTextures.push_back(loadTexture("pacman-art/pacman-down/2.png"));
    downTextures.push_back(loadTexture("pacman-art/pacman-down/3.png"));
    leftTextures.push_back(loadTexture("pacman-art/pacman-left/1.png"));
    leftTextures.push_back(loadTexture("pacman-art/pacman-left/2.png"));
    leftTextures.push_back(loadTexture("pacman-art/pacman-left/3.png"));
    rightTextures.push_back(loadTexture("pacman-art/pacman-right/1.png"));
    rightTextures.push_back(loadTexture("pacman-art/pacman-right/2.png"));
    rightTextures.push_back(loadTexture("pacman-art/pacman-right/3.png"));
    texture = rightTextures[0];
  }

  void updateTexture(float deltaTime)
  {
    animationTime += deltaTime;
    if (animationTime >= 0.1f)
    { // change frame every 0.1 seconds
      static int frame = 0;
      frame = (frame + 1) % 3;
      if (direction.x < 0)
      {
        texture = leftTextures[frame];
      }
      else if (direction.x > 0)
      {
        texture = rightTextures[frame];
      }
      else if (direction.y < 0)
      {
        texture = upTextures[frame];
      }
      else if (direction.y > 0)
      {
        texture = downTextures[frame];
      }
      animationTime = 0.0f;
    }
  }
};

struct Game
{
  float score = 0.0f;
  float tileSize = 32.0f;
  float startX = 200.0f;
  float startY = 200.0f;
  float gameTime = 0.0f;
  Pacman pacman;
  std::vector<std::string> map;
  GAME_STATE state = GAME_MENU;
  unsigned int VAO;
  Shader shaderProgram;

  // textures
  unsigned int wallTexture;
  unsigned int pelletTexture;

  // ghosts
  Ghost redGhost;

  float globalModeTimer = 0.0f;

  Game();
  void Reset();
  void Draw(glm::mat4 projection);
  void PhysicsUpdate(float deltaTime, glm::vec2 &desiredDir);

private:
  void updatePacmanPhysics(float deltaTime, glm::vec2 &desiredDir);
  void updateRedGhostPhysics(float deltaTime);
  bool centerAligned(glm::vec2 tilePx, glm::vec2 position, glm::vec2 velocity);
  glm::ivec2 pxToCell(glm::vec2 p);
  glm::vec2 cellToPx(glm::ivec2 cell);
};

glm::ivec2 Game::pxToCell(glm::vec2 p) {
    float fx = (p.x - startX) / tileSize;
    float fy = (p.y - startY) / tileSize;
    return { (int)std::round(fx), (int)std::round(fy) };
}

Game::Game() : redGhost("pacman-art/ghosts/blinky.png", {26, 1})
{
  // Initialize default map
  Reset();
  shaderProgram = Shader("./shaders/shader.vs", "./shaders/shader.fs");
  shaderProgram.use();
  shaderProgram.setInt("texture1", 0);

  glm::mat4 view = glm::mat4(1.0f);
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));

  wallTexture = loadTexture("wall.png");
  pelletTexture = loadTexture("pacman-art/other/dot.png");

  // quad vertices
  float vertices[] = {
      // positions        // texture coords
      0.5f, 0.5f, 0.0f, 1.0f, 1.0f,   // top right
      0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // bottom right
      -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // bottom left
      -0.5f, 0.5f, 0.0f, 0.0f, 1.0f   // top left
  };
  unsigned int indices[] = {
      0, 1, 3, // first triangle
      1, 2, 3  // second triangle
  };

  unsigned int VBO, VAO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO); // bind VAO

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0); // aPos
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float))); // aTexCoord
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind VBO
  glBindVertexArray(0);             // unbind VAO
  this->VAO = VAO;
}

void Game::Reset()
{
  score = 0.0f;
  pacman.position = glm::vec2(0.0f, 0.0f);
  pacman.direction = glm::vec2(0.0f, 0.0f);
  pacman.velocity = glm::vec2(0.0f, 0.0f);
  pacman.speed = tileSize * 5; // pixels per second
  redGhost.speed = tileSize * 4; // pixels per second
  redGhost.direction = glm::ivec2(0, -1);
  state = GAME_MENU;
  gameTime = 0.0f;

  map = {
      "############################",
      "#............##............#",
      "#.####.#####.##.#####.####.#",
      "#.####.#####.##.#####.####.#",
      "#.####.#####.##.#####.####.#",
      "#..........................#",
      "#.####.##.########.##.####.#",
      "#......##....##....##......#",
      "######.##### ## #####.######",
      "     #.##### ## #####.#     ",
      "     #.##          ##.#     ",
      "     #.## ###--### ##.#     ",
      "######.## #      # ##.######",
      "#     .   #      #   .     #",
      "######.## #  R   # ##.######",
      "     #.## ######## ##.#     ",
      "     #.##    P     ##.#     ",
      "     #.## ######## ##.#     ",
      "######.## ######## ##.######",
      "#............##............#",
      "#.####.#####.##.#####.####.#",
      "#...##................##...#",
      "###.##.##.########.##.##.###",
      "#......##....##....##......#",
      "#.##########.##.##########.#",
      "#..........................#",
      "############################"};

  for (unsigned int y = 0; y < map.size(); y++)
  {
    for (unsigned int x = 0; x < map[y].length(); x++)
    {
      if (map[y][x] == 'P')
      {
        pacman.position = glm::vec2(startX + (x * tileSize), startY + (y * tileSize));
        pacman.currentTile = glm::ivec2(x, y);
      }
      if (map[y][x] == 'R')
      {
        redGhost.position = glm::vec2(startX + (x * tileSize), startY + (y * tileSize));
      }
    }
  }
}

void Game::Draw(glm::mat4 projection)
{
  glBindVertexArray(this->VAO);
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

  for (unsigned int y = 0; y < map.size(); y++)
  {
    for (unsigned int x = 0; x < map[y].length(); x++)
    {
      if (map[y][x] == '#' || map[y][x] == '.') 
      {
        float xPos = startX + (x * tileSize);
        float yPos = startY + (y * tileSize);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(xPos, yPos, 0.0f));
        model = glm::scale(model, glm::vec3(tileSize, tileSize, 1.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
        unsigned int textureID = (map[y][x] == '.') ? pelletTexture : wallTexture;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      }
    }
  }

  // draw pacman
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(pacman.position, 0.0f));
  model = glm::scale(model, glm::vec3(tileSize, tileSize, 1.0f));
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, pacman.texture);
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  // draw red ghost
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(redGhost.position, 0.0f));
  model = glm::scale(model, glm::vec3(tileSize, tileSize, 1.0f));
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, redGhost.texture);
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  glBindVertexArray(0);
}

bool moveableTile(char tile)
{
  return tile != '#' && tile != '-';
}

bool canGhostMove(char tile)
{
  return tile != '#';
}

glm::vec2 Game::cellToPx(glm::ivec2 cell) {
    float px = startX + cell.x * tileSize;
    float py = startY + cell.y * tileSize;
    return glm::vec2(px, py);
}

bool Game::centerAligned(glm::vec2 tilePx, glm::vec2 position, glm::vec2 velocity) {
  float epsilon = velocity == glm::vec2(0.0f, 0.0f) ? 0.1f : glm::length(velocity) * 0.5f;
  return fabs(tilePx.x - position.x) < epsilon && fabs(tilePx.y - position.y) < epsilon;
}

void Game::updatePacmanPhysics(float deltaTime, glm::vec2 &desiredDir)
{
  pacman.currentTile = pxToCell(pacman.position);
  glm::vec2 pacmanTilePx = cellToPx(pacman.currentTile);
  bool isPacmanCenterAligned = centerAligned(pacmanTilePx, pacman.position, pacman.velocity);
  char* currentTileChar = &map[pacman.currentTile.y][pacman.currentTile.x];

  if (isPacmanCenterAligned)
  {
    pacman.position = pacmanTilePx; // Snap to center

    if (*currentTileChar == '.')
    {
      *currentTileChar = ' ';
      score += 10.0f;
      printf("Pellet eaten! Score: %.1f\n", score);
    }

    if ((desiredDir.x != 0 || desiredDir.y != 0))
    {
      auto nextTile = pxToCell(pacman.position) + (glm::ivec2)desiredDir;
      if (moveableTile(map[nextTile.y][nextTile.x]))
      {
        pacman.direction = desiredDir;
        desiredDir = glm::vec2(0, 0);
      }
    }
  }

  pacman.velocity = pacman.direction * deltaTime * pacman.speed;
  glm::ivec2 nextTile = pxToCell(pacman.position + pacman.velocity) + (glm::ivec2)pacman.direction;
  if (!isPacmanCenterAligned || moveableTile(map[nextTile.y][nextTile.x]))
  {
    pacman.position += pacman.velocity;
  }
  else
  {
    pacman.direction = glm::vec2(0, 0);
  }
}

void Game::updateRedGhostPhysics(float deltaTime)
{
  auto ghostCurrenTile = pxToCell(redGhost.position);
  auto ghostTilePx = cellToPx(ghostCurrenTile);
  bool isGhostCenterAligned = centerAligned(ghostTilePx, redGhost.position, redGhost.velocity);
  auto targetTile = pacman.currentTile;
  if (redGhost.mode == SCATTER)
    targetTile = redGhost.scatterCorner;

  if (isGhostCenterAligned)
  {
    // printf("Red ghost at tile (%d, %d)\n", ghostCurrenTile.x, ghostCurrenTile.y);
    redGhost.position = ghostTilePx; // Snap to center

    auto cmpVec2 = [](const glm::ivec2 &a, const glm::ivec2 &b) {
        return (a.y < b.y) || (a.y == b.y && a.x < b.x);
    };
    std::queue<glm::ivec2> q;
    std::map<glm::ivec2, glm::ivec2, decltype(cmpVec2)> cameFrom(cmpVec2);
    q.push(ghostCurrenTile);
    cameFrom[ghostCurrenTile] = ghostCurrenTile;

    static const glm::ivec2 dirs[] = {
        {1,0}, {-1,0}, {0,1}, {0,-1}
    };

    while (!q.empty())
    {
        glm::ivec2 cur = q.front(); q.pop();
        if (cur == targetTile) break;

        for (auto d : dirs)
        {
            glm::ivec2 next = cur + d;
            if (!canGhostMove(map[next.y][next.x])) continue;
            if (cameFrom.find(next) != cameFrom.end()) continue; // already visited
            // printf("Red ghost exploring tile (%d, %d) char=%c\n", next.x, next.y, map[next.y][next.x]);
            cameFrom[next] = cur;
            q.push(next);
        }
    }

    if (cameFrom.find(targetTile) == cameFrom.end())
    {
        printf("Red ghost could not find path to target tile: (%d, %d)\n", targetTile.x, targetTile.y);
        redGhost.direction = glm::vec2(0, 0);
        return; // no path found
    }

    // backtrack one step from goal to find direction
    glm::ivec2 step = targetTile;
    while (cameFrom.count(step) && cameFrom[step] != ghostCurrenTile)
      step = cameFrom[step];

    redGhost.direction = glm::sign(glm::vec2(step - ghostCurrenTile));
    // printf("Red ghost chooses direction (%.1f, %.1f)\n", redGhost.direction.x, redGhost.direction.y);
  }

  redGhost.velocity = redGhost.direction * deltaTime * redGhost.speed;
  redGhost.position += redGhost.velocity;
}

void Game::PhysicsUpdate(float deltaTime, glm::vec2 &desiredDir)
{
  gameTime += deltaTime;
  globalModeTimer += deltaTime;
  redGhost.updateGhostMode(globalModeTimer);
  updatePacmanPhysics(deltaTime, desiredDir);
  updateRedGhostPhysics(deltaTime);
}

#endif