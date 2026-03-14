#include "dialogs.hpp"

/**
 * @brief draw the filebrowser
 *
 * @param fPath contains the complete path of selected item
 * @param sCurrent string path of the current file
 * @param sNew string path of the new file
 * @param xNotLoaded - true, if the file is not loaded
 * @param _fileHandler instance of fileHandler
 * @param option contentPathOption
 * @param page reference to currentPage
 * @note use the xNoatLoaded for proceeding the file once
 */
auto Dialogs::drawFilebrowser(std::filesystem::path &path,
                                  std::string &stringCurrent,
                                  std::string &stringNew,
                                  bool &notLoaded,
                                  fileHandler &_fileHandler,
                                  fileHandler::contentPathOption option,
                                  currentPage &page) -> void
{
    ImGui::OpenPopup(_guiTexts.lbl.at(languageSelection).dialogNames.at(0).c_str());

    if (ImGui::BeginPopupModal(_guiTexts.lbl.at(languageSelection).dialogNames.at(0).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        // go back in file path
        if (ImGui::Button(_guiTexts.btn.at(languageSelection).fileBrowser.at(0).c_str()))
        {
            // Check whether parent is available
            if (path.has_parent_path() && path.parent_path() != path.root_path())
                path = path.parent_path();
        }
        ImGui::Text("Current path: %s", path.string().c_str());
        ImGui::Separator();

        stringNew = _fileHandler.getContentOfPath(path, option);

        ImGui::Separator();
        // OK Btn pressed
        if (ImGui::Button(_guiTexts.btn.at(languageSelection).fileBrowser.at(1).c_str()))
        {
            stringCurrent = stringNew;
            notLoaded = false; // Allow to load new file
            page = currentPage::MAIN;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();

        // Cancel Btn pressed
        if (ImGui::Button(_guiTexts.btn.at(languageSelection).fileBrowser.at(2).c_str()))
        {
            page = currentPage::MAIN;
            ImGui::CloseCurrentPopup();
        }
    }
    ImGui::EndPopup();
}

/**
 * @brief pening a dialog which allow to decide window
 *
 */
auto Dialogs::drawChoiceWindow(std::string stringName, std::string stringQuestion) -> bool
{
    auto ret = false;
    ImGui::OpenPopup(stringName.c_str());

    if (ImGui::BeginPopupModal(stringName.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s", stringQuestion.c_str());
        ImGui::Spacing();

        // OK Button
        if (ImGui::Button(_guiTexts.btn.at(languageSelection).choiceWindow.at(0).c_str()))
        {
            ImGui::CloseCurrentPopup();
            ret = true;
        }
        // Cancel Button
        else if (ImGui::Button(_guiTexts.btn.at(languageSelection).choiceWindow.at(1).c_str()))
        {
            ImGui::CloseCurrentPopup();
            ret = false;
        }
    }
    ImGui::EndPopup();

    return ret;
}

/**
 * @brief draw the mass storage device selector
 * All available mass storage devices are listed here
 * if the selected device contiains WAVE.CSV file, it can be loaded
 * for preventing that a big drive will be checked, the maximum size is limited to 10MB
 */
auto Dialogs::drawMsDeviceSelector(usbMSC &_usbMSC, bool active) -> bool
{
    if(!active)
        return false;

    static bool volumeListPrinted = false;
    static std::string msVolumes;
    std::string stringError = "";
    if (!volumeListPrinted)
    {
        msVolumes = _usbMSC.getMsVolumes();
        volumeListPrinted = true;
    }

    // get the list of volumes and parse it
    std::vector<std::string> volumeList = parseMsVolumes(msVolumes);

    ImGui::OpenPopup(_guiTexts.lbl.at(languageSelection).dialogNames.at(1).c_str());

    if (ImGui::BeginPopupModal(_guiTexts.lbl.at(languageSelection).dialogNames.at(1).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s", _guiTexts.lbl.at(languageSelection).msSelector.at(0).c_str());
        ImGui::Separator();

            // List all available volumes
            for(auto &volume : volumeList)
            {
                // if item selected, check the entry
                if(ImGui::Selectable(volume.c_str()))
                {
                    if(!_usbMSC.checkVolumeString(volume))
                    {
                        stringError = _guiTexts.lbl.at(languageSelection).msSelector.at(1);
                    }
                }
            }

        ImGui::Separator();
        ImGui::NewLine();
        ImGui::Text("%s", stringError.c_str());

        // OK Button
        if (ImGui::Button(_guiTexts.btn.at(languageSelection).choiceWindow.at(0).c_str()))
        {
            volumeListPrinted = false;
            ImGui::CloseCurrentPopup();
        }
        // Cancel Button
        else if (ImGui::Button(_guiTexts.btn.at(languageSelection).choiceWindow.at(1).c_str()))
        {
            volumeListPrinted = false;
            ImGui::CloseCurrentPopup();
        }
    }
    ImGui::EndPopup();

    return volumeListPrinted;
}
/**
 * @brief Parse the whole string of volumes
 *
 * @param stringVolumes
 * @return
 */
auto Dialogs::parseMsVolumes(std::string stringVolumes) -> std::vector<std::string>
{
    std::vector<std::string> volumeList;
    std::istringstream iss(stringVolumes);

    #ifdef __APPLE__

    for (std::string line; std::getline(iss, line, '\n'); )
        volumeList.push_back(line);

    #endif

    return volumeList;
}
