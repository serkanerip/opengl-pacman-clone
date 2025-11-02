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
#include "miniaudio.h"

enum GAME_STATE
{
  GAME_ACTIVE,
  GAME_MENU,
  GAME_WIN
};

enum GhostMode
{
  SCATTER,
  CHASE,
  FRIGHTENED,
  EATEN
};
enum GhostType
{
  BLINKY,
  PINKY,
  INKY,
  CLYDE
};

// ghost type to symbol mapping
std::map<GhostType, char> ghostTypeToSymbol = {
    {BLINKY, 'B'},
    {PINKY, 'P'},
    {INKY, 'I'},
    {CLYDE, 'C'}};

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
  GhostType type;
  char ghostSymbol;

  glm::vec2 housePosition;

  float frightenedUntil = 0.0f;

  Ghost(std::string texturePath, glm::ivec2 scatterCorner, GhostType type)
  {
    texture = loadTexture(texturePath.c_str());
    this->scatterCorner = scatterCorner;
    this->type = type;
    this->ghostSymbol = ghostTypeToSymbol[type];
  }

  void frighten(float duration)
  {
    mode = FRIGHTENED;
    speed *= 0.5f;
    frightenedUntil = duration;
    printf("Ghost frightened until %.2f seconds\n", frightenedUntil);
  }

  void updateGhostMode(float timer)
  {
    if (mode == FRIGHTENED)
      if (timer >= frightenedUntil)
      {
        printf("Ghost remains frightened at time %.2f seconds\n", timer);
        speed *= 2.0f;
      }
      else
      {
        return;
      }
    if (mode == EATEN)
    {
      if (position != housePosition) {
        return;
      }
    }
    if (timer < 7)
      mode = SCATTER;
    else if (timer < 27)
      mode = CHASE;
    else if (timer < 34)
      mode = SCATTER;
    else if (timer < 54)
      mode = CHASE;
    else if (timer < 59)
      mode = SCATTER;
    else
      mode = CHASE;
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
  unsigned int appleTexture;
  unsigned int frigthenedTexture;

  // ghosts
  std::vector<Ghost> ghosts;

  float frightenedUntil = 0.0f;
  float globalModeTimer = 0.0f;

  // sounds
  ma_engine soundEngine;
  ma_sound chompSound;

  Game();
  ~Game()
  {
    ma_sound_uninit(&chompSound);
    ma_engine_uninit(&soundEngine);
  }
  void Reset();
  void Draw(glm::mat4 projection);
  void PhysicsUpdate(float deltaTime, glm::vec2 &desiredDir);

private:
  void updatePacmanPhysics(float deltaTime, glm::vec2 &desiredDir);
  void updateGhostPhysics(Ghost &ghost, float deltaTime);
  bool centerAligned(glm::vec2 tilePx, glm::vec2 position, glm::vec2 velocity);
  glm::ivec2 pxToCell(glm::vec2 p);
  glm::vec2 cellToPx(glm::ivec2 cell);
};

glm::ivec2 Game::pxToCell(glm::vec2 p)
{
  float fx = (p.x - startX) / tileSize;
  float fy = (p.y - startY) / tileSize;
  return {(int)std::round(fx), (int)std::round(fy)};
}

Game::Game()
{
  // Initialize default map
  ma_result result;
  result = ma_engine_init(NULL, &soundEngine);
  if (result != MA_SUCCESS)
  {
    printf("Failed to initialize sound engine: %d\n", result);
  }
  else
  {
    printf("Sound engine initialized successfully.\n");
    // Initialize the chomp sound
    result = ma_sound_init_from_file(&soundEngine, "sounds/chomp.mp3", 0, NULL, NULL, &chompSound);
    if (result != MA_SUCCESS)
    {
      printf("Failed to initialize chomp sound: %d\n", result);
    }
  }

  ghosts.push_back(Ghost("pacman-art/ghosts/blinky.png", {26, 1}, BLINKY));
  ghosts.push_back(Ghost("pacman-art/ghosts/pinky.png", {3, 1}, PINKY));
  ghosts.push_back(Ghost("pacman-art/ghosts/inky.png", {1, 25}, INKY));
  ghosts.push_back(Ghost("pacman-art/ghosts/clyde.png", {26, 25}, CLYDE));

  shaderProgram = Shader("./shaders/shader.vs", "./shaders/shader.fs");
  shaderProgram.use();
  shaderProgram.setInt("texture1", 0);

  glm::mat4 view = glm::mat4(1.0f);
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));

  wallTexture = loadTexture("wall.png");
  pelletTexture = loadTexture("pacman-art/other/dot.png");
  appleTexture = loadTexture("pacman-art/other/apple.png");
  frigthenedTexture = loadTexture("pacman-art/ghosts/blue_ghost.png");

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
  Reset();
}

