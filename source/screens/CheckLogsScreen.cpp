#include "CheckLogsScreen.hpp"
#include "Gfx.hpp"
#include "common.h"
#include "utils/Utils.hpp"
#include <ctime>
#include <fstream>

CheckLogsScreen::CheckLogsScreen() {

}

CheckLogsScreen::~CheckLogsScreen() = default;


void CheckLogsScreen::DrawSimpleText(const std::string &text) const {
    Gfx::Print(Gfx::SCREEN_WIDTH / 2, Gfx::SCREEN_HEIGHT / 2, mDefaultFontSize, Gfx::COLOR_TEXT, text, Gfx::ALIGN_CENTER);
}

void CheckLogsScreen::DrawSimpleText(const std::vector<std::string> &items) const {
    int yOff = mDefaultYPos;
    for (const auto &item : items) {
        Gfx::Print(mDefaultXPos, yOff, mDefaultFontSize, Gfx::COLOR_TEXT, item, Gfx::ALIGN_VERTICAL);
        yOff += Gfx::GetTextHeight(mDefaultFontSize, item);
    }
}

void CheckLogsScreen::Draw() {
    DrawTopBar("Check Logs for Storage Device Errors");
    switch (mCheckState) {
        case CHECK_STATE_SELECT:
            DrawSimpleText("Press \ue000 to check logs for corruption errors");
            break;
        case CHECK_STATE_ERROR:
            DrawSimpleText("Checking failed. Press \ue001 to return");
            break;
        case CHECK_STATE_OPEN_DIR:
            DrawSimpleText("Preparing check");
            break;
        case CHECK_STATE_CHECK_FILE: {
            if (!mFilesToCheck.empty()) {
                std::string sourcePath = "slc:/sys/log/" + mFilesToCheck.front();
                std::string log        = Utils::sprintf("Checking %s", sourcePath.c_str());
                DrawSimpleText(log);
            } else {
                DrawSimpleText("Checking files...");
            }
            break;
        }

        case CHECK_STATE_CHECKING_DONE: {
            std::vector<std::string> text = { "Checking done! Found:" };
            for(size_t d=0; d<std::size(DEVICE_NAMES); d++){
                for(size_t e=0; e<std::size(ERROR_TYPES); e++){
                    if(errorCount[d][e]){
                        text.emplace_back(std::string(DEVICE_NAMES[d]) + ": " + ERROR_TYPES[e] + ": " + std::to_string(errorCount[d][e]));
                    }
                }
            }
            DrawSimpleText(text);
            break;
        }
    }

    DrawBottomBar(nullptr, "\ue044 Exit", "\ue001 Back");
}

void CheckLogsScreen::CheckLog(std::string path){
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line))
    {
        for(size_t e = 0; e < std::size(ERROR_TYPES); e++){
            size_t idx;
            if ((idx = line.find(ERROR_TYPES[e]))!= std::string::npos){
                idx = line.find("dev:", idx);
                for(size_t d = 0; d < std::size(DEVICE_NAMES); d++){
                    if(line.find(DEVICE_NAMES[d], idx + 4)!= std::string::npos) {
                        errorCount[d][e]++;
                        break;
                    }
                }
            }
        }
    }
}

bool CheckLogsScreen::Update(Input &input) {
    switch (mCheckState) {
        case CHECK_STATE_SELECT: {
            if (input.data.buttons_d & Input::BUTTON_B) {
                return false;
            } else if (input.data.buttons_d & Input::BUTTON_A) {
                mCheckState = CHECK_STATE_OPEN_DIR;
            }
            break;
        }
        case CHECK_STATE_OPEN_DIR:
            mSourceDirectoryHandle = opendir(mSourceDirectory.c_str());
            if (mSourceDirectoryHandle == nullptr) {
                mCheckState = CHECK_STATE_ERROR;
                break;
            }
            struct dirent *dp;
            while ((dp = readdir(mSourceDirectoryHandle)) != nullptr) {
                if (!std::string_view(dp->d_name).ends_with(".log")) {
                    continue;
                }
                std::string sourceFullPath = mSourceDirectory + "/" + dp->d_name;
                struct stat filestat {};
                if (stat(sourceFullPath.c_str(), &filestat) < 0 ||
                    (filestat.st_mode & S_IFMT) == S_IFDIR) {
                    continue;
                }
                mFilesToCheck.emplace(dp->d_name);
            }
            mCheckState = CHECK_STATE_CHECK_FILE;
            break;
        case CHECK_STATE_CHECK_FILE: {
            if (!mFilesToCheck.empty()) {
                std::string filename = mFilesToCheck.front();
                mFilesToCheck.pop();

                std::string sourceFullPath = mSourceDirectory + "/" + filename;
                CheckLog(sourceFullPath);

                break;
            } else {
                mCheckState = CHECK_STATE_CHECKING_DONE;
                break;
            }
        }
        case CHECK_STATE_CHECKING_DONE:
        case CHECK_STATE_ERROR:
            if (input.data.buttons_d & Input::BUTTON_B) {
                return false;
            }
            break;
    }

    return true;
}
