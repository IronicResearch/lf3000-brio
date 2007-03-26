#include <ErrorBrio.h>
#include <SystemTypes.h>

tErrType ErrorBrio::lookupBrioErrType(int errFunc)
{
    return (tErrType )errFunc;
}
