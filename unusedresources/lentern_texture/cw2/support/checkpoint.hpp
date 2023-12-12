#ifndef CHECKPOINT_HPP_3DFDA796_469C_4D37_B904_1C8D8FAE207B
#define CHECKPOINT_HPP_3DFDA796_469C_4D37_B904_1C8D8FAE207B

#define OGL_CHECKPOINT_ALWAYS() do {                                \
		::detail::check_gl_error( __FILE__, __LINE__ );             \
	} while(0)                                                      \
	/*ENDM*/

#if defined(NDEBUG)
#	define OGL_CHECKPOINT_DEBUG()   do {} while(0)
#else
#	define OGL_CHECKPOINT_DEBUG()   OGL_CHECKPOINT_ALWAYS()
#endif

namespace detail
{
	void check_gl_error( char const*, int );
}

#endif // CHECKPOINT_HPP_3DFDA796_469C_4D37_B904_1C8D8FAE207B

