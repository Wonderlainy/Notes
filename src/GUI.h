#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <string>
#include <random>
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

std::string getRandomString(unsigned int lenght);
std::vector<std::string> split(const std::string& line, char delimiter);

struct Note {
    std::string name;
    std::string randomName;
    std::string password;
};

struct GUI {

    std::vector<Note> notes;
    unsigned int selected;

    std::string addFileName;
    std::string text;

    bool addMode;
    bool deleteMode;    bool sureToDelete;
    bool openMode;      bool openNote;


    bool kernelModified; bool saveChanges;

    void setFlagsFalse(bool* b);


    void loadKernel();
    void saveKernel();

    void action();

    void init();
    void draw();
};