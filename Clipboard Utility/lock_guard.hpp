#pragma once

#include <stdexcept>

#include "assert.hpp"

namespace clip
{
	/*
		NOTE: By default, guards automatically assert if they are unable to aquire a lock,
		this means they are normally only applicable for critical resource acquisition.
		
		To change this behavior, change 'exceptions' flag.
		
		With exceptions enabled:
			If an error occurs whilst establishing a lock, this object will be thrown.
	*/
	template <typename resource_t, typename observable_t, observable_t failure, bool exceptions=false>
	struct lock_guard
	{
		private:
			observable_t observed = failure;

			resource_t& resource;
		public:
			lock_guard(resource_t& memory)
				: resource(memory)
			{
				observed = resource.lock();

				bool condition = (observed != failure);

				if (!condition)
				{
					if constexpr (exceptions)
					{
						throw *this;
					}
					else
					{
						ASSERT(condition);
					}
				}
			}

			~lock_guard()
			{
				if (observed != failure)
				{
					bool condition = (resource.unlock(observed));

					if (!condition)
					{
						if constexpr (exceptions)
						{
							throw *this;
						}
						else
						{
							ASSERT(condition);
						}
					}
				}
			}

			inline observable_t value() const
			{
				auto result = observed;

				// For the sake of ensuring runtime safety,
				// we'll make sure this is usable:
				ASSERT(result != failure);

				return result;
			}

			inline observable_t ptr() const { return value(); }

			// Operators:
			inline operator resource_t&() { return resource; };
	};
}