#pragma once

#include <exception>
#include <string>

#include "iocpcommon.h"

namespace iocp
{
	//iocp“Ï≥£
	class CIocpException : public std::exception
	{
	public:
		CIocpException(std::string error) :
			std::exception(error.c_str())
		{
		}
	};
}