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
			using format = platform::clipboard_format;

			// Static functions:
			clipboard(const window& wnd);
			clipboard(clipboard&&);

			~clipboard();

			// Operators:
			clipboard& operator=(clipboard&&) = delete;
			clipboard& operator=(const clipboard&) = delete;

			inline operator bool() const
			{
				return is_open();
			}

			/*
			template <typename T>
			inline operator T() const;
			*/
			inline operator std::string() const
			{
				return read<std::string>();
			}

			// Methods:
			inline bool is_open() const { return access; }
			inline bool is_closed() const { return !is_open(); }

			bool open(const window& owner = anonymous_window);
			bool close(const window& owner = anonymous_window);

			std::string read_text() const;

			// Memory is mapped and read from automatically when using 'read'.
			template <typename T=std::string, int integer_base=10>
			T read() const
			{
				//template <typename X>
				//using is_type<X> = std::is_same<T, X>;

				if constexpr (std::is_same_v<T, std::string> || std::is_convertible_v<std::string, T>)
				{
					return read_text();
				}
				else
				{
					const auto data = read<std::string>();

					if constexpr (std::is_arithmetic_v<T>)
					{
						if constexpr (std::is_same_v<T, long>)
						{
							return std::stol(data, nullptr, integer_base);
						}
						else if constexpr (std::is_same_v<T, long long>) // (std::is_same_v<T, long long>)
						{
							return std::stoll(data, nullptr, integer_base);
						}
						else if constexpr (std::is_same_v<T, int> || std::is_convertible_v<int, T>)
						{
							return std::stoi(data, nullptr, integer_base);
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

			// This will clear all data stored in the clipboard.
			bool clear();

			// Logging will fail gracefully if the clipboard isn't open,
			// or if a suitable output-stream could not be established.
			bool log(const path_t& file_path, bool append=false) const;

			template <typename call_back_t>
			inline int enumerate(const call_back_t& call_back, bool convert_types=false) const
			{
				int count = 0;

				platform::enum_clipboard_formats
				(
					[&](platform::clipboard_format type)
					{
						count += 1;

						return call_back(type);
					}, convert_types
				);

				return count;
			}

			inline int count() const
			{
				return enumerate
				(
					[](platform::clipboard_format) { return true; }
				);
			}

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