#pragma once
#include <cth/cth.hpp>


#include <string>

constexpr int PACKAGES_COUNT = 3;
constexpr std::array<std::string_view, PACKAGES_COUNT> packages{"yt-dlp", "ffmpeg", "chocolatey"};

constexpr std::string_view SETTINGS_FILE = "settings.txt";


//settings
enum Setting { SETTING_PATH_VIDEO, SETTING_PATH_AUDIO, SETTING_PATH_FFMPEG, SETTINGS_SIZE };
constexpr std::array<std::string_view, SETTINGS_SIZE> DEFAULT_SETTINGS{"videoPath=", "audioPath=", "ffmpegPath="};

void checkSettings(std::vector<std::string>& lines);
void setSetting(Setting setting, std::string_view content);
std::string getSetting(Setting setting);
int setPath(Setting setting);

//files
bool fileEmpty(std::string_view filepath, bool remove_if_empty);


//options and menus for commands
inline std::string cmdUrl{}, cmdQuality{}, cmdFps{};

int homeMenu();
int qualityMenu();
int urlMenu();
int fpsMenu();
int packagesUpdate();
void upgradePackageMenu(std::string_view package);
void installPackageMenu(std::string_view package);

int submit(int option);

//other
void displayOptions(std::vector<std::string> const& options, int start);
std::string getInput(std::vector<std::string> const& valid_inputs);