#include "main.hpp"

#include <fstream>
#include <functional>
#include <iostream>
#include <string>

#include <cth/io/file.hpp>
#include <Windows.h>


using namespace std;

//settings
void checkSettings(vector<string>& lines) {
    if(lines.size() >= SETTINGS_SIZE) return;
    lines.clear();
    for(auto setting : DEFAULT_SETTINGS) lines.emplace_back(setting);

}

void setSetting(Setting setting, string_view content) {
    auto lines = cth::io::file::chop(SETTINGS_FILE);
    checkSettings(lines);

    if(setting == SETTING_PATH_VIDEO) lines[SETTING_PATH_VIDEO] = std::format("{0}{1}", DEFAULT_SETTINGS[SETTING_PATH_VIDEO], content);
    else if(setting == SETTING_PATH_AUDIO) lines[SETTING_PATH_AUDIO] = std::format("{0}{1}", DEFAULT_SETTINGS[SETTING_PATH_AUDIO], content);
    else if(setting == SETTING_PATH_FFMPEG) lines[SETTING_PATH_FFMPEG] = std::format("{0}{1}", DEFAULT_SETTINGS[SETTING_PATH_FFMPEG], content);

    ofstream outFile(SETTINGS_FILE.data(), std::ios::trunc);
    for(auto& oLine : lines) outFile << oLine << '\n';
    outFile.close();
}

string getSetting(Setting setting) {
    if(!std::filesystem::exists(SETTINGS_FILE)) {
        std::filesystem::path const path{SETTINGS_FILE};
        ofstream{path};
    }

    auto lines = cth::io::file::chop(SETTINGS_FILE);
    checkSettings(lines);
    return lines[setting] <= DEFAULT_SETTINGS[setting] ? std::string{} :
           std::format(R"("{}")", lines[setting].substr(DEFAULT_SETTINGS[setting].size()));
}

int setPath(Setting setting) {
    system("cls");
    string current = getSetting(setting);
    println("current: {}", current.empty() ? "none" : current);
    displayOptions({"back", "default", "this dir"}, 0);

    if(setting == SETTING_PATH_FFMPEG) std::println("input ffmpeg path");
    else if(setting == SETTING_PATH_VIDEO) std::println("input path for video files");
    else if(setting == SETTING_PATH_AUDIO) std::println("input path for audio files");

    string input;
    cin >> input;
    if(input.length() == 1) {
        if(input == "0") return 0;
        if(input == "1") setSetting(setting, "");
        if(input == "2") setSetting(setting, std::filesystem::current_path().string());
    } else setSetting(setting, input);

    return 0;
}

//files
bool fileEmpty(std::string_view filepath, bool remove_if_empty) {
    ifstream file(filepath.data());
    bool const result = file.peek() == ifstream::traits_type::eof();
    file.close();
    if(remove_if_empty && result) std::filesystem::remove(filepath);
    return result;
}


//options and menus for commands
int homeMenu() {
    system("cls");
    std::println("This is a simple console yt-dlp gui :)");

    vector<string> const options = {
        "start",
        "set video output path",
        "set audio output path",
        "set ffmpeg path (only for audio files)",
    };

    displayOptions(options, 1);
    println("[0] exit");

    int const result = stoi(getInput({"0", "1", "2", "3", "4"}));
    if(result == 0) exit(EXIT_SUCCESS);
    if(result == 1) return urlMenu();
    if(result >= 2 && result <= 4) return setPath(static_cast<Setting>(result - 2));
    return 0;
}

int qualityMenu() {
    system("cls");
    std::println("\nchoose the video quality:");
    vector<string> const options{"back", "only audio", "2160p", "1440p", "1080p", "720p", "480p", "144p"};
    displayOptions(options, 0);

    if(int const option = stoi(getInput({"0", "1", "2", "3", "4", "5", "6", "7", "8"})); option == 0) return urlMenu();
    else {
        if(option == 1) return submit(1);
        cmdQuality = std::format("\"res: {}", options[option].substr(0, options[option].length() - 1));
    }

    return fpsMenu();
}
int urlMenu() {
    system("cls");
    println("\ninsert the video url:");
    println("[0] back");
    println("\ninput a value: ");

    string input;
    cin >> input;
    cmdUrl = std::format(R"("{}")", input);
    return input == "0" ? 0 : qualityMenu();
}
int fpsMenu() {
    system("cls");
    cout << '\n' << "choose fps:" << '\n';
    displayOptions({"back", "60 fps", "30 fps"}, 0);

    if(int const input = stoi(getInput({"0", "1", "2"})); input == 0) return qualityMenu();
    else if(input == 1) cmdFps = ",fps:60\"";
    else if(input == 2) cmdFps = ",fps:30\"";

    return submit(0);
}
int submit(int option) {
    system("cls");
    string cmd{"yt-dlp"};


    if(option == 0) {
        auto videoSetting = getSetting(SETTING_PATH_VIDEO);

        auto video = !videoSetting.empty() ? std::format("-P {}", videoSetting) : "";

        cmd += std::format(" -S {0} {1} -q --progress {2} {3}2>error.txt", cmdQuality, cmdFps, video, cmdUrl);
    } else if(option == 1) {
        auto ffmpegSetting = getSetting(SETTING_PATH_FFMPEG);
        auto audioSetting = getSetting(SETTING_PATH_AUDIO);


        auto const ffmpeg = !ffmpegSetting.empty() ? std::format("--ffmpeg-location {}", ffmpegSetting) : "";
        auto const audio = !audioSetting.empty() ? std::format("-P {}", audioSetting) : "";


        cmd += std::format(" -x --audio-quality 0 -q --progress --audio-format \"m4a\" {0} {1} {2}2>error.txt",
            audio, ffmpeg, cmdUrl);
    }
    std::println("{}", cmd);

    system(cmd.c_str());
    if(fileEmpty("error.txt", true)) println("\nfinished");
    else {
        println("\nFAILED");
        for(auto const lines = cth::io::file::chop("error.txt"); auto& line : lines) println("{}", line);
    }
    system("pause");
    remove("error.txt");
    return 0;
}
void installPackageMenu(std::string_view package) {

    std::println("\ninstall package {} now?", package);
    displayOptions({"no (quit)", "yes"}, 0);

    if(int const option = stoi(getInput({"0", "1"})); option == 0) exit(EXIT_FAILURE);
    else if(option == 1) system(std::format("choco install {}", package).c_str());
}
void upgradePackageMenu(std::string_view package) {
    std::println("\nupgrade ({}) now?", package);
    displayOptions({"no", "yes"}, 0);
    int const option = stoi(getInput({"0", "1"}));

    if(option == 0) return;

    if(option == 1) system(std::format("choco upgrade {}", package).c_str());
}


//other
string getInput(vector<string> const& valid_inputs) {
    string input;
    bool valid = false;
    do {
        cin >> input;
        for(auto& validInput : valid_inputs) if(input == validInput) valid = true;
        if(!valid) println("invalid input try again");
    } while(!valid);
    return input;
}
void displayOptions(vector<string> const& options, int start) {
    auto prefix = [&start]()-> string { return std::format(" [{}] ", to_string(start++)); };
    for(auto& option : options) std::println("{0}{1}", prefix(), option);
}



int main() {
    try {
        while(true) {
            homeMenu();
            system("cls");
        }
    }
    catch(std::exception const& e) {
        std::println("ERROR EXCEPTION ENCOUNTERED: {}", e.what());
        system("pause");
        exit(EXIT_SUCCESS);
    }
}
