#ifndef ERROR_HPP_3039DFC5_A827_4918_A9B6_FEAEAE5E979D
#define ERROR_HPP_3039DFC5_A827_4918_A9B6_FEAEAE5E979D

#include <string>
#include <exception>

// Class used for exceptions. Unlike e.g. std::runtime_error, which only
// accepts a "fixed" string, Error provides std::printf()-like formatting.
// Example:
//
//	throw Error( "glGetError() returned %d", glerr );
//
class Error : public std::exception
{
	public:
		explicit Error( char const*, ... );

	public:
		char const* what() const noexcept override;

	private:
		std::string mMsg;
};

#endif // ERROR_HPP_3039DFC5_A827_4918_A9B6_FEAEAE5E979D
