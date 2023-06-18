#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include <fstream>
#include <ctime>

const int WORD_LENGTH = 5;
const int NOT_MATCH = 0;
const int PARTIAL_MATCH = 1;
const int MATCH = 2;
const int NUMBER_OF_TRIES = 30;
const int PORT = 12345;
const int NUMBER_OF_WORDS = 2499;
std::string dictionary[NUMBER_OF_WORDS];

std::string getRandomWord();
bool isValid(std::string word);
void toUpperCase(std::string &input);
void markMatch(std::vector<std::vector<int>> &matches, int tryIndex, std::string target, std::string guess);
bool isAllMatch(std::string target, std::string guess);
void printWordle(std::vector<std::string> tries, std::vector<std::vector<int>> matches, int currentTry);

int main(int argc, char **argv)
{
    if (argc != 1)
    {
        std::cout << "Wrong number of arguments!" << std::endl;
        return 1;
    }
    else
    {
        // Load dictionary
        std::fstream file;
        file.open("words.txt", std::ios::in); //open a file to perform read operation using file object
        if (file.is_open()){ //checking whether the file is open
            int i = 0;
            std::string tp;
            while (getline(file, tp)) { toUpperCase(tp); dictionary[i] = tp; i++; } //read data from file object and put it into string.
            file.close(); //close the file object.
        } else {
            std::cout << "Where is file with words?" << std::endl;
            return 2;
        }
        //////////////////
        ////////////////// Server start
        int client, server;
        char buffer[WORD_LENGTH];
        struct sockaddr_in server_addr;
        socklen_t size;
        
        client = socket(AF_INET, SOCK_STREAM, 0);

        if (client < 0) 
        {
            std::cerr << "Error establishing socket..." << std::endl;
            return -1;
        }

        std::cout << "=> Socket server has been created..." << std::endl;

        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htons(INADDR_ANY);
        server_addr.sin_port = htons(PORT);

        if ((bind(client, (struct sockaddr*)&server_addr,sizeof(server_addr))) < 0) 
        {
            std::cout << "=> Error binding connection, the socket has already been established..." << std::endl;
            return -1;
        }

        size = sizeof(server_addr);
        std::cout << "=> Looking for clients..." << std::endl;

        listen(client, 1);

        int clientCount = 1;
        server = accept(client, (struct sockaddr *)&server_addr, &size);

        if (server < 0) std::cout << "=> Error on accepting..." << std::endl;

        // strcpy(buffer, "=>.\n");
        // send(server, buffer, WORD_LENGTH, 0);
        // Print the IP address the server is listening on
        // char ip[INET_ADDRSTRLEN];
        // inet_ntop(AF_INET, &server_address.sin_addr, ip, INET_ADDRSTRLEN);
        // std::cout << "Server listening on IP address: " << ip << std::endl;
        //////////////////
        //////////////////

        std::vector<std::string> tries(NUMBER_OF_TRIES);
        std::vector<std::vector<int>> matches(NUMBER_OF_TRIES, std::vector<int>(WORD_LENGTH));
        std::string targetWord = getRandomWord();
        toUpperCase(targetWord);

        //////////////////
        //////////////////
        strcpy(buffer, targetWord.c_str());
        send(server, buffer, WORD_LENGTH, 0);
        //////////////////
        //////////////////

        bool play = true;
        std::string input;

        while (play) {
            int currentTry = 0;

            printWordle(tries, matches, -1);
            std::cout << std::endl;
            while (currentTry < NUMBER_OF_TRIES)
            {
                if (currentTry % 2 == 0) {
                    std::cout << "It's your turn!" << std::endl;
                    do
                    {
                        std::cout << "Please enter your guess (word length must be " + std::to_string(WORD_LENGTH) + ") or type Q to quit: ";
                        std::getline(std::cin, input);
                        toUpperCase(input);

                        if (isValid(input)) std::cout << "Wrong input!" << std::endl;
                        if (input == "IGVUP") {
                            std::cout << "Ehh.. The word was: " << targetWord << "!" << std::endl;
                            //////////////////
                            ////////////////// Server part (send)
                            // Send a word to the client
                            strcpy(buffer, input.c_str());
                            send(server, buffer, WORD_LENGTH, 0);
                            //////////////////
                            //////////////////
                            input = "Q";
                        }
                    } while (input != "Q" && !isValid(input));
                    if (input == "Q")
                    {
                        std::cout << "Quit game" << std::endl;
                        play = false;
                        break;
                    }
                    tries[currentTry] = input;

                    markMatch(matches, currentTry, targetWord, input);
                    printWordle(tries, matches, currentTry);
                    if (isAllMatch(targetWord, input))
                    {
                        std::cout << "Found the word" << std::endl;
                        //////////////////
                        ////////////////// Server part (send)
                        // Send a word to the client
                        strcpy(buffer, input.c_str());
                        send(server, buffer, WORD_LENGTH, 0);
                        //////////////////
                        //////////////////
                        break;
                    }
                    else if (currentTry == NUMBER_OF_TRIES)
                    {
                        std::cout << "You didn't find the word" << std::endl;
                    }

                    //////////////////
                    ////////////////// Server part (send)
                    // Send a word to the client
                    strcpy(buffer, input.c_str());
                    send(server, buffer, WORD_LENGTH, 0);
                    //////////////////
                    //////////////////
                } else {
                    std::cout << "It's not your turn!" << std::endl;

                    //////////////////
                    ////////////////// Server part (receive)
                    // Receive a word from client
                    recv(server, buffer, WORD_LENGTH, 0);
                    input.assign(buffer, WORD_LENGTH);
                    //////////////////
                    //////////////////

                    tries[currentTry] = input;

                    markMatch(matches, currentTry, targetWord, input);
                    printWordle(tries, matches, currentTry);
                    if (isAllMatch(targetWord, input))
                    {
                        std::cout << "Found the word" << std::endl;
                        break;
                    }
                    else if (input == "IGVUP") {
                        std::cout << "Ehh.. The word was: " << targetWord << "!" << std::endl;
                        break;
                    }
                    else if (currentTry == NUMBER_OF_TRIES)
                    {
                        std::cout << "You didn't find the word" << std::endl;
                    }
                }

                currentTry++;
            }
            std::cout << "End of game. Do you want to continue? Write 'Q' if don't: ";
            std::cin >> input;
            toUpperCase(input);
            if (input == "Q") {
                std::cout << "Bye!" << std::endl;
                break;
            }
        }

        //////////////////
        ////////////////// Server end
        // Close the sockets
        close(server);
        close(client);
        //////////////////
        //////////////////
    }

    return 0;
}

