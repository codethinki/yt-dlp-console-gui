#pragma once
#include <array>
#include <string>
#include <vector>

#include <cth/macro.hpp>

cxpr std::string_view SETTINGS_FILE = "settings.txt";


//settings
enum Setting { SETTING_PATH_VIDEO, SETTING_PATH_AUDIO, SETTING_PATH_FFMPEG, SETTINGS_SIZE };
cxpr std::array<std::string_view, SETTINGS_SIZE> DEFAULT_SETTINGS{"videoPath=", "audioPath=", "ffmpegPath="};

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

int submit(int option);

//other
void displayOptions(std::vector<std::string> const& options, int start);
std::string getInput(std::vector<std::string> const& valid_inputs);
