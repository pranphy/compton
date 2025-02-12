#ifndef __COMPTONSEARCHPATH_HH
#define __COMPTONSEARCHPATH_HH

/*!
    --------------------------------------------------------------------------------------------
    comptonSearchPath

    Header-only file to search for files inside
    the compton directory.
    --------------------------------------------------------------------------------------------
Usage:
    comptonSearchPath mySearchPath;
    mySearchPath.add("macros");  //< Adds the macros directory to the list of directories the next line will search for gui.mac in
    mySearchPath("gui.mac"); //< Returns the canonical path of gui.mac as a std::string

    The add() function by default adds CMAKE_INSTALL_PREFIX, CMAKE_INSTALL_FULL_DATADIR
    and the current working directory to the list of directories the operator() will search in.
    --------------------------------------------------------------------------------------------
*/

#include <filesystem>
namespace fs = std::filesystem;

#include <vector>
#include <string>

#include <unistd.h>

class comptonSearchPath
{
private:
    static comptonSearchPath* fInstance;
    std::vector<fs::path> fSearchPath;
    comptonSearchPath();

public:
    static comptonSearchPath* getInstance();
    virtual ~comptonSearchPath();
    void add(const std::string& path);
    std::string operator() (const std::string& filename);
    static std::string resolve(const std::string& filename) {
        return comptonSearchPath::getInstance()->operator()(filename);
    }
};

#endif //__COMPTONSEARCHPATH_HH
