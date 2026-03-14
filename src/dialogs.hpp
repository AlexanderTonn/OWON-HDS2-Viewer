#ifndef DIALOGS_HPP
#define DIALOGS_HPP

#include <filesystem>
#include <string>
#include <vector>

#include "fileHandler.hpp"
#include "guiStrings.hpp"
#include "usbMSC.hpp"


class Dialogs
{


public:
    enum class currentPage
    {
        MAIN,
        OPEN_CSV_FILE,
        CHOOSE_MSC_PATH,
        CHOICE_WINDOW,
    };

    auto drawFilebrowser(std::filesystem::path &path,
                         std::string &stringCurrent,
                         std::string &stringNew,
                         bool &notLoaded,
                         fileHandler &_fileHandler,
                         fileHandler::contentPathOption option,
                         currentPage &page) -> void;

    auto drawChoiceWindow(std::string stringName, std::string stringQuestion) -> bool;

    auto drawMsDeviceSelector(usbMSC &_usbMSC, bool active) -> bool;
    private:
    auto parseMsVolumes(std::string stringVolumes) -> std::vector<std::string>;
};

#endif // DIALOGS_HPP
