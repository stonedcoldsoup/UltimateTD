#ifndef UTD_FISMATH_H
#define UTD_FISMATH_H

#include "common.h"

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_2_PI
#define M_2_PI (2.0*M_PI)
#endif

namespace UTD
{
    namespace math
    {
        // general purpose interpolator template
        // interface with linear and logarithmic
        // flavors.
        template <typename valueT>
        struct interpolator
        {
            virtual valueT operator()(const valueT &a, const valueT &b, float t) = 0;
        };

        template <typename valueT>
        struct lerp_op: interpolator<valueT>
        {
            inline virtual valueT operator()(const valueT &a, const valueT &b, float t)
            {
                return a*(1.0f-t) + b*t;
            }
        };

        template <typename valueT>
        struct log2erp_op: interpolator<valueT>
        {
            inline virtual valueT operator()(const valueT &a, const valueT &b, float t)
            {
                valueT _a = (a <= 0.0f) ? 0.0001f : a;
                return _a*powf(float(((b <= 0.0f) ? 0.0001f : b)/_a), t);
            }
        };
    
        // a pretty feature rich lightweight angle class originally
        // designed for 2D skeletal animation. it does range clamped
        // or shortest route interpolation.  all operations are constant
        // time, and there are no hacks.  the math used should always
        // just work.
        class angle
        {
        private:
            float m_ang;

        public:
            angle():
                m_ang(0.0f)
            {}

            angle(float m_ang):
                m_ang(m_ang)
            {}

            angle(const angle &m_ang):
                m_ang(m_ang.m_ang)
            {}

            inline angle &operator =(float m_ang) {this->m_ang = m_ang; return *this;}
            inline angle &operator =(const angle &m_ang) {this->m_ang = m_ang.m_ang; return *this;}

            inline operator float() const {return m_ang;}

            // depreciated
            inline angle &rotate(angle m_ang)
            {
                this->m_ang += m_ang.m_ang;
                return *this;
            }

            // depreciated
            inline angle &add(angle m_ang) {return rotate(m_ang);}

            // depreciated
            inline angle &sub(angle m_ang)
            {
                this->m_ang -= m_ang.m_ang;
                return *this;
            }
            
            // proper operators
            inline angle &operator +=(angle m_ang)  
            {
                this->m_ang += m_ang.m_ang;
                return *this;
            }
            
            inline angle &operator -=(angle m_ang)
            {
                this->m_ang -= m_ang.m_ang;
                return *this;
            }
            
            inline angle &operator *=(float m_scalar)
            {
                m_ang *= m_scalar;
                return *this;
            }
            
            inline angle &operator /=(float m_scalar)
            {
                m_ang /= m_scalar;
                return *this;
            }
            
            friend inline angle operator +(angle m_0, angle m_1)
            {
                m_0 += m_1;
                return m_0;
            }
            
            friend inline angle operator -(angle m_0, angle m_1)
            {
                m_0 -= m_1;
                return m_0;
            }
            
            friend inline angle operator *(angle m_0, float m_1)
            {
                m_0 *= m_1;
                return m_0;
            }
            
            friend inline angle operator *(float m_0, angle m_1)
            {
                m_1 *= m_0;
                return m_1;
            }
            
            friend inline angle operator /(angle m_0, float m_1)
            {
                m_0 /= m_1;
                return m_0;
            }

            inline angle &modulus()
            {
                // get the real angle modulus. fmodf gives
                // wrong result for negative values.
                m_ang -= float(M_2_PI)*std::floor(m_ang/float(M_2_PI));
                return *this;
            }

            inline angle &modulus_signed()
            {
                // get the real angle modulus. fmodf gives
                // wrong result for negative values.
                // modulus range is shifted back
                m_ang -= float(M_2_PI)*std::floor(m_ang/float(M_2_PI) + 0.5f);
                return *this;
            }

            inline angle &clamp(angle m_min, angle m_max)
            {
                angle ep = this->m_ang - m_min.m_ang;
                angle si = m_max.m_ang - m_min.m_ang;

                ep.modulus();
                si.modulus();

                float w = (float)M_PI + si.m_ang*0.5f;
                float th;

                // conditionals here are faster
                // than computing the solution analytically.
                if (ep > w)
                    th = 0.0f;
                else if (ep > si.m_ang)
                    th = si.m_ang;
                else
                    th = ep;

                this->m_ang = th + m_min.m_ang;

                return modulus();
            }

            template <typename interpolatorT = lerp_op<angle>>
            static inline angle lerp(angle m_0, angle m_1, float m_delta, interpolatorT &&m_lerp = interpolatorT())
            {
                return m_lerp(m_0.m_ang, m_1.m_ang, m_delta);
            }

            // Angluar lerp that always takes the shortest path.
            template <typename interpolatorT = lerp_op<angle>>
            static inline angle lerp_closest(angle m_0, angle m_1, float m_delta, interpolatorT &&m_lerp = interpolatorT())
            {
                if (fabsf(m_0.modulus().m_ang - m_1.modulus().m_ang) > (float)M_PI)
                {
                    if (m_0.m_ang < m_1.m_ang)
                        m_0.m_ang += (float)M_2_PI;
                    else
                        m_1.m_ang += (float)M_2_PI;
                }

                return lerp(m_0, m_1, m_delta, m_lerp);
            }

            // Angular lerp that always takes the shortest path, except for when that path falls outside of
            // a specific range.  This lerp is guaranteed not to pass outside the range unless it starts or ends there.
            template <typename interpolatorT = lerp_op<angle>>
            static inline angle lerp_range(angle m_0, angle m_1, float m_delta, angle m_min, angle m_max, interpolatorT &&m_lerp = interpolatorT())
            {
                if (m_max.modulus().m_ang > m_min.modulus().m_ang) m_min.m_ang += (float)M_2_PI;

                m_0.modulus();
                m_1.modulus();

                angle qi = (m_max.m_ang + m_min.m_ang) * 0.5f;
                qi.modulus();

                if (m_0.m_ang < m_1.m_ang)
                {
                    if (m_0.m_ang < qi.m_ang && qi.m_ang < m_1.m_ang)
                        m_0.m_ang += (float)M_2_PI;
                }
                else if (m_0.m_ang >= qi.m_ang && qi.m_ang >= m_1.m_ang)
                    m_1.m_ang += (float)M_2_PI;

                return lerp(m_0, m_1, m_delta, m_lerp);
            }
        };
    }
	
	namespace random
	{
		void seed_time();
		void seed_bytes(const void *p, size_t n);

		float floating();

		static inline float range(float min, float max)
		{
			return min + (max-min) * floating();
		}
	}
}

#endif

