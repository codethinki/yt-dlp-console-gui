#include <cth/cth.hpp>

#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <Windows.h>

using namespace std;
//#####################################################
//definitions
//#####################################################

//packages
constexpr int PACKAGES_COUNT = 1;
const std::array<string_view, PACKAGES_COUNT> packages{"yt-dlp"};

constexpr string_view SETTINGS_FILE = "settings.txt";

int packagesUpdate();
string extractVersion(const string& package_name);


//settings
enum Setting { SETTING_PATH_VIDEO, SETTING_PATH_AUDIO, SETTING_PATH_FFMPEG, SETTINGS_SIZE };
constexpr std::array<string_view, SETTINGS_SIZE> DEFAULT_SETTINGS{"videoPath=", "audioPath=", "ffmpegPath="};

void checkSettings(vector<string>& lines);
void setSetting(Setting setting, string_view content);
string getSetting(Setting setting);
int setPath(Setting setting);

//files
bool fileEmpty(const string& filepath, bool remove_if_empty);


//options and menus for commands
string cmdUrl{}, cmdQuality{}, cmdFps{};
int homeMenu();
int qualityMenu();
int urlMenu();
int fpsMenu();
int submit(int option);

//other
void displayOptions(const vector<string>& options, int start);
string getInput(const vector<string>& valid_inputs);

//#####################################################
//implementations
//#####################################################


//packages
int packagesUpdate() {

    system("choco upgrade yt-dlp");
    return 0;
}


void installPackageMenu(std::string_view package) {

    std::println("\ninstallpackage {} now?", package);
    displayOptions({"no (quit)", "yes"}, 0);

    if(const int option = stoi(getInput({"0", "1"})); option == 0) exit(EXIT_FAILURE);
    else if(option == 1) system(std::format("choco install {}", package).c_str());
}
void checkPackageInstallation(const string_view package) {
    constexpr string_view packageList = "package_list.txt";

    system(std::format("choco list>{}", packageList).c_str());
    const auto lines = cth::io::loadTxt(packageList);
    if(std::ranges::none_of(lines, [package](const string_view line) { return line.contains(package); })) {
        std::println("ERROR: package [{}] is not installed", package);
        installPackageMenu(package);
        system("cls");
    }
    remove(packageList.data());
}
void checkForPackages() {
    system("choco help>NUL 2>error.txt");
    if(!fileEmpty("error.txt", true)) {
        println("ERROR choco is not installed or not in PATH. Fix that first");
        remove("error.txt");
        exit(EXIT_FAILURE);
    }
    for(auto package : packages) checkPackageInstallation(package);
}

//settings
void checkSettings(vector<string>& lines) {
    if(lines.size() >= SETTINGS_SIZE) return;
    lines.clear();
    for(auto setting : DEFAULT_SETTINGS) lines.push_back(std::string(setting));

}
void setSetting(const Setting setting, string_view content) {
    auto lines = cth::io::loadTxt(SETTINGS_FILE.data());
    checkSettings(lines);

    if(setting == SETTING_PATH_VIDEO) lines[SETTING_PATH_VIDEO] = std::format("{0}{1}", DEFAULT_SETTINGS[SETTING_PATH_VIDEO], content);
    else if(setting == SETTING_PATH_AUDIO) lines[SETTING_PATH_AUDIO] = std::format("{0}{1}", DEFAULT_SETTINGS[SETTING_PATH_AUDIO], content);
    else if(setting == SETTING_PATH_FFMPEG) lines[SETTING_PATH_FFMPEG] = std::format("{0}{1}", DEFAULT_SETTINGS[SETTING_PATH_FFMPEG], content);

    ofstream outFile(SETTINGS_FILE.data(), std::ios::trunc);
    for(auto& oLine : lines) outFile << oLine << '\n';
    outFile.close();
}
string getSetting(const Setting setting) {
    auto lines = cth::io::loadTxt(SETTINGS_FILE.data());
    checkSettings(lines);
    return lines[setting] <= DEFAULT_SETTINGS[setting] ? R"("")" : std::format(R"("{}")", lines[setting].substr(DEFAULT_SETTINGS[setting].size()));
}
int setPath(const Setting setting) {
    system("cls");
    const string current = getSetting(setting);
    cout << "current: " << (current == "\"\"" ? "none" : current) << '\n';
    displayOptions({"back", "same dir"}, 0);

    if(setting == SETTING_PATH_FFMPEG) std::println("input ffmpeg path");
    else if(setting == SETTING_PATH_VIDEO) std::println("input path for video files");
    else if(setting == SETTING_PATH_AUDIO) std::println("input path for audio files");

    string input;
    cin >> input;
    if(input.length() == 1) {
        if(input == "0") return 0;
        if(input == "1") setSetting(setting, "");
    } else setSetting(setting, input);

    return 0;
}

