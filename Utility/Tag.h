#ifndef TAG_H
#define TAG_H

#include "includes.h"

class Tag
{
    public:
        Tag(std::string _tag);

        bool operator==(const Tag& _tag) const;
        bool operator==(const std::string& _tag) const;

    private:
        static int getTag(std::string _tag);

        static std::vector< std::string > tags;

        int tag;
};

#endif // TAG_H
