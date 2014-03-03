#ifndef __BTADDR_H_
#define __BTADDR_H_

#include <string>

/**
 * @brief A class to represent a Bluetooth device address, also known as a Bluetooth ID.
 */
class BTAddr
{
	public:
		/**
		 * @brief Copy Constructor
		 */
		BTAddr(const BTAddr& address);
		
		/**
		 * @brief Create a BTAddr object from a string
		 *
		 * @param address The string containing the address. It can either be
		 *                formatted as "000000000000" or "00:00:00:00:00:00"
		 */
		static BTAddr* fromString(const std::string& address);
		
		/**
		 * @brief Create a BTAddr object from a byte array
		 *
		 * @param address The byte array to take the address from. It must
		 *                be at least 6 bytes long.
		 */
		static BTAddr* fromByteArray(const char* address);
		
		/**
		 * @brief Copy this BTAddr into the given buffer.
		 *
		 * @param buf A buffer at least 6 bytes long that the address will be
		 *            copied into.
		 */
		void toByteArray(char* buf);
		
		/**
		 * @brief Copy this BTAddr into a human readable string.
		 *
		 * @return A human readable string in the format 00:00:00:00:00:00
		 */
		std::string toString();
		
	private:
		BTAddr();
		char[6]	mAddress;
};

#endif //BTADDR_H_