std::string getRandomWord()
{
    std::srand(std::time(nullptr));
    int wordPosition = std::rand()/((RAND_MAX + 1u) / NUMBER_OF_WORDS);
    return dictionary[wordPosition];
}

bool isValid(std::string word)
{
    bool inWords = std::find(std::begin(dictionary), std::end(dictionary), word) != std::end(dictionary);
    return word.length() == WORD_LENGTH && word.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ") == std::string::npos && inWords;
}

void toUpperCase(std::string &input)
{
    std::transform(input.begin(), input.end(), input.begin(), [](unsigned char c)
                   { return std::toupper(c); });
}

void markMatch(std::vector<std::vector<int>> &matches, int tryIndex, std::string target, std::string guess)
{
    for (int i = 0; i < guess.length(); i++)
    {
        matches[tryIndex][i] = NOT_MATCH;
    }
    for (int j = 0; j < guess.length(); j++)
    {
        for (int i = 0; i < target.length(); i++)
        {
            if (guess[j] == target[i])
            {
                if (i == j)
                {
                    matches[tryIndex][j] = MATCH;
                    break;
                }
                else
                {
                    matches[tryIndex][j] = PARTIAL_MATCH;
                }
            }
        }
    }
}

bool isAllMatch(std::string target, std::string guess)
{
    for (int i = 0; i < guess.length(); i++)
    {
        if (guess[i] != target[i])
            return false;
    }
    return true;
}

void printWordle(std::vector<std::string> tries, std::vector<std::vector<int>> matches, int currentTry)
{
    system("clear");
    std::cout << "=================================================================" << std::endl;
    std::cout << "|                                                               |" << std::endl;
    std::cout << "|   $$\\      $$\\                           $$\\ $$\\              |" << std::endl;
    std::cout << "|   $$ | $\\  $$ |                          $$ |$$ |             |" << std::endl;
    std::cout << "|   $$ |$$$\\ $$ | $$$$$$\\   $$$$$$\\   $$$$$$$ |$$ | $$$$$$\\     |" << std::endl;
    std::cout << "|   $$ $$ $$\\$$ |$$  __$$\\ $$  __$$\\ $$  __$$ |$$ |$$  __$$\\    |" << std::endl;
    std::cout << "|   $$$$  _$$$$ |$$ /  $$ |$$ |  \\__|$$ /  $$ |$$ |$$$$$$$$ |   |" << std::endl;
    std::cout << "|   $$$  / \\$$$ |$$ |  $$ |$$ |      $$ |  $$ |$$ |$$   ____|   |" << std::endl;
    std::cout << "|   $$  /   \\$$ |\\$$$$$$  |$$ |      \\$$$$$$$ |$$ |\\$$$$$$$\\    |" << std::endl;
    std::cout << "|   \\__/     \\__| \\______/ \\__|       \\_______|\\__| \\_______|   |" << std::endl;
    std::cout << "|                                                               |" << std::endl;
    std::cout << "=================================================================" << std::endl;
    for (int i = 0; i <= currentTry && i < tries.size(); i++)
    {
        std::string separator = "-";
        std::string padding = "|";
        std::string text = "|";
        for (int j = 0; j < tries[i].length(); j++)
        {
            separator += "------";
            padding += "     |";
            char value = std::toupper(tries[i][j]);
            text += "  ";
            if (matches[i][j] == PARTIAL_MATCH)
            {
                text += "\033[33m";
            }
            else if (matches[i][j] == MATCH)
            {
                text += "\033[32m";
            }
            text += value;
            if (matches[i][j] == PARTIAL_MATCH || matches[i][j] == MATCH)
            {
                text += "\033[0m";
            }
            text += "  |";
        }
        if (i == 0)
        {
            std::cout << separator << std::endl;
        }
        std::cout << padding << std::endl;
        std::cout << text << std::endl;
        std::cout << padding << std::endl;
        std::cout << separator << std::endl;
    }
}