void Game::Reset()
{
  score = 0.0f;
  pacman.position = glm::vec2(0.0f, 0.0f);
  pacman.direction = glm::vec2(0.0f, 0.0f);
  pacman.velocity = glm::vec2(0.0f, 0.0f);
  pacman.speed = tileSize * 8; // pixels per second
  for (auto &ghost : ghosts)
  {
    ghost.direction = glm::ivec2(0, -1);
    ghost.velocity = glm::vec2(0.0f, 0.0f);
    ghost.speed = tileSize * 8; // pixels per second
    ghost.mode = SCATTER;
  }
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
      "######.## # IBPC # ##.######",
      "     #.## ######## ##.#     ",
      "     #.##          ##.#     ",
      "     #.## ######## ##.#     ",
      "######.## ######## ##.######",
      "#............##............#",
      "#.####.#####.##.#####.####.#",
      "#...##................##...#",
      "###.##.##.########.##.##.###",
      "#......##....##....##......#",
      "#.##########.##.##########.#",
      "#.*......................A.#",
      "############################"};

  for (unsigned int y = 0; y < map.size(); y++)
  {
    for (unsigned int x = 0; x < map[y].length(); x++)
    {
      if (x == 14 && y == 16)
      {
        pacman.position = glm::vec2(startX + (x * tileSize), startY + (y * tileSize));
        pacman.currentTile = glm::ivec2(x, y);
      }
      if (map[y][x] == 'B' || map[y][x] == 'P' || map[y][x] == 'I' || map[y][x] == 'C')
      {
        for (auto &ghost : ghosts)
        {
          if (ghost.ghostSymbol == map[y][x])
          {
            ghost.position = glm::vec2(startX + (x * tileSize), startY + (y * tileSize));
            ghost.housePosition = ghost.position;
          }
        }
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
      if (map[y][x] == '#' || map[y][x] == '.' || map[y][x] == 'A' || map[y][x] == '*')
      {
        float xPos = startX + (x * tileSize);
        float yPos = startY + (y * tileSize);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(xPos, yPos, 0.0f));
        model = glm::scale(model, glm::vec3(tileSize, tileSize, 1.0f));
        unsigned int textureID = (map[y][x] == '.') ? pelletTexture : wallTexture;
        if (map[y][x] == 'A')
          textureID = appleTexture;
        // big pellet
        if (map[y][x] == '*')
        {
          model = glm::scale(model, glm::vec3(3.0f, 3.0f, 1.0f));
          textureID = pelletTexture;
        }
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));

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
  for (auto &ghost : ghosts)
  {
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(ghost.position, 0.0f));
    model = glm::scale(model, glm::vec3(tileSize, tileSize, 1.0f));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ghost.mode == FRIGHTENED ? frigthenedTexture : ghost.texture);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  }

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

glm::vec2 Game::cellToPx(glm::ivec2 cell)
{
  float px = startX + cell.x * tileSize;
  float py = startY + cell.y * tileSize;
  return glm::vec2(px, py);
}

bool Game::centerAligned(glm::vec2 tilePx, glm::vec2 position, glm::vec2 velocity)
{
  float epsilon = velocity == glm::vec2(0.0f, 0.0f) ? 0.1f : glm::length(velocity) * 0.5f;
  return fabs(tilePx.x - position.x) < epsilon && fabs(tilePx.y - position.y) < epsilon;
}

