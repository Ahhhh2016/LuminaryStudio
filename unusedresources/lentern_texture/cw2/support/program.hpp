#ifndef PROGRAM_HPP_39793FD2_7845_47A7_9E21_6DDAD42C9A09
#define PROGRAM_HPP_39793FD2_7845_47A7_9E21_6DDAD42C9A09

#include <glad.h>

#include <string>
#include <vector>

#include <cstdint>
#include <cstdlib>

class ShaderProgram final
{
	public:
		struct ShaderSource
		{
			GLenum type;
			std::string sourcePath;
		};

	public:
		explicit ShaderProgram( 
			std::vector<ShaderSource> = {}
		);

		~ShaderProgram();

		ShaderProgram( ShaderProgram const& ) = delete;
		ShaderProgram& operator= (ShaderProgram const&) = delete;

		ShaderProgram( ShaderProgram&& ) noexcept;
		ShaderProgram& operator= (ShaderProgram&&) noexcept;

	public:
		GLuint programId() const noexcept;

		void reload();

	private:
		GLuint mProgram;
		std::vector<ShaderSource> mSources;
};

#endif // PROGRAM_HPP_39793FD2_7845_47A7_9E21_6DDAD42C9A09
