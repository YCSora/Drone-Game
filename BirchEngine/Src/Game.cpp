#include "Game.h"
#include <vector>
#include <cstdlib>  // for rand()
#include <ctime>    // for time()
#include "TextManager.h"
#include <windows.h>   
#include <string>
#include <iostream>


// Open the serial port using WinAPI (Windows only)
HANDLE openSerialPort(const char* portName, DWORD baudRate = CBR_9600) {
    // For COM3

    HANDLE hSerial = CreateFileA(
        portName,
        GENERIC_READ | GENERIC_WRITE,
        0,              // exclusive access
        nullptr,
        OPEN_EXISTING,
        0,              // no overlapped I/O for now
        nullptr
    );

    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening port " << portName
            << ", code: " << GetLastError() << "\n";
        return INVALID_HANDLE_VALUE;
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error getting comm state\n";
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }

    dcbSerialParams.BaudRate = baudRate;   //9600
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.Parity = NOPARITY;
    dcbSerialParams.StopBits = ONESTOPBIT;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error setting comm state\n";
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    SetCommTimeouts(hSerial, &timeouts);

    return hSerial;
}

// Send a text line over serial
bool sendString(HANDLE hSerial, const std::string& s) {
    if (hSerial == INVALID_HANDLE_VALUE) return false;

    DWORD bytesWritten = 0;
    BOOL ok = WriteFile(
        hSerial,
        s.c_str(),
        static_cast<DWORD>(s.size()),
        &bytesWritten,
        nullptr
    );

    if (!ok) {
        std::cerr << "WriteFile failed, code: " << GetLastError() << "\n";
        return false;
    }

    if (bytesWritten != s.size()) {
        std::cerr << "Partial write: " << bytesWritten
            << " of " << s.size() << " bytes\n";
        return false;
    }

    return true;
}

std::string Game::getRandomWord() { //creates a list of words to choose from
    static std::vector<std::string> wordList = {
        // easy
        "cat", "dog", "sun", "sky", "tree", "book", "fish", "blue", "fast", "kind",
        "home", "time", "wind", "ball", "fire", "rain", "bird", "song", "ship", "wave",

        // medium
        "window", "garden", "planet", "school", "friend", "yellow", "travel", "puzzle",
        "winter", "summer", "memory", "wonder", "forest", "bridge", "silver", "moment",
        "energy", "thunder", "pencil", "family",

        // hard
        "computer", "language", "adventure", "generation", "character", "beautiful",
        "education", "mechanism", "challenge", "direction", "developer", "artificial",
        "important", "different", "universe", "community", "knowledge", "perception",
        "reflection", "connection",

        // extreme
        "synchronization", "responsibility", "configuration", "communication",
        "representation", "understanding", "architecture", "experimentation",
        "interpretation", "implementation"
    };

    int index = rand() % wordList.size();  // pick a random number from 0 to (size - 1)
    return wordList[index];                // return the word at that index
}

SDL_Renderer* Game::renderer = nullptr;
GameObject* drone = nullptr;
SDL_Texture* txt = nullptr;

Game::Game() {}
Game::~Game() {}

void Game::init(const char* title, int width, int height, bool fullscreen) {
    int flags = fullscreen ? SDL_WINDOW_FULLSCREEN : 0;

    if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
        std::cout << "SDL Initialized!\n";

        if (TTF_Init() == -1) {
            std::cout << "Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
            isRunning = false;
            return;
        }

        window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            width, height, flags);
        renderer = SDL_CreateRenderer(window, -1, 0);

        if (renderer) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            std::cout << "Renderer created!\n";
        }

        serialHandle = openSerialPort("COM3", CBR_9600);
        std::cout << "serialHandle after open: " << serialHandle << "\n";

        if (serialHandle == INVALID_HANDLE_VALUE) {
            std::cerr << "Warning: Failed to open Arduino serial port.\n";
        }
        if (serialHandle != INVALID_HANDLE_VALUE) {
            Sleep(2000); // 2 seconds
            sendString(serialHandle, "START\n");
        }

        isRunning = true;
    }

    drone = new GameObject("assets/drone.png", 384, 536);

    SDL_StartTextInput();

    srand(static_cast<unsigned int>(time(nullptr)));
    targetWord = getRandomWord();
    std::cout << "Word: " << targetWord << std::endl;

    font = TextManager::LoadFont("assets/Roboto.ttf", 24);
    if (!font) {
        std::cerr << "Failed to load font.\n";
        isRunning = false;
        return;
    }
    targetWordTexture = TextManager::RenderText("Type: " + targetWord, font, textColor, renderer);
    currentInputTexture = TextManager::RenderText("", font, textColor, renderer);
}


