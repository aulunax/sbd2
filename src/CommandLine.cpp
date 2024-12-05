#include "CommandLine.h"
#include "RecordBlockIO.h"
#include <iostream>

CommandLine::CommandLine()
{
    srand(time(NULL));

    btreeHandler = std::make_unique<BtreeHandler>(INDEXFILE_FILENAME, DATAFILE_FILENAME);

    commandsMap["help"] = [this](const std::vector<std::string> &args)
    { handleHelp(args); };
    commandsMap["h"] = commandsMap["help"];
    commandsMap["?"] = commandsMap["help"];

    commandsMap["quit"] = [this](const std::vector<std::string> &args)
    { quit = true; };
    commandsMap["q"] = commandsMap["quit"];

    commandsMap["loadtest"] = [this](const std::vector<std::string> &args)
    { loadTestFile(args); };
    commandsMap["lt"] = commandsMap["loadtest"];

    commandsMap["drandrecord"] = [this](const std::vector<std::string> &args)
    { addRawRecordToFile(args); };
    commandsMap["drr"] = commandsMap["drandrecord"];

    commandsMap["dgetrecord"] = [this](const std::vector<std::string> &args)
    { readRawRecordToFile(args); };
    commandsMap["dgr"] = commandsMap["dgetrecord"];

    commandsMap["dcleartest"] = [this](const std::vector<std::string> &args)
    { clearTestFile(args); };
    commandsMap["dct"] = commandsMap["dcleartest"];

    commandsMap["dblockstats"] = [this](const std::vector<std::string> &args)
    { printBlockStatistics(args); };
    commandsMap["dbs"] = commandsMap["dblockstats"];

    commandsMap["dforceflush"] = [this](const std::vector<std::string> &args)
    { forceFlush(args); };
    commandsMap["dff"] = commandsMap["dforceflush"];

    commandsMap["insert"] = [this](const std::vector<std::string> &args)
    { insertRecord(args); };
    commandsMap["i"] = commandsMap["insert"];

    commandsMap["search"] = [this](const std::vector<std::string> &args)
    { searchRecord(args); };
    commandsMap["s"] = commandsMap["search"];

    commandsMap["clear"] = [this](const std::vector<std::string> &args)
    { clearFiles(args); };
    commandsMap["c"] = commandsMap["clear"];

    // print help message when starting the CLI
    printHelp();
}

void CommandLine::printHelp()
{
    std::cout << HELP_MESSAGE;
}

bool CommandLine::getYesNoAnswer()
{
    std::string ans;
    std::cout << "Answer (y/n):";
    while (true)
    {
        std::getline(std::cin, ans);

        if (ans == "y" || ans == "yes")
        {
            std::cout << "\n";
            return true;
        }
        else if (ans == "n" || ans == "no")
        {
            std::cout << "\n";
            return false;
        }
    }
}

void CommandLine::handleHelp(const std::vector<std::string> &args)
{
    if (args.size() == 1)
    {
        printHelp();
    }
    else if (args.size() == 2 && (args[1] == "debug" || args[1] == "d"))
    {
        std::cout << DEBUG_HELP_MESSAGE;
    }
    else
    {
        std::cout << "Argument error: Wrong amount of arguments, needs 0 or 1.\n";
    }
}

void CommandLine::loadTestFile(const std::vector<std::string> &args)
{
    if (args.size() != 2)
    {
        std::cout << "Argument error: Wrong amount of arguments, needs a string.\n";
        return;
    }

    std::fstream file(args[1], std::ios::in);
    if (!isFileOpenedCorrectly(file))
    {
        return;
    }

    std::string command;
    std::vector<std::string> commandArgs;

    handleInput(file, true);

    file.close();
}

void CommandLine::handleInput(std::istream &input, bool isTestFile)
{
    std::string command;
    std::vector<std::string> args;

    while (!quit && std::getline(input, command))
    {
        if (isTestFile)
        {
            std::cout << command << "\n";
        }

        args.clear();
        std::istringstream stream(command);
        std::string word;

        // put each ws separated word into args
        while (stream >> word)
        {
            args.push_back(word);
        }

        // ignore random enter presses
        if (args.size() == 0)
        {
            continue;
        }

        // see if args[0] is a valid command
        auto it = commandsMap.find(args[0]);
        if (it != commandsMap.end())
        {
            try
            {
                it->second(args);
            }
            catch (std::invalid_argument &e)
            {
                std::cout << "Argument error: " << "Wrong argument type" << "\n";
            }
        }
        else
        {
            std::cout << "Unknown command: '" + args[0] + "'\nType 'help' to list all available commands.\n";
        }
    }
}

