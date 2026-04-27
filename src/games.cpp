#include "games.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <thread>
#include <termios.h>
#include <unistd.h>
#include <vector>

namespace {

void clearScreen() {
    std::system("clear");
}

struct RawTerminal {
    termios oldSettings;
    RawTerminal() {
        tcgetattr(STDIN_FILENO, &oldSettings);
        termios newSettings = oldSettings;
        newSettings.c_lflag &= ~(ICANON | ECHO);
        newSettings.c_cc[VMIN] = 0;
        newSettings.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);
    }
    ~RawTerminal() {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
    }
};

bool kbhit() {
    timeval tv{0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0;
}

char readChar() {
    char c = 0;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n == 1) {
        return c;
    }
    return 0;
}

int readInteger(const std::string &prompt, int minValue, int maxValue) {
    while (true) {
        std::cout << prompt;
        int value;
        std::cin >> value;
        if (std::cin.fail() || value < minValue || value > maxValue) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number between " << minValue << " and " << maxValue << ".\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return value;
    }
}

char readLetter(const std::string &prompt) {
    while (true) {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);
        if (!line.empty() && std::isalpha(static_cast<unsigned char>(line[0]))) {
            char letter = std::toupper(static_cast<unsigned char>(line[0]));
            return letter;
        }
        std::cout << "Please enter a letter (A-Z).\n";
    }
}

void pressEnterToContinue() {
    std::cout << "\nPress Enter to return to the menu...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

bool hasSnakeCollision(const std::vector<std::pair<int, int>> &snake, int x, int y) {
    for (const auto &segment : snake) {
        if (segment.first == x && segment.second == y) {
            return true;
        }
    }
    return false;
}

} // namespace

void SnakeGame::run() {
    const int width = 20;
    const int height = 12;
    std::vector<std::pair<int, int>> snake;
    snake.emplace_back(width / 2, height / 2);
    int dx = 1;
    int dy = 0;
    int score = 0;

    auto placeFood = [&]() {
        while (true) {
            int fx = std::rand() % (width - 2) + 1;
            int fy = std::rand() % (height - 2) + 1;
            if (!hasSnakeCollision(snake, fx, fy)) {
                return std::pair<int, int>{fx, fy};
            }
        }
    };

    auto [foodX, foodY] = placeFood();

    RawTerminal terminal;
    bool gameOver = false;
    bool exitGame = false;
    const auto frameDuration = std::chrono::milliseconds(200);

    while (!gameOver && !exitGame) {
        clearScreen();
        std::cout << "=== Snake Game ===\n";
        std::cout << "Score: " << score << "   (W/A/S/D to move, Q to quit)\n\n";

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                if (x == 0 || x == width - 1 || y == 0 || y == height - 1) {
                    std::cout << '#';
                    continue;
                }
                if (x == foodX && y == foodY) {
                    std::cout << '*';
                    continue;
                }
                bool printed = false;
                for (std::size_t i = 0; i < snake.size(); ++i) {
                    if (snake[i].first == x && snake[i].second == y) {
                        std::cout << (i == 0 ? 'O' : 'o');
                        printed = true;
                        break;
                    }
                }
                if (!printed) {
                    std::cout << ' ';
                }
            }
            std::cout << '\n';
        }

        if (kbhit()) {
            char key = readChar();
            if (key == 'w' || key == 'W') {
                if (dy != 1) {
                    dx = 0;
                    dy = -1;
                }
            } else if (key == 's' || key == 'S') {
                if (dy != -1) {
                    dx = 0;
                    dy = 1;
                }
            } else if (key == 'a' || key == 'A') {
                if (dx != 1) {
                    dx = -1;
                    dy = 0;
                }
            } else if (key == 'd' || key == 'D') {
                if (dx != -1) {
                    dx = 1;
                    dy = 0;
                }
            } else if (key == 'q' || key == 'Q') {
                exitGame = true;
                break;
            }
        }

        const auto &head = snake.front();
        int nextX = head.first + dx;
        int nextY = head.second + dy;

        if (nextX <= 0 || nextX >= width - 1 || nextY <= 0 || nextY >= height - 1 || hasSnakeCollision(snake, nextX, nextY)) {
            gameOver = true;
            break;
        }

        snake.insert(snake.begin(), std::make_pair(nextX, nextY));
        if (nextX == foodX && nextY == foodY) {
            score += 10;
            std::tie(foodX, foodY) = placeFood();
        } else {
            snake.pop_back();
        }

        std::this_thread::sleep_for(frameDuration);
    }

    clearScreen();
    if (exitGame) {
        std::cout << "Exiting Snake game. Score: " << score << "\n";
    } else {
        std::cout << "Game Over! Final score: " << score << "\n";
    }
    pressEnterToContinue();
}

