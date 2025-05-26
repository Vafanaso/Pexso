#include <SFML/Graphics.hpp>
#include <ctime>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

class Card {
private:
  sf::RectangleShape shape;
  int value;
  bool isFlipped;
  bool isMatched;

public:
  Card(float x, float y, int val)
      : value(val), isFlipped(false), isMatched(false) {
    shape.setSize(sf::Vector2f(100, 100));
    shape.setFillColor(sf::Color::Green);
    shape.setPosition(x, y);
    shape.setOutlineThickness(2);
    shape.setOutlineColor(sf::Color::White);
  }

  void draw(sf::RenderWindow &window, const sf::Font &font) {
    window.draw(shape);
    if (isFlipped || isMatched) {
      sf::Text text(std::to_string(value), font, 32);
      text.setFillColor(sf::Color::Black);
      text.setPosition(shape.getPosition().x + 35, shape.getPosition().y + 30);
      window.draw(text);
    }
  }

  bool contains(sf::Vector2f point) const {
    return shape.getGlobalBounds().contains(point);
  }

  void flip() {
    if (!isMatched)
      isFlipped = !isFlipped;
  }

  bool isVisible() const { return isFlipped; }

  bool matched() const { return isMatched; }

  void setMatched() { isMatched = true; }

  int getValue() const { return value; }

  // Operator overloading for comparison
  bool operator==(const Card &other) const { return value == other.value; }
};

class Game {
private:
  std::vector<Card> cards;
  Card *firstCard = nullptr;
  Card *secondCard = nullptr;
  bool waiting = false;
  sf::Clock flipClock;

  // ✅ New score-related variables:
  int attempts = 0;
  int matches = 0;
  sf::Font font; // load only once

public:
  Game() {
    // Load font
    if (!font.loadFromFile("Roboto-Black.ttf"))
      throw std::runtime_error("Font not found");

    // Prepare values
    std::vector<int> values;
    for (int i = 1; i <= 8; ++i) {
      values.push_back(i);
      values.push_back(i);
    }

    std::shuffle(values.begin(), values.end(),
                 std::default_random_engine(static_cast<unsigned>(time(0))));

    int index = 0;
    for (int y = 0; y < 4; ++y) {
      for (int x = 0; x < 4; ++x) {
        cards.emplace_back(120 * x + 50, 120 * y + 100,
                           values[index++]); // moved down 100px
      }
    }
  }

  void handleClick(sf::Vector2f pos) {
    if (waiting && flipClock.getElapsedTime().asSeconds() < 1)
      return;

    if (waiting) {
      attempts++; // Count the attempt

      if (*firstCard == *secondCard) {
        firstCard->setMatched();
        secondCard->setMatched();
        matches++; // Count successful match
      } else {
        firstCard->flip();
        secondCard->flip();
      }
      firstCard = secondCard = nullptr;
      waiting = false;
    }

    for (auto &card : cards) {
      if (!card.matched() && card.contains(pos) && !card.isVisible()) {
        card.flip();
        if (!firstCard) {
          firstCard = &card;
        } else {
          secondCard = &card;
          waiting = true;
          flipClock.restart();
        }
        break;
      }
    }
  }

  void draw(sf::RenderWindow &window) {
    // Draw score info
    sf::Text scoreText("Attempts: " + std::to_string(attempts) +
                           " | Matches: " + std::to_string(matches),
                       font, 24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(10, 10);
    window.draw(scoreText);

    for (auto &card : cards) {
      card.draw(window, font); // Pass font to Card
    }
  }

  bool isGameWon() const {
    for (const auto &card : cards) {
      if (!card.matched())
        return false;
    }
    return true;
  }
};

int main() {
  sf::RenderWindow window(sf::VideoMode(600, 600), "Pexeso Game");
  Game game;

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window.close();

      if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2f clickPos =
            window.mapPixelToCoords(sf::Mouse::getPosition(window));
        try {
          game.handleClick(clickPos);
        } catch (std::exception &e) {
          std::cerr << "Error: " << e.what() << '\n';
        }
      }
    }

    window.clear();
    game.draw(window);

    if (game.isGameWon()) {
      sf::Font font;
      font.loadFromFile("Roboto-Black.ttf");
      sf::Text winText("You Win!", font, 48);
      winText.setFillColor(sf::Color::Green);
      winText.setPosition(180, 250);
      window.draw(winText);
    }

    window.display();
  }

  return 0;
}
