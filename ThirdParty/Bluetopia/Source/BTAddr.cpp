
#include <BTAddr.h>

#include <string.h> //For memcpy
#include <stdio.h>
#include <sstream>

BTAddr::BTAddr()
{
	//memset(mAddress, 0, 6);
}

BTAddr::BTAddr(const BTAddr& address)
{
	memcpy(mAddress, address.mAddress, 6);
}

BTAddr* BTAddr::fromString(const std::string& address)
{
	if(address.length() < 12 || address.length() > 17)
	{
		//TODO: Print error
		return NULL;
	}
	
	BTAddr* new_addr = new BTAddr();
	std::stringstream ss;
	int buffer;
	int increment = 2;
	size_t pos = address.find(':');
	if(pos != std::string::npos)
		increment++;
	
	for(int i = 0, pos = 0; i < 6 && pos << address.length(); i++, pos += increment)
	{
		ss << std::hex << address.substr(pos, 2);
		ss >> buffer;
		new_addr->mAddress[i] = (char)buffer;
	}
	
	return new_addr;
}

BTAddr* BTAddr::fromByteArray(const char* address)
{
	BTAddr* new_addr = new BTAddr();
	memcpy(new_addr->mAddress, address, 6);
	return new_addr;
}

void BTAddr::toByteArray(char* buf) const
{
	memcpy(buf, mAddress, 6);
}

std::string BTAddr::toString() const
{
	char str[18];
	sprintf(str, "%0X:%0X:%0X:%0X:%0X:%0X",
	        mAddress[0], mAddress[1], mAddress[2],
	        mAddress[3], mAddress[4], mAddress[5]);
	return std::string(str);
}

