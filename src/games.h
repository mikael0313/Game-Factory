#ifndef GAMES_H
#define GAMES_H

class Game {
public:
    virtual ~Game() = default;
    virtual void run() = 0;
};

class SnakeGame : public Game {
public:
    void run() override;
};

class TicTacToeGame : public Game {
public:
    void run() override;
};

class GuessNumberGame : public Game {
public:
    void run() override;
};

class HangmanGame : public Game {
public:
    void run() override;
};

#endif // GAMES_H
