#include "usbMsc.hpp"

/**
 * @brief Create the save directory for the mass storage data if it does not exist
 *
 * @return true if the dir exists or was created, otherwise false
 */
auto usbMSC::createSaveDir(std::string stringInputPath) -> bool
{
    auto result = mkdir(stringInputPath.c_str(), 0755);
    if(result == 0 || errno == EEXIST)
        return true;
    else
        return false;
}

/**
 * @brief get a list of all volumes
 * @note
 *
 * @return the string with the volumes
 */
auto usbMSC::getMsVolumes() -> std::string
{
    // Buffer for the command output
    // if buffer is too small the fget() have to be called multiple times
    std::array<char, 2'048> arrayBuffer;
    std::string stringParseResult;

    // Open and execute command in shell


#ifdef _WIN32
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen("wmic logicaldisk get volumeName", "r"), _pclose);
#elif __linux__
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("lsblk -o NAME,SIZE,FSTYPE,MOUNTPOINT", "r"), pclose);
#elif __APPLE__
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("find /Volumes -mindepth 1 -maxdepth 1 -type d", "r"), pclose);
#endif

    // Throw exception if volume listing failed
    try
    {
        if (!pipe)
            throw std::runtime_error("popen() failed!");
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    while (fgets(arrayBuffer.data(), arrayBuffer.size(), pipe.get()) != nullptr)
    {
        stringParseResult += arrayBuffer.data();
        std::cout << stringParseResult << std::endl; // Show Volumes in Terminal
    }

    return stringParseResult;
}
/**
 * @brief move files from mass storage to the save directory
 *
 * @return true if files are found
 */
auto usbMSC::copy(std::string stringTargetSavePath) -> bool
{
    // Create subfolder for saving the files
    if (setSavePath(stringTargetSavePath))
    {
        // move the files
        for (auto i = 1; i < MAX_FILE_COUNT; i++)
        {
            getFiles(getFile::CSV, i);
            getFiles(getFile::BMP, i);
        }
        return true;
    }
    else
        return false;
}
/**
 * @brief
 *
 * @tparam size
 * @param arrayBuffer
 */
template <std::size_t size>
auto usbMSC::getVolumePath(std::array<char, size> arrayBuffer) -> void
{
#ifdef _WIN32 | __linux__
    std::string stringBuffer(arrayBuffer.data(), arrayBuffer.size());
#endif

#ifdef _WIN32
    std::size_t volPos = stringBuffer.find(sOwonVolume);
    if (volPos != std::string::npos)
        stringVolumePath = stringBuffer.substr(volPos - 1, 2) + "\\";

#elif __linux__
    // on Linux you have to use std::string instead of std::array
    std::string sBuffer(arrayBuffer.data(), arrayBuffer.size());

    std::size_t volPos = stringBuffer.find(stringOwonVolume);
    if (volPos != std::string::npos)
    {
        std::size_t startPos = stringBuffer.rfind('\n', volPos) + 1;
        std::size_t endPos = stringBuffer.find(' ', startPos);
        stringVolumePath = stringBuffer.substr(startPos, endPos - startPos);
    }
#elif __APPLE__
    //
#endif
}


/**
 * @brief Before start copy the files, create the subfolder for saving the files
 * @param stringInputpath Inputpath is the target path passed by the user
 * @return true if the subfolder exists or was created
 */
auto usbMSC::setSavePath(std::string stringInputpath) -> bool
{
    mStringCurrentDate = getDate();
    mStringSavePath = stringInputpath + std::string(stringSaveDir) + "/" + mStringCurrentDate;

    return createSaveDir(mStringSavePath);
}
/**
 * @brief get the Date of today
 *
 * @return day as string
 */
auto usbMSC::getDate() -> std::string
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
    return ss.str();
}
/**
 * @brief Move the files from volume to the save directory
 *
 * @param type enum class fileTypes
 * @param fileNo used for the file number
 */
auto usbMSC::getFiles(getFile type, int fileNo) -> bool
{
    auto sourcePath = mStringVolumePath;
    switch (type)
    {
    case getFile::BMP:
        sourcePath += "/" + std::string("IMAGE") + std::to_string(fileNo) + ".BMP";
        break;
    case getFile::CSV:
        sourcePath += "/" + std::string("WAVE") + std::to_string(fileNo) + ".CSV";
        break;
    default:
        return false;
        break;
    }

    std::filesystem::copy(sourcePath, mStringSavePath, std::filesystem::copy_options::overwrite_existing);

    return true;

}
/**
 * @brief Check the inserted string whether it's a valid OWON volume
 *
 * @return
 */
auto usbMSC::checkVolumeString(std::string stringVolume) -> bool
{
    bool ret = false;

    // exit if the volume is a system volume
    // exit if the volume is to big
    //if (isSystemVolume(stringVolume) || isToBig(stringVolume))
    //    return ret;

    // exit if no CSV Files present
    return tryReadingCsvFiles(stringVolume) ? true : false;
}
auto usbMSC::isSystemVolume(std::string stringVolume) -> bool
{
#ifdef __APPLE__

    std::istringstream iss(stringVolume);
    std::vector<std::string> stringComponents;

    for (std::string s; getline(iss, s, '/');)
    {
        stringComponents.push_back(s);
    }

    for (auto &component : stringComponents)
    {
        if (component == "System")
            return true;
    }

#endif
}
/**
 * @brief Check the size whether it's fitting to the volume size of the owon internal storage
 *
 * @param stringVolume
 * @return
 */
auto usbMSC::isToBig(std::string stringVolume) -> bool
{
//using statvfs (struct which supplies informations about system) to get the volume size
#ifdef __APPLE__ | __linux__
    struct statvfs stat;
    if (statvfs(stringVolume.c_str(), &stat) != 0)
    {
        std::cerr << "Error: statvfs() failed for " << stringVolume << std::endl;
        return false;
    }
    auto tenMegabyte = 10 * 1024 * 1024;
    auto totalSize = stat.f_blocks * stat.f_frsize;

    return totalSize >= tenMegabyte ? true : false;

#endif
}
/**
 * @brief  Check if the volume contains CSV files of the OWON scope
 *
 * @param stringVolume
 * @return true if one of the files is present
 */
auto usbMSC::tryReadingCsvFiles(std::string stringVolume) -> bool
{
    if (std::filesystem::exists(stringVolume + "/WAVE1.CSV") || std::filesystem::exists(stringVolume + "/WAVE2.CSV") ||
        std::filesystem::exists(stringVolume + "/WAVE3.CSV") || std::filesystem::exists(stringVolume + "/WAVE4.CSV"))
    {
        mStringVolumePath = stringVolume;
        mIsVolumeFound = true;
        fileHandler _fileHandler;
        std::string targetPath = _fileHandler.getHomeDir();

        #ifdef __APPLE__ || __linux__
        targetPath += "/Documents/";
        #endif

        copy(targetPath);

        return true;
    }

    else
        return false;
}
