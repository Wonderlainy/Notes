#include "GUI.h"

std::string getRandomString(unsigned int lenght) {
    std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::random_device rd; // Obtain a random number from hardware
    std::mt19937 gen(rd()); // Seed the generator
    // Define the distribution for integers
    std::uniform_int_distribution<> dis_int(0, chars.size() - 1); // Range
    std::string rs = "";
    for(unsigned int i = 0; i < lenght; i++) {
        rs += chars[dis_int(gen)];
    }
    return rs;
}

std::vector<std::string> split(const std::string& line, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream stream(line);
    std::string token;
    
    while (std::getline(stream, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

void GUI::loadKernel() {
    std::filesystem::path filePath = "kernel";
    if (std::filesystem::exists(filePath)) {
        system("gpg --yes --no-symkey-cache --decrypt --output file kernel");

        std::ifstream file("file");
        if (!file.is_open()) {
            std::cerr << "Failed to open kernel file" << std::endl;
            return;
        }

        std::string line;
        while (getline(file, line)) {
            std::vector<std::string> tokens = split(line, ';');
            Note n;
            n.name = tokens[0];
            n.randomName = tokens[1];
            n.password = tokens[2];
            notes.push_back(n);
        }

        file.close();

        system("del file");
    }
}

void GUI::saveKernel() {
    std::ofstream file;
    file.open("file");

    // Check if the file was successfully opened
    if (!file) {
        std::cerr << "Failed to create the kernel file." << std::endl;
        return;
    }

    // Write data to the file
    for(unsigned int i = 0; i < notes.size(); i++) {
        file << notes[i].name + ";" + notes[i].randomName + ";" + notes[i].password + "\n";
    }

    // Close the file
    file.close();

    system("gpg --yes --no-symkey-cache --symmetric --cipher-algo AES256 --output kernel file");
    system("del file");
}

void GUI::init() {
    setFlagsFalse(nullptr);
    loadKernel();

    // Check if notes folder exists
    std::string path = "notes";
    if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
        std::cout << "notes folder exists." << std::endl;
    } else {
        std::cout << "notes folder does not exist." << std::endl;
        system("mkdir notes");
    }

    kernelModified = false;
    sureToDelete = false;
    saveChanges = false;
}

void GUI::setFlagsFalse(bool* b) {
    if (&addMode != b) {
        addMode = false;
    }
    if(&deleteMode != b) {
        deleteMode = false;
    }
    if(&openMode != b) {
        openMode = false;
    }
}

void GUI::action() {
    if(deleteMode) {
        sureToDelete = true;
    }
    if(openMode) {
        std::string command = "";
        command = "gpg --yes --batch --no-symkey-cache --decrypt --passphrase " + notes[selected].password + " --output file notes\\" + notes[selected].randomName;
        system(command.c_str());
        text = "";
        std::ifstream file("file");
        if (!file.is_open()) {
            std::cerr << "Failed to open file" << std::endl;
            return;
        }

        std::string line;
        while (getline(file, line)) {
            text += line + "\n";
        }

        file.close();
        system("del file");
        openNote = true;
    }
}

void GUI::draw() {
    ImGui::Begin("Notes");
    if(ImGui::Button("Add")) {
        setFlagsFalse(&addMode);
        addMode = !addMode;
        addFileName = "";
    }
    ImGui::SameLine();
    if(ImGui::Button("Open")) {
        setFlagsFalse(&openMode);
        openMode = !openMode;
    }
    ImGui::SameLine();
    if(ImGui::Button("Delete")) {
        setFlagsFalse(&deleteMode);
        deleteMode = !deleteMode;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    for(unsigned int i = 0; i < notes.size(); i++) {
        if(ImGui::Button(std::to_string(i).c_str())) {
            selected = i;
            action();
        }
        ImGui::SameLine();
        ImGui::Text(notes[i].name.c_str());
    }

    if(addMode) {
        ImGui::Begin("Add note");
        ImGui::Text("Name:");
        ImGui::SameLine();
        ImGui::InputText("##", &addFileName);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        if(ImGui::Button("Add")) {
            Note n;
            n.name = addFileName;
            n.randomName = getRandomString(10);
            n.password = getRandomString(30);
            notes.push_back(n);
            kernelModified = true;
            addMode = false;
            std::ofstream file;
            file.open("file");
            // Check if the file was successfully opened
            if (!file) {
                std::cerr << "Failed to create the file." << std::endl;
                return;
            }
            // Close the file
            file.close();
            std::string command;
            command = "gpg --yes --batch --no-symkey-cache --passphrase " + n.password + " --symmetric --cipher-algo AES256 --output notes\\" + n.randomName + " file";
            system(command.c_str());
            system("del file");
        }
        ImGui::SameLine();
        if(ImGui::Button("Close")) {
            addMode = false;
        }
        ImGui::End();
    }

    if(openNote) {
        ImGui::Begin(notes[selected].name.c_str());
        if(ImGui::Button("Save")) {
            std::ofstream file;
            file.open("file");
            // Check if the file was successfully opened
            if (!file) {
                std::cerr << "Failed to create the file." << std::endl;
                return;
            }
            file << text;
            // Close the file
            file.close();
            std::string command;
            command = "gpg --yes --batch --no-symkey-cache --passphrase " + notes[selected].password + " --symmetric --cipher-algo AES256 --output notes\\" + notes[selected].randomName + " file";
            system(command.c_str());
            system("del file");
        }
        ImGui::SameLine();
        if(ImGui::Button("Close")) {
            openNote = false;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::InputTextMultiline("##", &text, ImGui::GetContentRegionAvail());
        ImGui::End();
    }

    if(sureToDelete) {
        ImGui::Begin("Delete note");
        ImGui::Text("Are you sure that you want to delete this note?");
        if(ImGui::Button("Yes")) {
            std::string command;
            command = "del notes\\" + notes[selected].randomName;
            system(command.c_str());
            notes.erase(notes.begin() + selected);
            kernelModified = true;
            sureToDelete = false;            
        }
        ImGui::SameLine();
        if(ImGui::Button("No")) {
            sureToDelete = false;
        }
        ImGui::End();
    }

    if(kernelModified) {
        ImGui::Begin("Kernel modified");
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Unsaved changes");
        if(ImGui::Button("Save")) {
            saveKernel();
            kernelModified = false;
        }
        ImGui::End();
    }

    if(saveChanges) {
        ImGui::Begin("Save changes");
        ImGui::Text("Exit saving changes?");
        if(ImGui::Button("Yes")) {
            saveKernel();
            kernelModified = false;
        }
        ImGui::SameLine();
        if(ImGui::Button("Discard")) {
            kernelModified = false;
        }
        ImGui::End();
    }

    ImGui::End();
}