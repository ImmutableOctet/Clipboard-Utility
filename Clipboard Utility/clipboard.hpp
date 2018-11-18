#pragma once

/*
	TODO:
		* Implement appending to the clipboard using the I/O operators.
		* Implement other formats properly.
		* Test current I/O functionality.
*/


// Includes:
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
			inline clipboard& operator>>(T& out)
			{
				out = read<T>();

				return *this;
			}

			template <typename T>
			inline clipboard& operator<<(const T& in)
			{
				// Just for the sake of it, assert on this write-operation.
				ASSERT(write<T>(in));

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

			// Reads from the 'TEXT' segment as "raw-data", rather than being formatted.
			// The 'offset' argument is unsigned for safety purposes.
			bool read_text_raw(void* data, std::size_t size, std::size_t offset=0) const;

			bool write_text(const std::string& data) const;
			bool write_text_raw(const void* data_in, std::size_t size, std::size_t offset=0) const;

			// Memory is mapped and read from automatically when using 'read'.
			template <typename T=std::string, int integer_base=10>
			T read(bool raw_transfer=false) const
			{
				//template <typename X>
				//using is_type<X> = std::is_same<T, X>;

				if constexpr (std::is_same_v<T, std::string> || std::is_convertible_v<std::string, T>)
				{
					return read_text();
				}
				else
				{
					if (raw_transfer)
					{
						T data_out = {};

						auto result = read_text_raw(&data_out, sizeof(data_out));

						ASSERT(result);

						return data_out;
					}
					else
					{
						const auto data = read<std::string>();

						if constexpr (std::is_arithmetic_v<T>)
						{
							try
							{
								if constexpr (std::is_floating_point_v<T>)
								{
									if constexpr (std::is_same_v<T, float>)
									{
										return std::stof(data);
									}
									else if constexpr (std::is_same_v<T, double>)
									{
										return std::stod(data);
									}
									else
									{
										static_assert(false, "Unable to find suitable conversion to floating-point arithmetic type.");
									}
								}
								else
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
									else
									{
										static_assert(false, "Unable to find suitable conversion to integral arithmetic type.");
									}
								}
							}
							catch (const std::invalid_argument&)
							{
								return 0;
							}
							catch (const std::out_of_range&)
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
			}

			template <typename T = std::string, int integer_base = 10>
			bool write(const T& data, bool raw_transfer=false) const
			{
				if constexpr (std::is_same_v<T, std::string> || std::is_convertible_v<T, std::string>)
				{
					return write_text(data);
				}
				else if constexpr (std::is_arithmetic_v<T>)
				{
					if (raw_transfer)
					{
						return write_text_raw(&data, sizeof(data));
					}
					else
					{
						return write_text(std::to_string(data));
					}
				}

				return false;
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