#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include <functional>
#include <time.h>
#include <random>
#include <memory>

#include "defines.h"
#include "BtreeHandler.h"

const std::string HELP_MESSAGE =
    "\n  List of available commands\n"
    "  TIP: Most commands can be used by using first letters of each word in the command\n"
    "  Example: 'am 5' is the same as 'addmulti 5' \n"
    "-------------------------------------------------------------------- \n"
    "  help                                   Show this help message\n"
    "  help debug                             Show help relating to debug stuff (only for debugging)\n"
    "  clear                                  Clear all the files\n"
    "  insert [key] [value]                   Insert record into the file\n"
    "  search [key]                           Search for record with given key\n"
    "  loadtest [filename]                    Loads test commands from a file\n";

const std::string DEBUG_HELP_MESSAGE =
    "\n  List of available debug commands\n"
    "-------------------------------------------------------------------- \n"
    "  dcleartest                                Clears the test file\n"
    "  dblockstats                               Prints block stats\n"
    "  drandrecord [n]                           Add n random records to a file\n"
    "  dforceflush                               Forces a flush of the files\n"
    "  dgetrecord [n]                            Gets a record at offset n\n";

class CommandLine
{
    std::unique_ptr<BtreeHandler> btreeHandler;

    bool quit = false;

    // helper command functions
    void printHelp();
    bool getYesNoAnswer();

    // main commands
    void handleHelp(const std::vector<std::string> &args);
    void loadTestFile(const std::vector<std::string> &args);
    void loadFile(const std::vector<std::string> &args);
    void saveFiles(const std::vector<std::string> &args);
    void clearFiles(const std::vector<std::string> &args);
    void printFiles(const std::vector<std::string> &args);
    void insertRecord(const std::vector<std::string> &args);
    void insertRandomRecords(const std::vector<std::string> &args);
    void insertRecordsRange(const std::vector<std::string> &args);
    void readRecord(const std::vector<std::string> &args);
    void searchRecord(const std::vector<std::string> &args);

    // debug commands
    void addRawRecordToFile(const std::vector<std::string> &args);
    void readRawRecordToFile(const std::vector<std::string> &args);
    void clearTestFile(const std::vector<std::string> &args);
    void printBlockStatistics(const std::vector<std::string> &args);
    void forceFlush(const std::vector<std::string> &args);

    // actual private functions
    bool isFileOpenedCorrectly(std::fstream &file);

    std::map<std::string, std::function<void(std::vector<std::string> &)>> commandsMap;

public:
    /// @brief Binds commands and aliases to the CLI instance
    /// @note Add new commands and aliases here.
    CommandLine();

    /// @brief Starts the CLI
    void handleInput(std::istream &input = std::cin, bool isTestFile = false);
};