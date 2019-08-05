#include "Utility/IO/IOFile.h"

#include <fstream>

std::string IOFile::baseDirectory = "Resources/";

IOFile::IOFile()
{ }

IOFile::IOFile(std::string _path)
{
    load(_path);
}

IOFile::~IOFile()
{
    clear();
}

/// Methods
void IOFile::load(std::string _path)
{
    path = baseDirectory + _path;
    std::ifstream file(path.c_str());

    if (!file)
    {
        std::ofstream create(path.c_str());
        create.close();

        file.open(path.c_str());
    }

    std::string line;
    std::stringstream* stream = nullptr;

    while (getline(file, line))
    {
        if (!line.size())
            continue;

        stream = new std::stringstream(line);
        *stream >> line;

        entries[line] = stream;
    }
}

void IOFile::clear()
{
    for (auto& entry: entries)
        delete entry.second;

    entries.clear();
}

void IOFile::save()
{
    std::ofstream file(path.c_str());

    for (auto& entry: entries)
        file << entry.second->str() << std::endl;
}

void IOFile::remove(std::string _entry)
{
    auto it = entries.find(_entry);

    if (it == entries.end())
        return;

    delete it->second;

    entries.erase(it);
}

void IOFile::setBaseDirectory(const std::string& _baseDirectory)
{
    baseDirectory = _baseDirectory;

    if (baseDirectory.size() && baseDirectory.back() != '/')
        baseDirectory.append("/");
}

// TODO: optimize
std::stringstream& IOFile::operator[](std::string _entry)
{
    auto it = entries.find(_entry);

    if (it == entries.end())
    {
        entries[_entry] = new std::stringstream();
        it = entries.find(_entry);
        *it->second << _entry;
    }

    return *it->second;
}
