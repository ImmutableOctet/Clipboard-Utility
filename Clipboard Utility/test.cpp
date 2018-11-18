#include "types.hpp"
#include "clipboard.hpp"

// Unit-test dependencies:
#include <iostream>

#include <chrono>
#include <thread>
//#include <random>

namespace clip
{
	namespace unit_test
	{
		// Private:
		template <typename T, bool hex_output=true>
		void output_bytes(const T& value)
		{
			constexpr auto value_size = sizeof(T);

			std::cout << +value << " = [";

			auto bytes = reinterpret_cast<const std::uint8_t*>(&value);

			auto i = 0;

			while (true)
			{
				const auto& current_byte = bytes[i];

				if constexpr (hex_output)
				{
					std::cout << std::hex << +current_byte;
				}
				else
				{
					std::cout << +current_byte;
				}

				if (i++ >= value_size)
				{
					break;
				}
				else
				{
					std::cout << ", ";
				}
			}

			std::cout << "]\n";
		}

		template <typename T>
		void test_io_formatted(clipboard& c, const T& value)
		{
			std::cout << "Writing a new message to the clipboard...\n";

			c << value;

			std::cout << "Reading the message back from the clipboard...\n";

			T value_out;

			c >> value_out;

			std::cout << "\"" << value << "\" vs. \"" << value_out << "\"\n";
		}

		template <typename T>
		bool test_io_raw(clipboard& c, const T& value)
		{
			std::cout << '\n';

			std::cout << "Writing raw data of type '" << typeid(T).name() << "' to the clipboard...\n";

			auto result = c.write(value, true);

			output_bytes<T, false>(value);

			if (result)
			{

				std::cout << "Reading raw data back from the clipboard...\n";

				auto value_out = c.read<T>(true);

				output_bytes<T, false>(value_out);

				std::cout << "\"" << value << "\" vs. \"" << value_out << "\"\n";
			}
			else
			{
				std::cout << "Failed to write to clipboard.\n";
			}

			std::cout << '\n';

			return result;
		}

		// Public:
		bool test(bool condition, const std::string& when_true, const std::string& when_false)
		{
			const auto endl = '\n';

			if (condition)
				std::cout << when_true << endl;
			else
				std::cout << when_false << endl;

			return condition;
		}

		void feature_test(const int iterations, const int seperator_length)
		{
			auto make_separator = [&]()
			{
				std::cout << '\n';

				for (auto i = 1; i <= seperator_length; i++)
				{
					std::cout << '-';
				}

				std::cout << '\n';
			};

			for (auto i = 1; i <= iterations; i++)
			{
				std::cout << "Running test suite... (#" << i << ")\n\n";


				std::cout << "Opening handle to clipboard...\n";

				clip::clipboard c(clip::anonymous_window);

				//DEBUG_ASSERT(c.is_open(), "Clipboard not available.");

				while (c.is_closed())
				{
					std::cout << "Unable to open handle to clipboard, retrying...\n";
					std::this_thread::sleep_for(std::chrono::seconds(1));

					if (c.open())
					{
						std::cout << "Clipboard handle opened." << std::endl;
					}
				}


				std::cout << "\nCounting clipboard segments...\n\n";

				auto clipboard_segments = c.count();

				std::cout << "Entries found: " << clipboard_segments << "\n";
				std::cout << "Total size: " << c.size() << " bytes\n";

				std::cout << "\nLooking for TEXT segment...\n";

				if (c)
				{
					if (c.has_text())
					{
						std::cout << "\nText currently in clipboard:" << "\n" << c << "\n";
						std::cout << "\nLength of TEXT segment: " << c.text_length() << "\n";
						std::cout << "\nLogging text to output file...\n";

						// Log the clipboard to a text file.
						c.log("output/clipboard.txt");
					}
					else
					{
						std::cout << "No TEXT segment detected.\n";
					}

					std::cout << "\nClearing clipboard contents...\n\n";

					// Clear the clipboard's contents.
					c.clear();
				}
				else
				{
					std::cout << "No segments found. No reason to clear clipboard.\n";
				}

				test_io_formatted<std::string>(c, "Hello world.");
				test_io_raw(c, 1.23456f);
				test_io_raw(c, 7.891011);
				test_io_raw(c, 0xff00ff00);

				if (i < iterations)
				{
					std::cout << "Running tests again... (Tests remaining: " << (iterations - i) << ")\n";

					make_separator();
				}
			}

			for (auto i = 1; i <= 4; i++)
				std::cout << '\n';
		}
	}
}