#include "game.h"
#include <glm/gtc/matrix_transform.hpp>

// This just holds updateOrth().
// Nothing particularly fancy.

void Game::updateOrtho()
{
    const float halfWidth = windowWidth / 2.0f;
    const float halfHeight = windowHeight / 2.0f;
    this->projection = glm::ortho(-halfWidth * this->zoom, halfWidth * this->zoom,
        -halfHeight * this->zoom, halfHeight * this->zoom,
        0.1f, 1500.0f);
}
