#pragma once

#include <type_traits>
#include <ostream>
#include <cstddef>
#include <stdexcept>

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
				return (is_open() && has_segment());
			}

			/*
			template <typename T>
			inline operator T() const;
			*/
			inline operator std::string() const
			{
				return read<std::string>();
			}

			/*
			inline bool operator[](format type=format::ANY) const
			{
				return has_segment(type);
			}
			*/

			// This simply redirects to the 'context' command.
			// NOTE: When using this operator from the context of this class, beware of pointer-indexing.
			inline memory operator[](format type) const
			{
				return context(type);
			}

			template <typename T>
			inline clipboard& operator>>(T& out) const
			{
				out = read<T>();

				return *this;
			}

			// Methods:
			inline bool is_open() const { return access; }
			inline bool is_closed() const { return !is_open(); }

			bool open(const window& owner=anonymous_window);
			bool close(const window& owner=anonymous_window);

			// This opens a memory-context for the format specified.
			// The clipboard object must have an open handle to the system's clipboard.
			memory context(format type) const;

			std::string read_text() const;
			bool write_text(const std::string& data) const;

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
						try
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
						catch (const std::invalid_argument& ex)
						{
							return 0;
						}
						catch (const std::out_of_range& ex)
						{
							return 0;
						}
					}
					else
					{
						static_assert(false, "Unable to find suitable conversion type.");
					}
				}
			}

			// Logging will fail gracefully if the clipboard isn't open,
			// or if a suitable output-stream could not be established.
			bool log(const path_t& file_path, bool append=false) const;

			// This will clear all data stored in the clipboard.
			bool clear();
			
			std::size_t size() const;

			// This returns the raw size of a clipboard data-segment. (In bytes)
			std::size_t size(format type) const;

			// This returns the length of the text-segment, rather than
			// its size, as allocated by the operating system.
			std::size_t text_length() const;

			template <typename call_back_t>
			inline int enumerate(const call_back_t& call_back, bool convert_types=false) const
			{
				int count = 0;

				platform::enum_clipboard_formats
				(
					[&](format type)
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

			inline bool has_segment(format type=format::ANY) const
			{
				return platform::has_clipboard_format(type);
			}

			inline bool has_text() const
			{
				//return (text_length() > 0);
				return has_segment(format::TEXT);
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