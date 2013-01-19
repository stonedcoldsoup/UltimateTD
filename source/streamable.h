#ifndef UTD_STREAMABLE_H
#define UTD_STREAMABLE_H

#include "common.h"

namespace UTD
{
	class streamable
	{
	public:
		virtual ~streamable() {}
	
		virtual std::istream &read(std::istream &is) = 0;
		virtual std::ostream &write(std::ostream &os) const = 0;
	};
}

#endif