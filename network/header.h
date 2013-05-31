#ifndef __HEADER_H
#define __HEADER_H

#include "corelib/simple-ref-count.h"

namespace vndn
{

class Buffer;

class Header : public SimpleRefCount<Header>
{
public:
    virtual void Serialize(Buffer &buffer) const = 0;
    virtual uint32_t Deserialize(const Buffer &buffer) = 0;
    virtual uint32_t GetSize() const = 0;
    virtual ~Header() {}
};

}
#endif
