#pragma once

#include <map>
#include <string>
#include <sstream>

class IOFile
{
    public:
        IOFile();
        IOFile(std::string _path);

        ~IOFile();

        void load(std::string _path);
        void clear();

        void save();

        void remove(std::string _entry);

        template <typename T>
        T read(std::string _entry)
        {
            auto& stream = operator[](_entry);
            stream.seekp(0, stream.beg);

            T val;
            stream >> val;

            return val;
        }

        template <typename T>
        bool read(std::string _entry, T& _val)
        {
            auto& stream = operator[](_entry);
            stream.seekp(0, stream.beg);

            return (stream >> _val);
        }

        template <typename T>
        void write(std::string _entry, T _val)
        {
            auto& stream = operator[](_entry);
            stream.seekp(0, stream.end);

            stream << " " << _val;
        }

        std::stringstream& operator[](std::string _entry);

        static void setBaseDirectory(const std::string& _baseDirectory = "");

    private:
        std::string path;

        std::map<std::string, std::stringstream*> entries;

        static std::string baseDirectory;
};
