#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <iostream>
#include <SFML/Audio.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const float HERO_SPEED = 250.0f;
const float PROJECTILE_SPEED = 600.0f;
const float ENEMY_SPEED = 100.0f;
const int BASE_HEALTH = 5;
const int HERO_HEALTH = 2;
const int HERO_PROJECTILE = 10;
int ENEMY_SPAWN_INTERVAL = 2000;
const int RESPAWN_TIME = 5; // tempo de inatividade
const float SPAWN_ACCELERATION = 1.0f; // controla a dificuldade com o tempo
const float DROP_LIFETIME = 5.0f; // Tempo de vida de drops
int killCount = 0;


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

struct AmmoBag {
    sf::Sprite sprite;
    bool active = true;
    float timer = 0.0f;
};

struct HealBag {
    sf::Sprite sprite;
    bool active = true;
    float timer = 0.0f;
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

sf::Music backgroundMusic;

int main() {
    sf::RenderWindow window(sf::VideoMode(960, 540), "Base Defense");
    window.setFramerateLimit(60);

    // Música de fundo
    if (!backgroundMusic.openFromFile("sounds/soundtrack.wav")) {
        std::cerr << "Error: Failed to load soundtrack.\n";
        return -1; // Fecha se não encontrar
    }
    backgroundMusic.setVolume(40.f);
    backgroundMusic.setLoop(true);
    backgroundMusic.play();        

    // Efeitos Sonoros
    sf::SoundBuffer laserShootBuffer, ammoBuffer, healBuffer, deathBuffer, enemyDeadBuffer, baseHitBuffer, respawnBuffer, powerUpBuffer;
    if (!laserShootBuffer.loadFromFile("sounds/laserShoot.wav") ||
        !ammoBuffer.loadFromFile("sounds/ammo.wav") ||
        !healBuffer.loadFromFile("sounds/heal.wav") ||
        !deathBuffer.loadFromFile("sounds/death.wav") ||
        !enemyDeadBuffer.loadFromFile("sounds/enemydead.wav") ||
        !baseHitBuffer.loadFromFile("sounds/BaseHit.wav") ||
        !respawnBuffer.loadFromFile("sounds/respawn.wav") ||
        !powerUpBuffer.loadFromFile("sounds/powerUp.wav")) {
        std::cerr << "Error: Failed to load sound effects.\n";
        return -1; // Fecha se não encontrar
    }

    sf::Sound laserShootSound(laserShootBuffer);
    sf::Sound ammoSound(ammoBuffer);
    sf::Sound healSound(healBuffer);
    sf::Sound deathSound(deathBuffer);
    sf::Sound enemyDeadSound(enemyDeadBuffer);
    sf::Sound baseHitSound(baseHitBuffer);
    sf::Sound respawnSound(respawnBuffer);
    sf::Sound powerUpSound(powerUpBuffer);

    // Cria texturas
    sf::Texture playerTexture, enemyTexture, ammoTexture, healTexture, backgroundTexture;
    if (!playerTexture.loadFromFile("img/Nave.png") || !enemyTexture.loadFromFile("img/Enemy.png") || !ammoTexture.loadFromFile("img/ammo.png") || !healTexture.loadFromFile("img/life.png") || !backgroundTexture.loadFromFile("img/fundo.png")) {
        return -1; // Erro com as texturas
    }

    // Cria fundo
    sf::Sprite background;
    background.setTexture(backgroundTexture);

    // Inicia o player
    Entity hero;
    hero.sprite.setTexture(playerTexture);
    hero.sprite.setScale(2.0f, 2.0f);
    hero.sprite.setOrigin(playerTexture.getSize().x / 2.0f, playerTexture.getSize().y / 2.0f);
    hero.sprite.setPosition(480.0f, 270.0f);

    int heroHealth = HERO_HEALTH;
    int heroProjectileCount = HERO_PROJECTILE;
    bool heroActive = true;
    sf::Vector2f heroDestination = hero.sprite.getPosition();


    // Cria a base
    sf::CircleShape base(100.0f);
    base.setFillColor(sf::Color::Transparent);
    base.setOutlineColor(sf::Color::Green);
    base.setOutlineThickness(5.0f);
    base.setOrigin(100.0f, 100.0f);
    base.setPosition(480.0f, 270.0f);

    int baseHealth = BASE_HEALTH;

    // Guarda informações
    std::vector<Entity> enemies;
    std::vector<Projectile> projectiles;
    std::vector<AmmoBag> ammoBags;
    std::vector<HealBag> healBags;

    sf::Clock enemySpawnClock;
    sf::Clock gameClock;
    sf::Clock respawnClock;

    std::srand(std::time(nullptr));

    // Respawn
    int respawnTimer = 0;

    // Fonte customizada
    sf::Font font;
    if (!font.loadFromFile("fonts/arial.ttf")) {
        std::cerr << "Error: Failed to load font 'arial.ttf'. Ensure the file is in the correct directory.\n";
        return -1;
    }

    bool gameOver = false;  // Indica Game Over
    sf::Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(80);
    gameOverText.setFillColor(sf::Color::White);
    gameOverText.setPosition(350, 180);

    sf::Text respawnText;
    respawnText.setFont(font);
    respawnText.setCharacterSize(30);
    respawnText.setFillColor(sf::Color::White);
    respawnText.setPosition(400, 250);

    // HUD
    sf::Text ammoText;
    ammoText.setFont(font);
    ammoText.setCharacterSize(40);
    ammoText.setFillColor(sf::Color::White);
    ammoText.setPosition(10, 10);

    sf::Text killCountText;
    killCountText.setFont(font);
    killCountText.setCharacterSize(40);
    killCountText.setFillColor(sf::Color::White);
    killCountText.setPosition(10, 50);


    // Menu Inicial
    bool gameStarted = false;
    sf::Text menuText;
    menuText.setFont(font);
    menuText.setCharacterSize(40);
    menuText.setFillColor(sf::Color::White);
    menuText.setString("Base Defense\n\nProtect the MotherShip from the Alien Invasion!\n\nRMB to move\nMouse to Aim\nQ to Shoot\n\nPress SPACE to Start");
    menuText.setPosition(80, 50);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (!gameStarted && event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) {
                    gameStarted = true;
                }
            }

