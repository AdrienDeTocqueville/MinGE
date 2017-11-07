#pragma once

#include "includes.h"

class Tag
{
    public:
        Tag(const char* _tag);
        Tag(const std::string& _tag);

        void operator=(const char* _tag);
        void operator=(const std::string& _tag);

        bool operator==(const Tag& _tag) const;
        bool operator==(const char* _tag) const;
        bool operator==(const std::string& _tag) const;

        bool operator!=(const Tag& _tag) const;

        std::string toString() const;

    private:
        static int getTag(const std::string& _tag);

        static std::vector< std::string > tags;

        int tag;
};

std::ostream& operator<<(std::ostream& os, const Tag& tag);
