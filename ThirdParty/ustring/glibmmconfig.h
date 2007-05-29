/* glib/glibmmconfig.h.  Generated by configure.  */
#ifndef _GLIBMM_CONFIG_H
#define _GLIBMM_CONFIG_H 1

/* version numbers */
#define GLIBMM_MAJOR_VERSION 2
#define GLIBMM_MINOR_VERSION 4
#define GLIBMM_MICRO_VERSION 8

// detect common platforms
#if defined(_WIN32)
// Win32 compilers have a lot of varation
#if defined(_MSC_VER)
#define GLIBMM_MSC
#define GLIBMM_WIN32
#define GLIBMM_DLL
#elif defined(__CYGWIN__)
#define GLIBMM_CONFIGURE
#elif defined(__MINGW32__)
#define GLIBMM_WIN32
#define GLIBMM_CONFIGURE
#else
//AIX clR compiler complains about this even though it doesn't get this far:
//#warning "Unknown architecture (send me gcc --dumpspecs or equiv)"
#endif
#else
#define GLIBMM_CONFIGURE
#endif /* _WIN32 */

#ifdef GLIBMM_CONFIGURE
/* compiler feature tests that are used during compile time and run-time
   by gtk-- only. tests used by gdk-- and gtk-- should go into
   gdk--config.h.in */
/* #undef GLIBMM_CXX_HAVE_MUTABLE */
/* #undef GLIBMM_CXX_HAVE_NAMESPACES */
//#undef GLIBMM_CXX_GAUB
//#undef GLIBMM_CXX_AMBIGUOUS_TEMPLATES
#define GLIBMM_HAVE_NAMESPACE_STD 1
#define GLIBMM_HAVE_STD_ITERATOR_TRAITS 1
/* #undef GLIBMM_HAVE_SUN_REVERSE_ITERATOR */
#define GLIBMM_HAVE_TEMPLATE_SEQUENCE_CTORS 1, Defined if the STL containers have templated sequence ctors
#define GLIBMM_HAVE_DISAMBIGUOUS_CONST_TEMPLATE_SPECIALIZATIONS 1
#define GLIBMM_CAN_USE_DYNAMIC_CAST_IN_UNUSED_TEMPLATE_WITHOUT_DEFINITION 1
#define GLIBMM_CAN_ASSIGN_NON_EXTERN_C_FUNCTIONS_TO_EXTERN_C_CALLBACKS 1
#define GLIBMM_CAN_USE_NAMESPACES_INSIDE_EXTERNC 1
/* #undef GLIBMM_COMPILER_SUN_FORTE */
/* #undef GLIBMM_DEBUG_REFCOUNTING */
#endif

#ifdef GLIBMM_MSC
  #define GLIBMM_CXX_HAVE_MUTABLE
  #define GLIBMM_CXX_HAVE_NAMESPACES
  #define GLIBMM_HAVE_NAMESPACE_STD 1
  #define GLIBMM_HAVE_STD_ITERATOR_TRAITS 1
  #define GLIBMM_HAVE_TEMPLATE_SEQUENCE_CTORS 1, Defined if the STL containers have templated sequence ctors
  #define GLIBMM_HAVE_DISAMBIGUOUS_CONST_TEMPLATE_SPECIALIZATIONS 1
  #define GLIBMM_CAN_USE_DYNAMIC_CAST_IN_UNUSED_TEMPLATE_WITHOUT_DEFINITION
  #define GLIBMM_CAN_ASSIGN_NON_EXTERN_C_FUNCTIONS_TO_EXTERN_C_CALLBACKS
  #define GLIBMM_CAN_USE_NAMESPACES_INSIDE_EXTERNC
  #pragma warning (disable: 4786 4355 4800 4181)
#endif

#ifndef GLIBMM_HAVE_NAMESPACE_STD
#  define GLIBMM_USING_STD(Symbol) namespace std { using ::Symbol; }
#else
#  define GLIBMM_USING_STD(Symbol) /* empty */
#endif

#define GLIBMM_HAVE_ALLOWS_STATIC_INLINE_NPOS 1, Defined if a static member variable may be initialized inline to std::string::npos

#ifdef GLIBMM_DLL
  #if defined(GLIBMM_BUILD) && defined(_WINDLL)
    /* Do not dllexport as it is handled by gendef on MSVC */
    #define GLIBMM_API
  #elif !defined(GLIBMM_BUILD)
    #define GLIBMM_API __declspec(dllimport)
  #else
    /* Build a static library */
    #define GLIBMM_API
  #endif /* GLIBMM_BUILD - _WINDLL */
#else
  #define GLIBMM_API
#endif /* GLIBMM_DLL */

#endif /* _GLIBMM_CONFIG_H */

