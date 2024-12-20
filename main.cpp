#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const float HERO_SPEED = 200.0f;
const float PROJECTILE_SPEED = 400.0f;
const float ENEMY_SPEED = 100.0f;
const int BASE_HEALTH = 5;
const int HERO_PROJECTILE = 10;
const int ENEMY_SPAWN_INTERVAL = 2000;

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

    // Criar player
    sf::Texture playerSprite;
    if (!playerSprite.loadFromFile("img/Nave.png")) {
        return -1; // Erro ao carregar a textura
    }

    sf::Texture enemySprite;
    if (!enemySprite.loadFromFile("img/Enemy.png")) {
        return -1;
    }

    Entity hero;
    hero.sprite.setTexture(playerSprite);
    hero.sprite.setScale(2.0f, 2.0f);
    hero.sprite.setOrigin(playerSprite.getSize().x / 2.0f, playerSprite.getSize().y / 2.0f);
    hero.sprite.setPosition(480.0f, 270.0f);

    int heroprojetil = HERO_PROJECTILE;
    sf::Vector2f heroDestination = hero.sprite.getPosition();

    // Criar Base
    sf::RectangleShape base(sf::Vector2f(200.0f, 200.0f));
    base.setFillColor(sf::Color::Transparent);
    base.setOutlineColor(sf::Color::Green);
    base.setOutlineThickness(5.0f);
    base.setOrigin(100.0f, 100.0f);
    base.setPosition(480.0f, 270.0f);

    int baseHealth = BASE_HEALTH;

    // Inimigos e projéteis
    std::vector<Entity> enemies;
    std::vector<Projectile> projetil;
    std::vector<Projectile> enemyprojetil;

    sf::Clock enemySpawnClock;
    sf::Clock gameClock;

    std::srand(std::time(nullptr));

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Right) {
                    heroDestination = window.mapPixelToCoords({ event.mouseButton.x, event.mouseButton.y });
                }
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Q && heroprojetil > 0) {
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
                        projetil.push_back(projectile);
                        --heroprojetil;
                    }
                }
            }
        }

        float deltaTime = 1.0f / 60.0f;

        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        sf::Vector2f direction = mousePos - hero.sprite.getPosition();
        float angle = std::atan2(direction.y, direction.x) * 180.0f / M_PI;
        hero.sprite.setRotation(angle + 90.0f);

        moveEntity(hero, heroDestination, HERO_SPEED, deltaTime);

        for (auto& proj : projetil) {
            if (proj.active) {
                proj.shape.move(proj.velocity * deltaTime);
                if (!window.getViewport(window.getView()).contains(sf::Vector2i(proj.shape.getPosition()))) {
                    proj.active = false;
                }
            }
        }

        projetil.erase(std::remove_if(projetil.begin(), projetil.end(), [](const Projectile& p) { return !p.active; }), projetil.end());

        if (enemySpawnClock.getElapsedTime().asMilliseconds() >= ENEMY_SPAWN_INTERVAL) {
            enemySpawnClock.restart();
            Entity enemy;
            enemy.sprite.setTexture(enemySprite);
            enemy.sprite.setScale(1.8f, 1.8f);
            int side = std::rand() % 4;
            switch (side) {
            case 0: enemy.sprite.setPosition(std::rand() % 960, 0); break;
            case 1: enemy.sprite.setPosition(960, std::rand() % 540); break;
            case 2: enemy.sprite.setPosition(std::rand() % 960, 540); break;
            case 3: enemy.sprite.setPosition(0, std::rand() % 960); break;
                
            }
            enemies.push_back(enemy);
        }

        for (auto& enemy : enemies) {
            moveEntity(enemy, base.getPosition(), ENEMY_SPEED, deltaTime);

            // Check if the enemy collides with the base
            if (base.getGlobalBounds().intersects(enemy.sprite.getGlobalBounds())) 
            {
                if (enemy.active) {
                    baseHealth--;
                    enemy.active = false;
                }
            }
        }


        for (auto& proj : projetil) {
            if (proj.active) {
                for (auto& enemy : enemies) {
                    if (proj.shape.getGlobalBounds().intersects(enemy.sprite.getGlobalBounds())) {
                        enemy.active = false;
                        proj.active = false;
                        heroprojetil++;
                        break;
                    }
                }
            }
        }

        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](const Entity& e) { return !e.active; }), enemies.end());

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

        window.clear();
        window.draw(base);
        window.draw(hero.sprite);
        for (const auto& proj : projetil) {
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
