#pragma once
#include "Screen.hpp"
#include "common.h"
#include <dirent.h>
#include <queue>
#include <array>

constexpr const char* ERROR_TYPES[] = {"DATA CORRUPTION ERROR", "MEDIA ERROR"};
constexpr const char* DEVICE_NAMES[] = {"mlc", "slc", "slccmpt", "usb", "odd"};

class CheckLogsScreen : public Screen {
public:
    CheckLogsScreen();
    ~CheckLogsScreen() override;

    void DrawSimpleText(const std::string &text) const;
    void DrawSimpleText(const std::vector<std::string> &texts) const;

    void Draw() override;

    bool Update(Input &input) override;

private:
    enum CheckState {
        CHECK_STATE_SELECT,
        CHECK_STATE_ERROR,
        CHECK_STATE_OPEN_DIR,
        CHECK_STATE_CHECK_FILE,
        CHECK_STATE_CHECKING_DONE,
    };
    CheckState mCheckState = CHECK_STATE_SELECT;

    const std::string mSourceDirectory = CRASH_LOG_SOURCE_PATH;
    DIR *mSourceDirectoryHandle        = nullptr;

    // enum ErrorTypes {
    //     ERROR_DATA_CORRUPTION,
    //     ERROR_MEDIA,
    //     ERROR
    // };

    //std::array<std::array<int, DEVICE_NAMES.size()>, ERROR_TYPES.size()> errorCount;

    int errorCount[std::size(DEVICE_NAMES)][std::size(ERROR_TYPES)] = {};

    //std::vector<std::vector<int> > errorCount(DEVICE_NAMES.size(), std::vector<int>(RROR_TYPES.size())); // Defaults to zero initial value
    
    std::queue<std::string> mFilesToCheck;

    const int mDefaultXPos     = 32;
    const int mDefaultYPos     = 128;
    const int mDefaultFontSize = 56;

    void CheckLog(std::string path);
};