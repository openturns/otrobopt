
#ifndef OTROBOPT_PRIVATE_HXX
#define OTROBOPT_PRIVATE_HXX

/* From http://gcc.gnu.org/wiki/Visibility */
/* Generic helper definitions for shared library support */
#if defined _WIN32 || defined __CYGWIN__
#define OTROBOPT_HELPER_DLL_IMPORT __declspec(dllimport)
#define OTROBOPT_HELPER_DLL_EXPORT __declspec(dllexport)
#define OTROBOPT_HELPER_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define OTROBOPT_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
#define OTROBOPT_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
#define OTROBOPT_HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define OTROBOPT_HELPER_DLL_IMPORT
#define OTROBOPT_HELPER_DLL_EXPORT
#define OTROBOPT_HELPER_DLL_LOCAL
#endif
#endif

/* Now we use the generic helper definitions above to define OTROBOPT_API and OTROBOPT_LOCAL.
 * OTROBOPT_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)
 * OTROBOPT_LOCAL is used for non-api symbols. */

#ifndef OTROBOPT_STATIC /* defined if OT is compiled as a DLL */
#ifdef OTROBOPT_DLL_EXPORTS /* defined if we are building the OT DLL (instead of using it) */
#define OTROBOPT_API OTROBOPT_HELPER_DLL_EXPORT
#else
#define OTROBOPT_API OTROBOPT_HELPER_DLL_IMPORT
#endif /* OTROBOPT_DLL_EXPORTS */
#define OTROBOPT_LOCAL OTROBOPT_HELPER_DLL_LOCAL
#else /* OTROBOPT_STATIC is defined: this means OT is a static lib. */
#define OTROBOPT_API
#define OTROBOPT_LOCAL
#endif /* !OTROBOPT_STATIC */


#endif // OTROBOPT_PRIVATE_HXX