void CommandLine::searchRecord(const std::vector<std::string> &args)
{
    if (args.size() != 2)
    {
        std::cout << "Argument error: Wrong amount of arguments, needs 2.\n";
        return;
    }

    int key = std::stoi(args[1]);

    std::optional<Record> result = btreeHandler->searchRecord(key);
    if (result == std::nullopt)
    {
        std::cout << "Result: NOT FOUND\n";
    }
    else
    {
        std::cout << "Result: " << result->toString() << "\n";
    }
}

void CommandLine::addRawRecordToFile(const std::vector<std::string> &args)
{
    if (args.size() != 2)
    {
        std::cout << "Argument error: Wrong amount of arguments, needs 2.\n";
        return;
    }

    int recordCount = std::stoi(args[1]);

    RecordBlockIO file(TEST_FILENAME);

    for (int i = 0; i < recordCount; i++)
    {
        Record record;
        record.Randomize();
        if (!file.writeRecordAt(i, record))
        {
            std::cout << "Error: Could not write record at offset " << i << "\n";
        }
    }

    std::cout << "Added " << recordCount << " records to the file\n";
}

void CommandLine::readRawRecordToFile(const std::vector<std::string> &args)
{
    if (args.size() != 2)
    {
        std::cout << "Argument error: Wrong amount of arguments, needs 2.\n";
        return;
    }

    int n = std::stoi(args[1]);

    RecordBlockIO file(TEST_FILENAME);

    Record record;
    if (!file.readRecordAt(n, record))
    {
        std::cout << "Error: Could not read record at offset " << n << "\n";
        return;
    }

    std::cout << record.toString() << std::endl;
}

void CommandLine::clearTestFile(const std::vector<std::string> &args)
{
    std::fstream file(TEST_FILENAME, std::ios::out | std::ios::trunc);
    if (!isFileOpenedCorrectly(file))
    {
        std::cout << "Error: Could not clear test file\n";
        return;
    }
    file.close();

    std::cout << "Test file cleared\n";
}

void CommandLine::printBlockStatistics(const std::vector<std::string> &args)
{
    std::cout << "Block reads: " << BlockInputOutput::getAllBlockReads() << "\n";
    std::cout << "Block writes: " << BlockInputOutput::getAllBlockWrites() << "\n";
    std::cout << "Record block reads: " << RecordBlockIO::getAllRecordBlockReads() << "\n";
    std::cout << "Record block writes: " << RecordBlockIO::getAllRecordBlockWrites() << "\n";
    std::cout << "Index block reads: " << IndexBlockIO::getAllIndexBlockReads() << "\n";
    std::cout << "Index block writes: " << IndexBlockIO::getAllIndexBlockWrites() << "\n";
}

void CommandLine::forceFlush(const std::vector<std::string> &args)
{
    btreeHandler->forceFlush();
    std::cout << "Files flushed\n";
}

bool CommandLine::isFileOpenedCorrectly(std::fstream &file)
{
    if (!file.is_open())
    {
        std::cout << "Error: Failed to open file '" << "'\n";
        return false;
    }
    if (!file.good())
    {
        std::cout << "Error: Unknown error with the file\n";
        file.close();
        return false;
    }
    return true;
}

void CommandLine::clearFiles(const std::vector<std::string> &args)
{
    btreeHandler = std::make_unique<BtreeHandler>(INDEXFILE_FILENAME, DATAFILE_FILENAME);
    std::cout << "Files cleared successfully\n";
}

void CommandLine::insertRecord(const std::vector<std::string> &args)
{
    if (args.size() != 3)
    {
        std::cout << "Argument error: Wrong amount of arguments, needs 3.\n";
        return;
    }

    int key = std::stoi(args[1]);
    int value = std::stoi(args[2]);

    Record record;
    record.fill(value);
    record.key = key;
    btreeHandler->insertRecord(record);
}
