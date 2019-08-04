#include "Tag.h"

std::vector< std::string > Tag::tags = { "Untagged", "MainCamera" };

Tag::Tag(const char* _tag):
    Tag(std::string(_tag))
{ }

Tag::Tag(const std::string& _tag):
    tag(getTag(_tag))
{ }

std::string Tag::toString() const
{
    return tags[tag];
}


void Tag::operator=(const char* _tag)
{
    tag = getTag(_tag);
}

void Tag::operator=(const std::string& _tag)
{
    tag = getTag(_tag);
}


bool Tag::operator==(const Tag& _tag) const
{
    return (tag == _tag.tag);
}

bool Tag::operator==(const char* _tag) const
{
    return (tag == getTag(_tag));
}

bool Tag::operator==(const std::string& _tag) const
{
    return (tag == getTag(_tag));
}


bool Tag::operator!=(const Tag& _tag) const
{
    return (tag != _tag.tag);
}


int Tag::getTag(const std::string& _tag)
{
    auto it = std::find(tags.begin(), tags.end(), _tag);

    if ( it != tags.end() )
       return it - tags.begin();
    else
    {
        tags.push_back(_tag);
        return tags.size()-1;
    }
}

std::ostream& operator<<(std::ostream& os, const Tag& tag)
{
    return os << tag.toString();
}