            if (gameStarted && event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Right) {
                    heroDestination = window.mapPixelToCoords({ event.mouseButton.x, event.mouseButton.y });
                }
            }

            if (gameStarted && event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Q && heroProjectileCount > 0 && heroActive) {
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
                        laserShootSound.play();
                        --heroProjectileCount;
                    }
                }
            }
        }

        float deltaTime = 1.0f / 60.0f;

        // Movimento e rotação do player
        if (gameStarted && heroActive) {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            sf::Vector2f direction = mousePos - hero.sprite.getPosition();
            float angle = std::atan2(direction.y, direction.x) * 180.0f / M_PI;
            hero.sprite.setRotation(angle + 90.0f);

            moveEntity(hero, heroDestination, HERO_SPEED, deltaTime);
        }

        // Update de projéteis e colisão
        for (auto& proj : projectiles) {
            if (proj.active) {
                proj.shape.move(proj.velocity * deltaTime);

                // colisão com inimigos
                for (auto& enemy : enemies) {
                    if (enemy.active && proj.shape.getGlobalBounds().intersects(enemy.sprite.getGlobalBounds())) {
                        proj.active = false;
                        enemy.active = false;

                        killCount++;

                        if (std::rand() % 100 < 50) {  // chance de dropar munição
                            AmmoBag ammoBag;
                            ammoBag.sprite.setTexture(ammoTexture);
                            ammoBag.sprite.setOrigin(ammoTexture.getSize().x / 2.0f, ammoTexture.getSize().y / 2.0f);
                            ammoBag.sprite.setPosition(enemy.sprite.getPosition());
                            ammoBags.push_back(ammoBag);
                        }

                        if (std::rand() % 100 < 10) {  // chance de dropar cura
                            HealBag healBag;
                            healBag.sprite.setTexture(healTexture);
                            healBag.sprite.setOrigin(healTexture.getSize().x / 2.0f, healTexture.getSize().y / 2.0f);
                            healBag.sprite.setPosition(enemy.sprite.getPosition());
                            healBags.push_back(healBag);
                        }

                        break;
                    }

                }

                if (!window.getViewport(window.getView()).contains(sf::Vector2i(proj.shape.getPosition()))) {
                    proj.active = false;
                }
            }
        }

        killCountText.setString("Kills: " + std::to_string(killCount));

        projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), [](const Projectile& p) { return !p.active; }), projectiles.end());

        // Spawn de inimigos e ajuste de dificuldade/tempo
        if (gameStarted && enemySpawnClock.getElapsedTime().asMilliseconds() >= ENEMY_SPAWN_INTERVAL) {
            enemySpawnClock.restart();

            // Gera o inimigo
            Entity enemy;
            enemy.sprite.setTexture(enemyTexture);
            enemy.sprite.setScale(1.8f, 1.8f);

            // Posiciona o inimigo
            int side = std::rand() % 4;
            switch (side) {
            case 0: enemy.sprite.setPosition(std::rand() % 960, 0); break;
            case 1: enemy.sprite.setPosition(960, std::rand() % 540); break;
            case 2: enemy.sprite.setPosition(std::rand() % 960, 540); break;
            case 3: enemy.sprite.setPosition(0, std::rand() % 540); break;
            }
            enemies.push_back(enemy);

            // gerenciador de dificuldade
            if (ENEMY_SPAWN_INTERVAL > 500) {
                ENEMY_SPAWN_INTERVAL -= SPAWN_ACCELERATION * deltaTime * 1000;
            }
        }

        // Update de inimigos e colisão
        for (auto& enemy : enemies) {
            if (enemy.active) {
                moveEntity(enemy, base.getPosition(), ENEMY_SPEED, deltaTime);

                // colisão com a base
                if (base.getGlobalBounds().intersects(enemy.sprite.getGlobalBounds())) {
                    baseHealth--;
                    baseHitSound.play();
                    enemy.active = false;
                }

                //colisão com player
                if (heroActive && hero.sprite.getGlobalBounds().intersects(enemy.sprite.getGlobalBounds())) {
                    heroHealth -= 2;
                    enemy.active = false;
                    if (heroHealth <= 0) {
                        deathSound.play();
                        heroActive = false;
                        respawnClock.restart();
                    }
                }
            }

            if (!enemy.active) {
                enemyDeadSound.play();
            }
        }

        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](const Entity& e) { return !e.active; }), enemies.end());

        // Update munição e interação com player
        for (auto& ammoBag : ammoBags) {
            if (ammoBag.active && hero.sprite.getGlobalBounds().intersects(ammoBag.sprite.getGlobalBounds())) {
                heroProjectileCount += 3;
                ammoSound.play();
                ammoBag.active = false;
            }
            ammoBag.timer += deltaTime;
            if (ammoBag.timer > DROP_LIFETIME) {
                ammoBag.active = false;
            }
        }

        ammoBags.erase(std::remove_if(ammoBags.begin(), ammoBags.end(), [](const AmmoBag& ab) { return !ab.active; }), ammoBags.end());

        // Update cura e interação com player
        for (auto& healBag : healBags) {
            if (healBag.active && hero.sprite.getGlobalBounds().intersects(healBag.sprite.getGlobalBounds())) {
                baseHealth += 1;
                healSound.play();
                healBag.active = false;
            }
            healBag.timer += deltaTime;
            if (healBag.timer > DROP_LIFETIME) {
                healBag.active = false;
            }
        }

        healBags.erase(std::remove_if(healBags.begin(), healBags.end(), [](const HealBag& hb) { return !hb.active; }), healBags.end());

        // Lógica de Respawn
        if (!heroActive) {
            respawnTimer = RESPAWN_TIME - static_cast<int>(respawnClock.getElapsedTime().asSeconds());
            if (respawnTimer <= 0) {
                heroActive = true;
                heroHealth = HERO_HEALTH;
                hero.sprite.setPosition(base.getPosition());
                respawnSound.play();
            }
            else {
                respawnText.setString("Respawning in: " + std::to_string(respawnTimer));
            }
        }

        // Medidor de vida da base
        if (baseHealth >= 3) {
            base.setOutlineColor(sf::Color::Green);
        }
        else if (baseHealth == 2) {
            base.setOutlineColor(sf::Color::Yellow);
        }
        else if (baseHealth == 1) {
            base.setOutlineColor(sf::Color::Red);
        }
        else if (baseHealth <= 0 && !gameOver) {
            gameOver = true;
            gameOverText.setString("Game Over\nKills: " + std::to_string(killCount));
        }

        ammoText.setString("Ammo: " + std::to_string(heroProjectileCount));

        // Gera frame
        window.clear();
        if (gameOver) {
            window.draw(gameOverText); // Tela de Game Over
            // Muta os sons já que na verdade a tela só ficou preta o jogo continua
            backgroundMusic.stop();
            laserShootSound.stop();
            baseHitSound.stop();
            enemyDeadSound.stop();
            deathSound.stop();
            respawnSound.stop();
        }
        else {
            if (!gameStarted) {
                window.draw(menuText); // Tela de Menu
            }
            else {
                // Geração do jogo
                window.draw(background);
                window.draw(base);
                if (heroActive) {
                    window.draw(hero.sprite);
                }
                for (auto& enemy : enemies) {
                    if (enemy.active) {
                        window.draw(enemy.sprite);
                    }
                }
                for (auto& proj : projectiles) {
                    if (proj.active) {
                        window.draw(proj.shape);
                    }
                }
                for (auto& ammo : ammoBags) {
                    if (ammo.active) {
                        window.draw(ammo.sprite);
                    }
                }
                for (auto& heal : healBags) {
                    if (heal.active) {
                        window.draw(heal.sprite);
                    }
                }
                // Gera HUD
                window.draw(ammoText);
                window.draw(killCountText);
                if (!heroActive) {
                    window.draw(respawnText);
                }
            }
        }
        window.display();
    }

    return 0;
}