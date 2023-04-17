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
constexpr int packagesCount = 2;
const string packages[packagesCount]{"yt-dlp", "wheel"};

int packagesUpdate();
int packagesVersion();
string extractVersion(const string& package_name);


//settings
enum Setting { SETTING_PATH_VIDEO, SETTING_PATH_AUDIO, SETTING_PATH_FFMPEG, SETTINGS_SIZE };
const string defaultSettings[SETTINGS_SIZE]{"videoPath=", "audioPath=", "ffmpegPath="};

void checkSettings(vector<string>& lines);
void setSetting(Setting setting, const string& content);
string getSetting(Setting setting);
int setPath(Setting setting);

//files
bool fileEmpty(const string& filepath, bool remove_if_empty);
vector<string> loadFile(const string& filename);


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
    system("python.exe -m pip install --upgrade pip");
    system("pip install wheel --upgrade");
    system("pip install yt-dlp --upgrade");
    return 0;
}
string extractVersion(const string& package_name) {
    cout << package_name << " version" << '\n';
    system(("pip show " + package_name + ">temp.txt").c_str());
    const auto lines = loadFile("temp.txt");
    remove("temp.txt");
    return lines[1].substr(9);
}
int packagesVersion() {
    cout << extractVersion("yt-dlp") << '\n' << '\n';
    cout << extractVersion("wheel") << '\n' << '\n';
    system("pause");
    return 0;
}
void installPackageMenu(const string& package) {
    cout << '\n' << "install package " << package << " now?" << '\n';
    displayOptions({"no (quit)", "yes"}, 0);

    if(const int option = stoi(getInput({"0", "1"})); option == 0) exit(EXIT_FAILURE);
    else if(option == 1) system(("pip install" + package).c_str());
}
void checkPackageInstallation(const string& package) {
    system(("pip show " + package + ">NUL 2>error.txt").c_str());
    if(!fileEmpty("error.txt", true)) {
        cout << "ERROR: " + package + " is not installed" << '\n';
        installPackageMenu(package);
        system("cls");
    }
    remove("error.txt");
}
void checkForPackages() {
    system("pip help>NUL 2>error.txt");
    if(!fileEmpty("error.txt", true)) {
        cout << "ERROR: pip is not installed or not in PATH. Fix that first" << '\n';
        remove("error.txt");
        exit(EXIT_FAILURE);
    }
    for(auto& package : packages) checkPackageInstallation(package);
}

//settings
void checkSettings(vector<string>& lines) {
    if(lines.size() < SETTINGS_SIZE) {
        lines.clear();
        for(auto& setting : defaultSettings) lines.push_back(setting);
    }
}
void setSetting(const Setting setting, const string& content) {
    auto lines = loadFile("settings.txt");
    checkSettings(lines);

    if(setting == SETTING_PATH_VIDEO) lines[SETTING_PATH_VIDEO] = defaultSettings[SETTING_PATH_VIDEO] + content;
    else if(setting == SETTING_PATH_AUDIO) lines[SETTING_PATH_AUDIO] = defaultSettings[SETTING_PATH_AUDIO] + content;
    else if(setting == SETTING_PATH_FFMPEG) lines[SETTING_PATH_FFMPEG] = defaultSettings[SETTING_PATH_FFMPEG] + content;

    ofstream outFile("settings.txt", std::ios::trunc);
    for(auto& oLine : lines) outFile << oLine << '\n';
    outFile.close();
}
string getSetting(const Setting setting) {
    auto lines = loadFile("settings.txt");
    checkSettings(lines);
    return lines[setting] <= defaultSettings[setting] ? "\"\"" : '\"' + lines[setting].substr(defaultSettings[setting].size()) + '\"';
}
int setPath(const Setting setting) {
    system("cls");
    const string current = getSetting(setting);
    cout << "current: " << (current == "\"\"" ? "none" : current) << '\n';
    displayOptions({"back", "same dir"}, 0);

    if(setting == SETTING_PATH_FFMPEG) cout << "input ffmpeg path" << '\n';
    else if(setting == SETTING_PATH_VIDEO) cout << "input path for video files" << '\n';
    else if(setting == SETTING_PATH_AUDIO) cout << "input path for audio files" << '\n';

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
vector<string> loadFile(const string& filename) {
    ifstream file(filename);
    vector<string> lines;
    string line;
    while(getline(file, line)) lines.push_back(line);
    file.close();
    return lines;
}


//options and menus for commands
int homeMenu() {
    system("cls");
    cout << "This is a simple console yt-dlp gui :)" << '\n';

    const vector<string> options = {"start",
        "set video output path",
        "set audio output path",
        "set ffmpeg path (only for audio files)",
        "update required packages",
        "versions of required packages"};

    displayOptions(options, 1);
    cout << "[0] exit" << '\n';

    const int result = stoi(getInput({"0", "1", "2", "3", "4", "5", "6"}));
    if(result == 0) exit(EXIT_SUCCESS);
    if(result == 1) return urlMenu();
    if(result >= 2 && result <= 4) return setPath(static_cast<Setting>(result - 2));
    if(result == 5) return packagesUpdate();
    if(result == 6) return packagesVersion();
    return 0;
}
int qualityMenu() {
    system("cls");
    cout << '\n' << "choose the video quality:" << '\n';
    const vector<string> options{"back", "only audio", "2160p", "1440p", "1080p", "720p", "480p", "144p"};
    displayOptions(options, 0);

    if(const int option = stoi(getInput({"0", "1", "2", "3", "4", "5", "6", "7", "8"})); option == 0) return urlMenu();
    else {
        if(option == 1) return submit(1);
        cmdQuality = "\"res:" + options[option].substr(0, options[option].length() - 1);
    }

    return fpsMenu();
}
int urlMenu() {
    system("cls");
    cout << '\n' << "insert the video url:" << '\n';
    cout << "[0] back" << '\n';
    cout << '\n' << "input a value:" << '\n';
    string input;
    cin >> input;
    cmdUrl = "\"" + input + "\"";
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
        cmd += " -S " + cmdQuality + cmdFps + " -q --progress -P " + getSetting(SETTING_PATH_VIDEO) + " " + cmdUrl +
            "2>error.txt";
    else if(option == 1) {
        const string ffmpegPath = getSetting(SETTING_PATH_FFMPEG);
        cmd += " -x --audio-quality 0 -q --progress --audio-format \"m4a\" -P " + getSetting(SETTING_PATH_AUDIO) + ' ' +
            (ffmpegPath.empty() ? "" : "--ffmpeg-location " + ffmpegPath + ' ') + cmdUrl + "2>error.txt";
    }
    cout << cmd << '\n';

    system(cmd.c_str());
    if(fileEmpty("error.txt", true)) cout << '\n' << "finished" << '\n';
    else {
        cout << '\n' << "FAILED:" << '\n';
        for(const auto lines = loadFile("error.txt"); auto& line : lines) cout << line << '\n';
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
        if(!valid) cout << "invalid input try again" << '\n';
    } while(!valid);
    return input;
}
void displayOptions(const vector<string>& options, int start) {
    auto prefix = [&start]()->string { return "[" + to_string(start++) + "] "; };
    for(auto& option : options) cout << prefix() << option << '\n';
    cout << '\n';
}



int main() {
    checkForPackages();
    while(true) {
        homeMenu();
        system("cls");
    }
}
