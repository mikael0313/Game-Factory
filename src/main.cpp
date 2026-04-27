#include <iostream>
#include <limits>
#include <memory>
#include <cstdlib>
#include <ctime>
#include "games.h"

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    while (true) {
        std::cout << "=== Game Factory ===\n";
        std::cout << "Choose a game:\n";
        std::cout << "1) Snake Game\n";
        std::cout << "2) Tic-Tac-Toe\n";
        std::cout << "3) Guess the Number\n";
        std::cout << "4) Hangman\n";
        std::cout << "5) Exit\n";
        std::cout << "Enter choice: ";

        int choice;
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (!std::cin) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            choice = 0;
        }

        switch (choice) {
            case 1: {
                SnakeGame game;
                game.run();
                break;
            }
            case 2: {
                TicTacToeGame game;
                game.run();
                break;
            }
            case 3: {
                GuessNumberGame game;
                game.run();
                break;
            }
            case 4: {
                HangmanGame game;
                game.run();
                break;
            }
            case 5:
                std::cout << "Goodbye!\n";
                return 0;
            default:
                std::cout << "Please enter a number from 1 to 5.\n\n";
                break;
        }
    }
}