void Game::updatePacmanPhysics(float deltaTime, glm::vec2 &desiredDir)
{
  pacman.currentTile = pxToCell(pacman.position);
  glm::vec2 pacmanTilePx = cellToPx(pacman.currentTile);
  bool isPacmanCenterAligned = centerAligned(pacmanTilePx, pacman.position, pacman.velocity);
  char *currentTileChar = &map[pacman.currentTile.y][pacman.currentTile.x];

  if (isPacmanCenterAligned)
  {
    pacman.position = pacmanTilePx; // Snap to center

    if (*currentTileChar == '.')
    {
      // Check if the chomp sound is not currently playing, or restart if it's already playing
      if (!ma_sound_is_playing(&chompSound))
      {
        printf("Playing chomp sound.\n");
        ma_sound_seek_to_pcm_frame(&chompSound, 0); // Reset to beginning
        ma_sound_start(&chompSound);                // Start playing
      }
      *currentTileChar = ' ';
      score += 10.0f;
      printf("Pellet eaten! Score: %.1f\n", score);
    }
    else if (*currentTileChar == 'A')
    {
      *currentTileChar = ' ';
      score += 100.0f;
      printf("Apple eaten! Score: %.1f\n", score);
      ma_engine_play_sound(&soundEngine, "sounds/pacman_eatfruit.wav", NULL);
    }
    else if (*currentTileChar == '*')
    {
      *currentTileChar = ' ';
      score += 50.0f;
      printf("Big pellet eaten! Score: %.1f\n", score);
      for (auto &ghost : ghosts)
      {
        ghost.frighten(gameTime + 7.0f); // frightened for 10 seconds
      }
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

void Game::updateGhostPhysics(Ghost &ghost, float deltaTime)
{
  auto ghostCurrenTile = pxToCell(ghost.position);
  auto ghostTilePx = cellToPx(ghostCurrenTile);
  bool isGhostCenterAligned = centerAligned(ghostTilePx, ghost.position, ghost.velocity);
  glm::ivec2 targetTile;
  if (ghost.type == BLINKY)
    targetTile = pacman.currentTile;
  if (ghost.type == PINKY)
    targetTile = pacman.currentTile + glm::ivec2(4 * (int)pacman.direction.x, 4 * (int)pacman.direction.y);
  if (ghost.type == INKY)
  {
    glm::ivec2 blinkyTile;
    for (auto &g : ghosts)
    {
      if (g.type == BLINKY)
      {
        blinkyTile = pxToCell(g.position);
        break;
      }
    }
    glm::ivec2 vector = pacman.currentTile + glm::ivec2(2 * (int)pacman.direction.x, 2 * (int)pacman.direction.y) - blinkyTile;
    targetTile = blinkyTile + vector;
  }
  if (ghost.type == CLYDE)
  {
    float distance = glm::length(glm::vec2(ghostCurrenTile - pacman.currentTile));
    if (distance > 8.0f)
      targetTile = pacman.currentTile;
    else
      targetTile = ghost.scatterCorner;
  }
  
  if (ghost.mode == SCATTER)
    targetTile = ghost.scatterCorner;

  if (ghost.mode == EATEN)
    targetTile = pxToCell(ghost.housePosition);

  if (isGhostCenterAligned)
  {
    static const glm::ivec2 dirs[] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    // printf("Red ghost at tile (%d, %d)\n", ghostCurrenTile.x, ghostCurrenTile.y);
    ghost.position = ghostTilePx; // Snap to center
    if (ghost.mode == FRIGHTENED)
    {
      std::vector<glm::ivec2> possibleDirs;
      for (auto d : dirs)
      {
        glm::ivec2 next = ghostCurrenTile + d;
        if (canGhostMove(map[next.y][next.x]) && glm::vec2(d) != -ghost.direction)
        {
          possibleDirs.push_back(d);
        }
      }
      if (!possibleDirs.empty())
      {
        int r = rand() % possibleDirs.size();
        ghost.direction = glm::vec2(possibleDirs[r]);
      }
      else
      {
        ghost.direction = -ghost.direction; // reverse
      }
      ghost.velocity = ghost.direction * deltaTime * ghost.speed;
      ghost.position += ghost.velocity;
      // printf("Frightened ghost chooses direction (%.1f, %.1f)\n", ghost.direction.x, ghost.direction.y);
      return;
    }

    auto cmpVec2 = [](const glm::ivec2 &a, const glm::ivec2 &b)
    {
      return (a.y < b.y) || (a.y == b.y && a.x < b.x);
    };
    std::queue<glm::ivec2> q;
    std::map<glm::ivec2, glm::ivec2, decltype(cmpVec2)> cameFrom(cmpVec2);
    q.push(ghostCurrenTile);
    cameFrom[ghostCurrenTile] = ghostCurrenTile;

    while (!q.empty())
    {
      glm::ivec2 cur = q.front();
      q.pop();
      if (cur == targetTile)
        break;

      for (auto d : dirs)
      {
        glm::ivec2 next = cur + d;
        if (!canGhostMove(map[next.y][next.x]))
          continue;
        if (cameFrom.find(next) != cameFrom.end())
          continue; // already visited
        // printf("Red ghost exploring tile (%d, %d) char=%c\n", next.x, next.y, map[next.y][next.x]);
        cameFrom[next] = cur;
        q.push(next);
      }
    }

    if (cameFrom.find(targetTile) == cameFrom.end())
    {
      // printf("Ghost could not find path to target tile: (%d, %d)\n", targetTile.x, targetTile.y);
      ghost.direction = glm::vec2(0, 0);
      return; // no path found
    }

    // backtrack one step from goal to find direction
    glm::ivec2 step = targetTile;
    while (cameFrom.count(step) && cameFrom[step] != ghostCurrenTile)
      step = cameFrom[step];

    ghost.direction = glm::sign(glm::vec2(step - ghostCurrenTile));
    // printf("Red ghost chooses direction (%.1f, %.1f)\n", redGhost.direction.x, redGhost.direction.y);
  }

  ghost.velocity = ghost.direction * deltaTime * ghost.speed;
  ghost.position += ghost.velocity;
}

void Game::PhysicsUpdate(float deltaTime, glm::vec2 &desiredDir)
{
  gameTime += deltaTime;
  globalModeTimer += deltaTime;
  auto pacmanTile = pxToCell(pacman.position);
  for (auto &ghost : ghosts)
  {
    ghost.updateGhostMode(globalModeTimer);
    auto ghostTile = pxToCell(ghost.position);
    if (pacmanTile == ghostTile && ghost.mode != EATEN)
    {
      if (ghost.mode == FRIGHTENED)
      {
        printf("Pacman ate a ghost!\n");
        ghost.targetTile = ghost.housePosition;
        ghost.mode = EATEN;
      }
      else {
        printf("Ghost %c caught Pacman! Game Over!\n", ghost.ghostSymbol);
        Reset();
      }
    }
  }

  updatePacmanPhysics(deltaTime, desiredDir);
  for (auto &ghost : ghosts)
  {
    updateGhostPhysics(ghost, deltaTime);
  }
}

#endif