//files
bool fileEmpty(const string& filepath, const bool remove_if_empty) {
    ifstream file(filepath);
    const bool result = file.peek() == ifstream::traits_type::eof();
    file.close();
    if(remove_if_empty && result) remove(filepath.c_str());
    return result;
}


//options and menus for commands
int homeMenu() {
    system("cls");
    std::println("This is a simple console yt-dlp gui :)");

    const vector<string> options = {"start",
        "set video output path",
        "set audio output path",
        "set ffmpeg path (only for audio files)",
        "update required packages",
    };

    displayOptions(options, 1);
    println("[0] exit");

    const int result = stoi(getInput({"0", "1", "2", "3", "4", "5"}));
    if(result == 0) exit(EXIT_SUCCESS);
    if(result == 1) return urlMenu();
    if(result >= 2 && result <= 4) return setPath(static_cast<Setting>(result - 2));
    if(result == 5) return packagesUpdate();
    return 0;
}
int qualityMenu() {
    system("cls");
    std::println("\nchoose the video quality:");
    const vector<string> options{"back", "only audio", "2160p", "1440p", "1080p", "720p", "480p", "144p"};
    displayOptions(options, 0);

    if(const int option = stoi(getInput({"0", "1", "2", "3", "4", "5", "6", "7", "8"})); option == 0) return urlMenu();
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

    if(const int input = stoi(getInput({"0", "1", "2"})); input == 0) return qualityMenu();
    else if(input == 1) cmdFps = ",fps:60\"";
    else if(input == 2) cmdFps = ",fps:30\"";

    return submit(0);
}
int submit(const int option) {
    system("cls");
    string cmd("yt-dlp");


    if(option == 0)
        cmd += std::format(" -S {0} {1} -q --progress -P {2} {3}2>error.txt", cmdQuality, cmdFps, getSetting(SETTING_PATH_VIDEO), cmdUrl);
    else if(option == 1) {
        const string ffmpegPath = getSetting(SETTING_PATH_FFMPEG);

        const string ffmpeg = ffmpegPath.empty() ? "" : std::format("--ffmpeg-location {} ", ffmpegPath);

        cmd += std::format(" -x --audio-quality 0 -q --progress --audio-format \"m4a\" -P {0} {1} {2}2>error.txt",
            getSetting(SETTING_PATH_AUDIO), ffmpeg, cmdUrl);
    }
    std::println("{}", cmd);

    system(cmd.c_str());
    if(fileEmpty("error.txt", true)) println("\nfinished");
    else {
        println("\nFAILED");
        for(const auto lines = cth::io::loadTxt("error.txt"); auto& line : lines) println("{}", line);
    }
    system("pause");
    remove("error.txt");
    return 0;
}


//other
string getInput(const vector<string>& valid_inputs) {
    string input;
    bool valid = false;
    do {
        cin >> input;
        for(auto& validInput : valid_inputs) if(input == validInput) valid = true;
        if(!valid) println("invalid input try again");
    } while(!valid);
    return input;
}
void displayOptions(const vector<string>& options, int start) {
    auto prefix = [&start]()-> string { return std::format(" [{}] ", to_string(start++)); };
    for(auto& option : options) std::println("{0}{1}", prefix(), option);
}



int main() {
    checkForPackages();
    while(true) {
        homeMenu();
        system("cls");
    }
}
