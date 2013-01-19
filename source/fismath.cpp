#include "fismath.h"
#include "prng.h"

namespace UTD
{
	namespace random
	{
		void seed_time()                         {prng_seed_time();}
		void seed_bytes(const void *p, size_t n) {prng_seed_bytes(p, n);}

		float floating() {return prng_get_double();}
	}
}
