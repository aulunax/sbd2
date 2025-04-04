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
    "  help                                     Show this help message\n"
    "  help debug                               Show help relating to debug stuff (only for debugging)\n"
    "  setcompensation [true/false]             Toggle compensation on insert\n"
    "  clear                                    Clear all the files\n"
    "  rand [n]                                 Insert n random records to db\n"
    "  update [key] [value] [newKey: OPTIONAL]  Update record with given key\n"
    "  insert [key] [value]                     Insert record into the file\n"
    "  search [key]                             Search for record with given key\n"
    "  print [group] [all]                      Prints all records in db in order (or grouped by page)\n"
    "  loadtest [filename]                      Loads test commands from a file\n";

const std::string DEBUG_HELP_MESSAGE =
    "\n  List of available debug commands\n"
    "-------------------------------------------------------------------- \n"
    "  dblockstats                               Prints block stats\n"
    "  dforceflush                               Forces a flush of the files\n"
    "  dgetrecord [n]                            Gets a record at offset (data file)n\n";

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
    void clearFiles(const std::vector<std::string> &args);
    void printBtree(const std::vector<std::string> &args);
    void insertRecord(const std::vector<std::string> &args);
    void insertRandomRecords(const std::vector<std::string> &args);
    void searchRecord(const std::vector<std::string> &args);
    void toggleCompensation(const std::vector<std::string> &args);
    void updateRecord(const std::vector<std::string> &args);
    void removeRecord(const std::vector<std::string> &args);

    // debug commands
    void readRawRecordToFile(const std::vector<std::string> &args);
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