void TicTacToeGame::run() {
    std::vector<char> board(9, ' ');
    char player = 'X';

    auto printBoard = [&]() {
        clearScreen();
        std::cout << "=== Tic-Tac-Toe ===\n";
        for (int i = 0; i < 9; ++i) {
            std::cout << " " << (board[i] == ' ' ? std::to_string(i + 1) : std::string(1, board[i]));
            if ((i + 1) % 3 == 0) {
                std::cout << "\n";
                if (i < 8) {
                    std::cout << "---+---+---\n";
                }
            } else {
                std::cout << " |";
            }
        }
    };

    auto checkWinner = [&](char mark) {
        static const int wins[8][3] = {
            {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
            {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
            {0, 4, 8}, {2, 4, 6}
        };
        for (auto &line : wins) {
            if (board[line[0]] == mark && board[line[1]] == mark && board[line[2]] == mark) {
                return true;
            }
        }
        return false;
    };

    bool finished = false;
    while (!finished) {
        printBoard();
        int choice = readInteger("Choose a cell (1-9): ", 1, 9) - 1;
        if (board[choice] != ' ') {
            std::cout << "Cell already taken. Try again.\n";
            continue;
        }

        board[choice] = player;
        if (checkWinner(player)) {
            printBoard();
            std::cout << "Player " << player << " wins!\n";
            finished = true;
        } else if (std::none_of(board.begin(), board.end(), [](char c) { return c == ' '; })) {
            printBoard();
            std::cout << "It's a tie!\n";
            finished = true;
        } else {
            player = (player == 'X' ? 'O' : 'X');
        }
    }
    pressEnterToContinue();
}

void GuessNumberGame::run() {
    clearScreen();
    std::cout << "=== Guess the Number ===\n";
    int secret = std::rand() % 100 + 1;
    int attempts = 0;
    bool guessed = false;

    while (!guessed) {
        int guess = readInteger("Guess a number between 1 and 100: ", 1, 100);
        ++attempts;

        if (guess < secret) {
            std::cout << "Too low!\n";
        } else if (guess > secret) {
            std::cout << "Too high!\n";
        } else {
            std::cout << "Correct! You guessed it in " << attempts << " attempts.\n";
            guessed = true;
        }
    }
    pressEnterToContinue();
}

void HangmanGame::run() {
    clearScreen();
    std::cout << "=== Hangman ===\n";

    const std::vector<std::string> words = {
        "COMPUTER", "FUNNY", "PROGRAM", "PUZZLE", "ANSWER",
        "GITHUB", "LANGUAGE", "CONSOLE", "TUTORIAL", "MODEL"
    };

    std::string secret = words[std::rand() % words.size()];
    std::string display(secret.size(), '_');
    std::vector<char> guessedLetters;
    int remainingGuesses = 6;
    bool solved = false;

    while (remainingGuesses > 0 && !solved) {
        clearScreen();
        std::cout << "=== Hangman ===\n";
        std::cout << "Word: ";
        for (char c : display) {
            std::cout << c << ' ';
        }
        std::cout << "\n";
        std::cout << "Guessed: ";
        for (char c : guessedLetters) {
            std::cout << c << ' ';
        }
        std::cout << "\n";
        std::cout << "Remaining wrong guesses: " << remainingGuesses << "\n";

        char guess = readLetter("Enter a letter: ");
        if (std::find(guessedLetters.begin(), guessedLetters.end(), guess) != guessedLetters.end()) {
            std::cout << "You already guessed " << guess << ". Try again.\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        guessedLetters.push_back(guess);
        bool hit = false;
        for (std::size_t i = 0; i < secret.size(); ++i) {
            if (secret[i] == guess) {
                display[i] = guess;
                hit = true;
            }
        }

        if (!hit) {
            --remainingGuesses;
            std::cout << "Wrong guess!\n";
        } else {
            std::cout << "Nice!\n";
        }

        if (display == secret) {
            solved = true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(800));
    }

    clearScreen();
    if (solved) {
        std::cout << "You win! The word was: " << secret << "\n";
    } else {
        std::cout << "Game over! The word was: " << secret << "\n";
    }
    pressEnterToContinue();
}
