#include <DisplayTypes.h>
#include <EmulationConfig.h>
#include <SystemTypes.h>
#include <StringTypes.h>

#include <DebugMPI.h>
#include <FontMPI.h>

#include <math.h>

#include "2d.h"


LF_BEGIN_BRIO_NAMESPACE()

// ============================================================================
// SetUp2D:  Create display surface to draw on
// ============================================================================
    tDisplayHandle
SetUp2D(CDisplayMPI *displayMPI, int left, int top, int width, int height)
{
tDisplayHandle handle = displayMPI->CreateHandle(height, width, kPixelFormatARGB8888, NULL);
displayMPI->Register(handle, left, top, 0, 0);

// Set alpha channel for display surface transparency
//	displayMPI->SetAlpha(handle, ALPHA, false); //true);

// Update display
//displayMPI->Invalidate(0, NULL);

return (handle);
}   // ---- end SetUp2D() ----

LF_END_BRIO_NAMESPACE()


