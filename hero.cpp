#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const float HERO_SPEED = 200.0f;
const float PROJECTILE_SPEED = 400.0f;
const float ENEMY_SPEED = 100.0f;
const int BASE_HEALTH = 5;
const int HERO_HEALTH = 2;
const int HERO_PROJECTILE = 10;
const int ENEMY_SPAWN_INTERVAL = 2000;
const int RESPAWN_TIME = 5; // Respawn time in seconds

struct Entity {
    sf::Sprite sprite;
    sf::Vector2f velocity;
    bool active = true;
};

struct Projectile {
    sf::CircleShape shape;
    sf::Vector2f velocity;
    bool active = true;
};

void moveEntity(Entity& entity, const sf::Vector2f& destination, float speed, float deltaTime) {
    sf::Vector2f direction = destination - entity.sprite.getPosition();
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0.1f) {
        direction /= length;
        entity.velocity = direction * speed;
        entity.sprite.move(entity.velocity * deltaTime);
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(960, 540), "Base Defense");
    window.setFramerateLimit(60);

    // Load textures
    sf::Texture playerTexture, enemyTexture;
    if (!playerTexture.loadFromFile("img/Nave.png") || !enemyTexture.loadFromFile("img/Enemy.png")) {
        return -1; // Error loading textures
    }

    // Initialize hero
    Entity hero;
    hero.sprite.setTexture(playerTexture);
    hero.sprite.setScale(2.0f, 2.0f);
    hero.sprite.setOrigin(playerTexture.getSize().x / 2.0f, playerTexture.getSize().y / 2.0f);
    hero.sprite.setPosition(480.0f, 270.0f);

    int heroHealth = HERO_HEALTH;
    int heroProjectileCount = HERO_PROJECTILE;
    bool heroActive = true;
    sf::Vector2f heroDestination = hero.sprite.getPosition();

    // Create base
    sf::RectangleShape base(sf::Vector2f(200.0f, 200.0f));
    base.setFillColor(sf::Color::Transparent);
    base.setOutlineColor(sf::Color::Green);
    base.setOutlineThickness(5.0f);
    base.setOrigin(100.0f, 100.0f);
    base.setPosition(480.0f, 270.0f);

    int baseHealth = BASE_HEALTH;

    // Enemy and projectile storage
    std::vector<Entity> enemies;
    std::vector<Projectile> projectiles;

    sf::Clock enemySpawnClock;
    sf::Clock gameClock;
    sf::Clock respawnClock;

    std::srand(std::time(nullptr));

    // Respawn logic
    int respawnTimer = 0;

    // Load font for countdown display
    sf::Font font;
    if (!font.loadFromFile("fonts/arial.ttf")) {  // Ensure the font path is correct
        // Fallback: if font fails, display a message
        std::cerr << "Error: Failed to load font 'arial.ttf'. Ensure the file is in the correct directory.\n";
        return -1; // Terminate the application
    }

    sf::Text respawnText;
    respawnText.setFont(font);
    respawnText.setCharacterSize(50);
    respawnText.setFillColor(sf::Color::White);
    respawnText.setPosition(400, 250);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (heroActive && event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Right) {
                    heroDestination = window.mapPixelToCoords({ event.mouseButton.x, event.mouseButton.y });
                }
            }

            if (heroActive && event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Q && heroProjectileCount > 0) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                    sf::Vector2f direction = mousePos - hero.sprite.getPosition();
                    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                    if (length > 0.1f) {
                        direction /= length;
                        Projectile projectile;
                        projectile.shape.setRadius(5.0f);
                        projectile.shape.setFillColor(sf::Color::White);
                        projectile.shape.setOrigin(5.0f, 5.0f);
                        projectile.shape.setPosition(hero.sprite.getPosition());
                        projectile.velocity = direction * PROJECTILE_SPEED;
                        projectiles.push_back(projectile);
                        --heroProjectileCount;
                    }
                }
            }
        }

        float deltaTime = 1.0f / 60.0f;

        // Update hero movement and rotation
        if (heroActive) {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            sf::Vector2f direction = mousePos - hero.sprite.getPosition();
            float angle = std::atan2(direction.y, direction.x) * 180.0f / M_PI;
            hero.sprite.setRotation(angle + 90.0f);

            moveEntity(hero, heroDestination, HERO_SPEED, deltaTime);
        }

        // Update projectiles
        for (auto& proj : projectiles) {
            if (proj.active) {
                proj.shape.move(proj.velocity * deltaTime);
                if (!window.getViewport(window.getView()).contains(sf::Vector2i(proj.shape.getPosition()))) {
                    proj.active = false;
                }
            }
        }

        projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), [](const Projectile& p) { return !p.active; }), projectiles.end());

        // Spawn enemies
        if (enemySpawnClock.getElapsedTime().asMilliseconds() >= ENEMY_SPAWN_INTERVAL) {
            enemySpawnClock.restart();
            Entity enemy;
            enemy.sprite.setTexture(enemyTexture);
            enemy.sprite.setScale(1.8f, 1.8f);
            int side = std::rand() % 4;
            switch (side) {
            case 0: enemy.sprite.setPosition(std::rand() % 960, 0); break;
            case 1: enemy.sprite.setPosition(960, std::rand() % 540); break;
            case 2: enemy.sprite.setPosition(std::rand() % 960, 540); break;
            case 3: enemy.sprite.setPosition(0, std::rand() % 540); break;
            }
            enemies.push_back(enemy);
        }

        // Update enemies and check collisions
        for (auto& enemy : enemies) {
            if (enemy.active) {
                moveEntity(enemy, base.getPosition(), ENEMY_SPEED, deltaTime);

                // Check collision with base
                if (base.getGlobalBounds().intersects(enemy.sprite.getGlobalBounds())) {
                    baseHealth--;
                    enemy.active = false;
                }

                // Check collision with hero
                if (heroActive && hero.sprite.getGlobalBounds().intersects(enemy.sprite.getGlobalBounds())) {
                    heroHealth -= 2;
                    enemy.active = false;
                    if (heroHealth <= 0) {
                        heroActive = false;
                        respawnClock.restart();
                    }
                }
            }
        }

        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](const Entity& e) { return !e.active; }), enemies.end());

        // Respawn logic
        if (!heroActive) {
            respawnTimer = RESPAWN_TIME - static_cast<int>(respawnClock.getElapsedTime().asSeconds());
            if (respawnTimer <= 0) {
                heroActive = true;
                heroHealth = HERO_HEALTH;
                hero.sprite.setPosition(base.getPosition());
            }
            else {
                respawnText.setString("Respawning in: " + std::to_string(respawnTimer));
            }
        }

        // Update base color based on health
        if (baseHealth >= 3) {
            base.setOutlineColor(sf::Color::Green);
        }
        else if (baseHealth == 2) {
            base.setOutlineColor(sf::Color::Yellow);
        }
        else if (baseHealth == 1) {
            base.setOutlineColor(sf::Color::Red);
        }
        else {
            base.setOutlineColor(sf::Color::Transparent);
        }

        // Render
        window.clear();
        window.draw(base);
        if (heroActive) {
            window.draw(hero.sprite);
        }
        else {
            window.draw(respawnText);
        }
        for (const auto& proj : projectiles) {
            window.draw(proj.shape);
        }
        for (const auto& enemy : enemies) {
            window.draw(enemy.sprite);
        }
        window.display();

        if (baseHealth <= 0) {
            break;
        }
    }

    return 0;
}