/**
 * @file usbMsc.hpp
 * @author Alexander Tonn
 * @brief Get file data if owon device is as mass storage device connected
 * @version 0.1
 * @date 2024-03-05
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef USB_MSC_HPP
#define USB_MSC_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <array>
#include <memory>
#include <chrono>
#include <sstream>
#include <filesystem>
#include <cstdio>
#include <vector>
#include <sys/stat.h>

#include "fileHandler.hpp"

#ifdef __APPLE__ || __linux__
#include <sys/statvfs.h>
#endif

class usbMSC
{

public:
    auto getMsVolumes() -> std::string;
    auto copy(std::string stringTargetSavePath) -> bool;
    auto isVolumeFound() -> bool { return mIsVolumeFound; };
    auto checkVolumeString(std::string stringVolume) -> bool;

    private:
    auto isSystemVolume(std::string stringVolume) -> bool;
    auto isToBig(std::string stringVolume) -> bool;
    auto tryReadingCsvFiles(std::string stringVolume) -> bool;


private:
    constexpr static std::string_view stringSaveDir = "OWON";

    // create volume path
private:
    template <std::size_t size>
    auto getVolumePath(std::array<char, size> arrayBuffer) -> void;
    std::string mStringVolumePath;
    std::string mStringCurrentDate;
    std::string mStringSavePath;
    bool mIsVolumeFound;

private:
    auto createSaveDir(std::string stringInputPath) -> bool;
    enum class getFile
    {
        BMP,
        CSV,
        MOVE
    };
    auto getFiles(getFile type, int fileNo) -> bool;
    constexpr static uint8_t MAX_FILE_COUNT = 4;

// Create subfolder for saving the files
private:

    auto setSavePath(std::string stringInputpath) -> bool;
    auto getDate() -> std::string;

};

#endif // USB_MSC_HPP
