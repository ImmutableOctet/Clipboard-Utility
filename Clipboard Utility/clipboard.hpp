#pragma once

#include <type_traits>
#include <ostream>

#include "assert.hpp"
#include "platform.hpp"

namespace clip
{
	class clipboard
	{
		private:
			clipboard(const clipboard&) = default;
		public:
			clipboard(const window& wnd);
			clipboard(clipboard&&);

			~clipboard();

			// Operators:
			clipboard& operator=(clipboard&&) = delete;
			clipboard& operator=(const clipboard&) = delete;

			inline operator bool() const
			{
				return access;
			}

			/*
			template <typename T>
			inline operator T() const
			*/
			inline operator std::string() const
			{
				using T = std::string;

				static_assert(std::is_default_constructible_v<T>, "Specified type must be default-constructible.");
				static_assert(std::is_constructible_v<T, const char*>, "Specified type must be constructible using 'const char*' as a parameter.");

				ASSERT(is_open());

				// ATTENTION: We do not own this handle, and will
				// therefore not need to free it. We will,
				// however, need to lock and unlock it.
				handle data_handle = GetClipboardData(CF_TEXT);

				// Check if we could open the handle.
				if (data_handle)
				{
					const char* raw_data = reinterpret_cast<const char*>(GlobalLock(data_handle));

					if (raw_data)
					{
						// ATTENTION: Not memory-safe; if an exception is thrown, 
						auto out = T(raw_data);

						GlobalUnlock(data_handle);

						return out;
					}
				}

				return T();
			}

			// Methods:
			bool open(const window& owner = anonymous_window);
			bool close(const window& owner = anonymous_window);

			template <typename T=std::string>
			T read(int base=10) const
			{
				//template <typename X>
				//using is_type<X> = std::is_same<T, X>;

				if constexpr (std::is_same_v<T, std::string> || std::is_convertible_v<std::string, T>)
				{
					return static_cast<std::string>(*this);
				}
				else
				{
					const auto data = read<std::string>();

					if constexpr (std::is_arithmetic_v<T>)
					{
						if constexpr (std::is_same_v<T, long>)
						{
							return std::stol(data, nullptr, base);
						}
						else if constexpr (std::is_same_v<T, long long>) // (std::is_same_v<T, long long>)
						{
							return std::stoll(data, nullptr, base);
						}
						else if constexpr (std::is_same_v<T, int> || std::is_convertible_v<int, T>)
						{
							return std::stoi(data, nullptr, base);
						}
						else if constexpr (std::is_same_v<T, float>)
						{
							return std::stof(data);
						}
						else if constexpr (std::is_convertible_v<T, double>)
						{
							return std::stod(data);
						}
						else
						{
							static_assert(false, "Unable to find suitable conversion to arithmetic type.");
						}
					}
					else
					{
						static_assert(false, "Unable to find suitable conversion type.");
					}
				}
			}

			bool clear();

			// Log can silently fail if the clipboard isn't open.
			bool log(const path_t& file_path, bool append=false) const;

			inline bool is_open() const { return *this; }
			inline bool is_closed() const { return !is_open(); }

			// Fields:
			const window& owner;

			bool access = false;
	};

	inline void operator~(clipboard& c)
	{
		c.clear();
	}

	// STL Output-stream support.
	inline std::ostream& operator<<(std::ostream& os, const clipboard& c)
	{
		os << static_cast<std::string>(c);

		return os;
	}
}