void Game::handleEvents() {

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            isRunning = false;
            break;
        case SDL_TEXTINPUT:
            if (currentInput.length() < 30) {
                currentInput += event.text.text;

                // Destroy old texture if it exists
                if (currentInputTexture) SDL_DestroyTexture(currentInputTexture);

                // Re-render new texture each time user types
                currentInputTexture = TextManager::RenderText(currentInput, font, textColor, renderer);
                std::cout << "Typed: " << currentInput << std::endl;

                drone->resetIdleTimer();

            }
            break;

        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_BACKSPACE && !currentInput.empty()) {
                
                currentInput.pop_back();

                // Rebuild the input texture so it matches the new text
                if (currentInputTexture) SDL_DestroyTexture(currentInputTexture);

                currentInputTexture = TextManager::RenderText(
                    currentInput.empty() ? " " : currentInput,  
                    font, textColor, renderer
                );

            }
            else if (event.key.keysym.sym == SDLK_RETURN) {

                std::cout << "Final word: " << currentInput << std::endl;

                if (currentInput == targetWord) {

                    if (serialHandle != INVALID_HANDLE_VALUE) {
                        sendString(serialHandle, "CORRECT\n");
                    }

                    std::cout << "Correct!" << std::endl;

                    if (feedbackTexture) SDL_DestroyTexture(feedbackTexture);
                    feedbackTexture = TextManager::RenderText("Correct!", font, { 0, 255, 0, 255 }, renderer);

                    currentInput.clear();
                    targetWord = getRandomWord();

                    if (targetWordTexture) SDL_DestroyTexture(targetWordTexture);
                    targetWordTexture = TextManager::RenderText("Type: " + targetWord, font, textColor, renderer);
                    if (currentInputTexture) SDL_DestroyTexture(currentInputTexture);
                    currentInputTexture = TextManager::RenderText("", font, textColor, renderer);

                    std::cout << "Target word: " << targetWord << std::endl;

                    drone->acceleration(1.0f);
                }
                else {

                    std::cout << "Incorrect!" << std::endl;

                    if (feedbackTexture) SDL_DestroyTexture(feedbackTexture);
                    feedbackTexture = TextManager::RenderText("Incorrect!", font, { 255, 0, 0, 255 }, renderer);

                    if (serialHandle != INVALID_HANDLE_VALUE) {
                        sendString(serialHandle, "WRONG\n");  
                    }

                    currentInput.clear();
                    if (currentInputTexture) SDL_DestroyTexture(currentInputTexture);
                    currentInputTexture = TextManager::RenderText("", font, textColor, renderer);

                    drone->impulse(1.1f);
                }
            }
            break;
        }
    }
}


void Game::update() {
    drone->Update();
}

void Game::render() {
    SDL_RenderClear(renderer);

    if (drone) drone->Render();

    // Draws target words
    if (targetWordTexture) {
        SDL_Rect targetRect = { 20, 20, 0, 0 };
        TTF_SizeText(font, ("Type: " + targetWord).c_str(), &targetRect.w, &targetRect.h);
        SDL_RenderCopy(renderer, targetWordTexture, nullptr, &targetRect);
    }

    // Draws current input
    if (currentInputTexture) {
        SDL_Rect inputRect = { 20, 60, 0, 0 };
        TTF_SizeText(font, currentInput.c_str(), &inputRect.w, &inputRect.h);
        SDL_RenderCopy(renderer, currentInputTexture, nullptr, &inputRect);
    }

    // Draws feedback 
    if (feedbackTexture) {
        SDL_Rect feedbackRect = { 20, 100, 0, 0 };
        int w, h;
        TTF_SizeText(font, "Feedback", &w, &h);
        feedbackRect.w = w;
        feedbackRect.h = h;
        SDL_RenderCopy(renderer, feedbackTexture, nullptr, &feedbackRect);
    }



    SDL_RenderPresent(renderer);
}

void Game::sendStopSignal() {
    if (serialHandle != INVALID_HANDLE_VALUE) {
        sendString(serialHandle, "STOP\n");
    }
}

void Game::clean() {
    sendString(serialHandle, "STOP\n");

    if (targetWordTexture) SDL_DestroyTexture(targetWordTexture);
    if (currentInputTexture) SDL_DestroyTexture(currentInputTexture);
    if (feedbackTexture) SDL_DestroyTexture(feedbackTexture);
    if (font) TTF_CloseFont(font);

    if (serialHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(serialHandle);
        serialHandle = INVALID_HANDLE_VALUE;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_StopTextInput();
    TTF_Quit();
    SDL_Quit();
    std::cout << "Game cleaned.\n";
}

