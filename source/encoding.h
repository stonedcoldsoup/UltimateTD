#ifndef UTD_ENCODING_H
#define UTD_ENCODING_H

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

namespace UTD
{
	namespace encoding
	{
		namespace detail
		{
			template <char neg>
			struct sign_functions
			{
				template <typename intT, typename containerT>
				static inline void append_sign(intT m_input, containerT &m_container)
				{
					if (m_input < 0) m_container.push_back(neg);
				}
				
				template <typename intT, uint8_t min_value, uint8_t base>
				static inline void get_digit(uint8_t v, intT &result, intT &factor)
				{
					if (char(v) != neg)
					{
						result += factor * intT(v - min_value);
						factor *= intT(base);
					}
					else
						result *= -1;
				}
			};
			
			template <>
			struct sign_functions<'\0'>
			{
				template <typename intT, typename containerT>
				static inline void append_sign(intT m_input, containerT &m_container) {}
				
				template <typename intT, uint8_t min_value, uint8_t base>
				static inline void get_digit(uint8_t v, intT &result, intT &factor)
				{
					result += factor * intT(v - min_value);
					factor *= intT(base);
				}
			};
		}
	
		template <typename intT, char neg, uint8_t min_value, uint8_t max_value = 254>
		struct profile
		{
			typedef intT int_type;
			
			static constexpr bool    is_signed      = neg != '\0';
			
			static constexpr uint8_t alphabet_start = min_value;
			static constexpr uint8_t base           = max_value - min_value;
			
			template <typename containerT = std::vector<char> >
			static inline void encode(intT m_input, containerT &m_out)
			{
				m_out.clear();
				
				intT quotient = std::abs(m_input);

				do
				{
					m_out.push_back(alphabet_start + quotient % base);
					quotient /= base;
				} while (quotient);

				detail::sign_functions<neg>::append_sign(m_input, m_out);

				std::reverse(m_out.begin(), m_out.end());
			}

			template <typename containerT = std::vector<char> >
			static inline intT decode(const containerT &m_in)
			{
				intT result = 0, factor = 1;
				for (auto it = m_in.rbegin(); it != m_in.rend(); ++it)
					detail::sign_functions<neg>::template get_digit<intT, min_value, base>(uint8_t(*it), result, factor);
				
				return result;
			}
		};
		
		template <typename intT, uint8_t n_base = 254 - '"'>
		struct base_signed_ascii_profile:
			profile<intT, '!', '"', '"' + n_base>
		{
			typedef intT int_type;
			
			static constexpr bool    is_signed      = true;
			
			static constexpr uint8_t alphabet_start = '"';
			static constexpr uint8_t base           = n_base;
		};
		
		template <typename intT, uint8_t n_base = 254 - '!'>
		struct base_unsigned_ascii_profile:
			profile<intT, '\0', '!', '!' + n_base>
		{
			typedef intT int_type;
			
			static constexpr bool    is_signed      = false;
			
			static constexpr uint8_t alphabet_start = '!';
			static constexpr uint8_t base           = n_base;
		};
		
		namespace detail
		{
			template <bool b_signed, typename intT>
			struct base_ascii_profile_select;
			
			template <typename intT>
			struct base_ascii_profile_select<true, intT>
			{
				typedef base_signed_ascii_profile<intT> type;
			};
			
			template <typename intT>
			struct base_ascii_profile_select<false, intT>	
			{
				typedef base_unsigned_ascii_profile<intT> type;
			};
		}
		
		template <typename intT>
		struct default_ascii_profile:
			detail::base_ascii_profile_select<std::is_signed<intT>::value, intT>::type
		{};
		
		template <typename profileT>
		struct encode
		{
			typedef profileT profile_type;
			typedef typename profile_type::int_type int_type;
			
			static constexpr bool    is_signed      = profile_type::is_signed;
			
			static constexpr uint8_t alphabet_start = profile_type::alphabet_start;
			static constexpr uint8_t base           = profile_type::base;
			
			std::vector<char> dat;
			
			encode(int_type value_out)
			{
				profile_type::encode(value_out, dat);
			}
			
			friend inline std::ostream &operator <<(std::ostream &os, const encode &en)
			{
				for (char v: en.dat) os.put(v);
				return os;
			}
		};
		
		template <typename profileT>
		struct decode
		{
			typedef profileT profile_type;
			typedef typename profile_type::int_type int_type;
			
			static constexpr uint8_t alphabet_start = profile_type::alphabet_start;
			static constexpr uint8_t base           = profile_type::base;
			
			int_type &value_ref;
			
			decode(int_type &value_ref):
				value_ref(value_ref)
			{}
			
			friend inline std::istream &operator >>(std::istream &is, const decode &de)
			{
				std::string s;
				if (is >> s)
					de.value_ref = profile_type::decode(s);
				
				return is;
			}
		};
	}
}

#endif
