#include "Tag.h"

std::vector< std::string > Tag::tags = { "Untagged", "MainCamera" };

Tag::Tag(std::string _tag):
    tag(getTag(_tag))
{ }

bool Tag::operator==(const Tag& _tag) const
{
    return (tag == _tag.tag);
}

bool Tag::operator==(const std::string& _tag) const
{
    return (tag == getTag(_tag));
}

int Tag::getTag(std::string _tag)
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
