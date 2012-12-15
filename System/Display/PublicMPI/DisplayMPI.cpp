//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		DisplayMPI.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio 
//		Display Manager module.
//
//==============================================================================

#include <DisplayMPI.h>
#include <DisplayPriv.h>
#include <Module.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
LF_BEGIN_BRIO_NAMESPACE()


const CString	kMPIName = "DisplayMPI";

S32 CY[256] = {-4768,-4470,-4172,-3874,-3576,-3278,-2980,-2682,-2384,-2086,-1788,-1490,-1192,-894,-596,-298,0,298,596,894,1192,1490,1788,2086,2384,2682,2980,3278,3576,3874,4172,4470,4768,5066,5364,5662,5960,6258,6556,6854,7152,7450,7748,8046,8344,8642,8940,9238,9536,9834,10132,10430,10728,11026,11324,11622,11920,12218,12516,12814,13112,13410,13708,14006,14304,14602,14900,15198,15496,15794,16092,16390,16688,16986,17284,17582,17880,18178,18476,18774,19072,19370,19668,19966,20264,20562,20860,21158,21456,21754,22052,22350,22648,22946,23244,23542,23840,24138,24436,24734,25032,25330,25628,25926,26224,26522,26820,27118,27416,27714,28012,28310,28608,28906,29204,29502,29800,30098,30396,30694,30992,31290,31588,31886,32184,32482,32780,33078,33376,33674,33972,34270,34568,34866,35164,35462,35760,36058,36356,36654,36952,37250,37548,37846,38144,38442,38740,39038,39336,39634,39932,40230,40528,40826,41124,41422,41720,42018,42316,42614,42912,43210,43508,43806,44104,44402,44700,44998,45296,45594,45892,46190,46488,46786,47084,47382,47680,47978,48276,48574,48872,49170,49468,49766,50064,50362,50660,50958,51256,51554,51852,52150,52448,52746,53044,53342,53640,53938,54236,54534,54832,55130,55428,55726,56024,56322,56620,56918,57216,57514,57812,58110,58408,58706,59004,59302,59600,59898,60196,60494,60792,61090,61388,61686,61984,62282,62580,62878,63176,63474,63772,64070,64368,64666,64964,65262,65560,65858,66156,66454,66752,67050,67348,67646,67944,68242,68540,68838,69136,69434,69732,70030,70328,70626,70924,71222};
S32 ERV[256] = {-52352,-51943,-51534,-51125,-50716,-50307,-49898,-49489,-49080,-48671,-48262,-47853,-47444,-47035,-46626,-46217,-45808,-45399,-44990,-44581,-44172,-43763,-43354,-42945,-42536,-42127,-41718,-41309,-40900,-40491,-40082,-39673,-39264,-38855,-38446,-38037,-37628,-37219,-36810,-36401,-35992,-35583,-35174,-34765,-34356,-33947,-33538,-33129,-32720,-32311,-31902,-31493,-31084,-30675,-30266,-29857,-29448,-29039,-28630,-28221,-27812,-27403,-26994,-26585,-26176,-25767,-25358,-24949,-24540,-24131,-23722,-23313,-22904,-22495,-22086,-21677,-21268,-20859,-20450,-20041,-19632,-19223,-18814,-18405,-17996,-17587,-17178,-16769,-16360,-15951,-15542,-15133,-14724,-14315,-13906,-13497,-13088,-12679,-12270,-11861,-11452,-11043,-10634,-10225,-9816,-9407,-8998,-8589,-8180,-7771,-7362,-6953,-6544,-6135,-5726,-5317,-4908,-4499,-4090,-3681,-3272,-2863,-2454,-2045,-1636,-1227,-818,-409,0,409,818,1227,1636,2045,2454,2863,3272,3681,4090,4499,4908,5317,5726,6135,6544,6953,7362,7771,8180,8589,8998,9407,9816,10225,10634,11043,11452,11861,12270,12679,13088,13497,13906,14315,14724,15133,15542,15951,16360,16769,17178,17587,17996,18405,18814,19223,19632,20041,20450,20859,21268,21677,22086,22495,22904,23313,23722,24131,24540,24949,25358,25767,26176,26585,26994,27403,27812,28221,28630,29039,29448,29857,30266,30675,31084,31493,31902,32311,32720,33129,33538,33947,34356,34765,35174,35583,35992,36401,36810,37219,37628,38037,38446,38855,39264,39673,40082,40491,40900,41309,41718,42127,42536,42945,43354,43763,44172,44581,44990,45399,45808,46217,46626,47035,47444,47853,48262,48671,49080,49489,49898,50307,50716,51125,51534,51943};
S16 DGU[256] = {-12800,-12700,-12600,-12500,-12400,-12300,-12200,-12100,-12000,-11900,-11800,-11700,-11600,-11500,-11400,-11300,-11200,-11100,-11000,-10900,-10800,-10700,-10600,-10500,-10400,-10300,-10200,-10100,-10000,-9900,-9800,-9700,-9600,-9500,-9400,-9300,-9200,-9100,-9000,-8900,-8800,-8700,-8600,-8500,-8400,-8300,-8200,-8100,-8000,-7900,-7800,-7700,-7600,-7500,-7400,-7300,-7200,-7100,-7000,-6900,-6800,-6700,-6600,-6500,-6400,-6300,-6200,-6100,-6000,-5900,-5800,-5700,-5600,-5500,-5400,-5300,-5200,-5100,-5000,-4900,-4800,-4700,-4600,-4500,-4400,-4300,-4200,-4100,-4000,-3900,-3800,-3700,-3600,-3500,-3400,-3300,-3200,-3100,-3000,-2900,-2800,-2700,-2600,-2500,-2400,-2300,-2200,-2100,-2000,-1900,-1800,-1700,-1600,-1500,-1400,-1300,-1200,-1100,-1000,-900,-800,-700,-600,-500,-400,-300,-200,-100,0,100,200,300,400,500,600,700,800,900,1000,1100,1200,1300,1400,1500,1600,1700,1800,1900,2000,2100,2200,2300,2400,2500,2600,2700,2800,2900,3000,3100,3200,3300,3400,3500,3600,3700,3800,3900,4000,4100,4200,4300,4400,4500,4600,4700,4800,4900,5000,5100,5200,5300,5400,5500,5600,5700,5800,5900,6000,6100,6200,6300,6400,6500,6600,6700,6800,6900,7000,7100,7200,7300,7400,7500,7600,7700,7800,7900,8000,8100,8200,8300,8400,8500,8600,8700,8800,8900,9000,9100,9200,9300,9400,9500,9600,9700,9800,9900,10000,10100,10200,10300,10400,10500,10600,10700,10800,10900,11000,11100,11200,11300,11400,11500,11600,11700,11800,11900,12000,12100,12200,12300,12400,12500,12600,12700};
S16 EGV[256] = {-26624,-26416,-26208,-26000,-25792,-25584,-25376,-25168,-24960,-24752,-24544,-24336,-24128,-23920,-23712,-23504,-23296,-23088,-22880,-22672,-22464,-22256,-22048,-21840,-21632,-21424,-21216,-21008,-20800,-20592,-20384,-20176,-19968,-19760,-19552,-19344,-19136,-18928,-18720,-18512,-18304,-18096,-17888,-17680,-17472,-17264,-17056,-16848,-16640,-16432,-16224,-16016,-15808,-15600,-15392,-15184,-14976,-14768,-14560,-14352,-14144,-13936,-13728,-13520,-13312,-13104,-12896,-12688,-12480,-12272,-12064,-11856,-11648,-11440,-11232,-11024,-10816,-10608,-10400,-10192,-9984,-9776,-9568,-9360,-9152,-8944,-8736,-8528,-8320,-8112,-7904,-7696,-7488,-7280,-7072,-6864,-6656,-6448,-6240,-6032,-5824,-5616,-5408,-5200,-4992,-4784,-4576,-4368,-4160,-3952,-3744,-3536,-3328,-3120,-2912,-2704,-2496,-2288,-2080,-1872,-1664,-1456,-1248,-1040,-832,-624,-416,-208,0,208,416,624,832,1040,1248,1456,1664,1872,2080,2288,2496,2704,2912,3120,3328,3536,3744,3952,4160,4368,4576,4784,4992,5200,5408,5616,5824,6032,6240,6448,6656,6864,7072,7280,7488,7696,7904,8112,8320,8528,8736,8944,9152,9360,9568,9776,9984,10192,10400,10608,10816,11024,11232,11440,11648,11856,12064,12272,12480,12688,12896,13104,13312,13520,13728,13936,14144,14352,14560,14768,14976,15184,15392,15600,15808,16016,16224,16432,16640,16848,17056,17264,17472,17680,17888,18096,18304,18512,18720,18928,19136,19344,19552,19760,19968,20176,20384,20592,20800,21008,21216,21424,21632,21840,22048,22256,22464,22672,22880,23088,23296,23504,23712,23920,24128,24336,24544,24752,24960,25168,25376,25584,25792,26000,26208,26416};
S32 DBU[256] = {-66048,-65532,-65016,-64500,-63984,-63468,-62952,-62436,-61920,-61404,-60888,-60372,-59856,-59340,-58824,-58308,-57792,-57276,-56760,-56244,-55728,-55212,-54696,-54180,-53664,-53148,-52632,-52116,-51600,-51084,-50568,-50052,-49536,-49020,-48504,-47988,-47472,-46956,-46440,-45924,-45408,-44892,-44376,-43860,-43344,-42828,-42312,-41796,-41280,-40764,-40248,-39732,-39216,-38700,-38184,-37668,-37152,-36636,-36120,-35604,-35088,-34572,-34056,-33540,-33024,-32508,-31992,-31476,-30960,-30444,-29928,-29412,-28896,-28380,-27864,-27348,-26832,-26316,-25800,-25284,-24768,-24252,-23736,-23220,-22704,-22188,-21672,-21156,-20640,-20124,-19608,-19092,-18576,-18060,-17544,-17028,-16512,-15996,-15480,-14964,-14448,-13932,-13416,-12900,-12384,-11868,-11352,-10836,-10320,-9804,-9288,-8772,-8256,-7740,-7224,-6708,-6192,-5676,-5160,-4644,-4128,-3612,-3096,-2580,-2064,-1548,-1032,-516,0,516,1032,1548,2064,2580,3096,3612,4128,4644,5160,5676,6192,6708,7224,7740,8256,8772,9288,9804,10320,10836,11352,11868,12384,12900,13416,13932,14448,14964,15480,15996,16512,17028,17544,18060,18576,19092,19608,20124,20640,21156,21672,22188,22704,23220,23736,24252,24768,25284,25800,26316,26832,27348,27864,28380,28896,29412,29928,30444,30960,31476,31992,32508,33024,33540,34056,34572,35088,35604,36120,36636,37152,37668,38184,38700,39216,39732,40248,40764,41280,41796,42312,42828,43344,43860,44376,44892,45408,45924,46440,46956,47472,47988,48504,49020,49536,50052,50568,51084,51600,52116,52632,53148,53664,54180,54696,55212,55728,56244,56760,57276,57792,58308,58824,59340,59856,60372,60888,61404,61920,62436,62952,63468,63984,64500,65016,65532};

//============================================================================
// CDisplayMPI
//============================================================================
//----------------------------------------------------------------------------
CDisplayMPI::CDisplayMPI() : pModule_(NULL)
{
	ICoreModule*	pModule;
	Module::Connect(pModule, kDisplayModuleName, kDisplayModuleVersion);
	pModule_ = reinterpret_cast<CDisplayModule*>(pModule);
}

//----------------------------------------------------------------------------
CDisplayMPI::~CDisplayMPI()
{
	Module::Disconnect(pModule_);
}

//----------------------------------------------------------------------------
Boolean	CDisplayMPI::IsValid() const
{
	return (pModule_ != NULL) ? true : false;
}

//----------------------------------------------------------------------------
const CString* CDisplayMPI::GetMPIName() const
{
	return &kMPIName;
}

//----------------------------------------------------------------------------
tVersion CDisplayMPI::GetModuleVersion() const
{
	if(!pModule_)
		return kUndefinedVersion;
	return pModule_->GetModuleVersion();
}

//----------------------------------------------------------------------------
const CString* CDisplayMPI::GetModuleName() const
{
	if(!pModule_)
		return &kNullString;
	return pModule_->GetModuleName();
}

//----------------------------------------------------------------------------
const CURI* CDisplayMPI::GetModuleOrigin() const
{
	if(!pModule_)
		return &kNullURI;
	return pModule_->GetModuleOrigin();
}


//============================================================================
//----------------------------------------------------------------------------
U16 CDisplayMPI::GetNumberOfScreens() const
{
	if(!pModule_)
		return 0;
	return pModule_->GetNumberOfScreens();
}

//----------------------------------------------------------------------------
const tDisplayScreenStats* CDisplayMPI::GetScreenStats(tDisplayScreen screen) const
{
	if(!pModule_)
		return NULL;
	return pModule_->GetScreenStats(screen);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::Invalidate(tDisplayScreen screen, tRect *pDirtyRect)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	return pModule_->Invalidate(screen, pDirtyRect);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::SetBrightness(tDisplayScreen screen, S8 brightness)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SetBrightness(screen, brightness);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::SetContrast(tDisplayScreen screen, S8 contrast)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SetContrast(screen, contrast);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::SetBacklight(tDisplayScreen screen, S8 backlight)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SetBacklight(screen, backlight);
}

//----------------------------------------------------------------------------
S8	CDisplayMPI::GetBrightness(tDisplayScreen screen)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->GetBrightness(screen);
}

//----------------------------------------------------------------------------
S8	CDisplayMPI::GetContrast(tDisplayScreen screen)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->GetContrast(screen);
}

//----------------------------------------------------------------------------
S8	CDisplayMPI::GetBacklight(tDisplayScreen screen)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->GetBacklight(screen);
}

//============================================================================
//----------------------------------------------------------------------------
tDisplayHandle CDisplayMPI::CreateHandle(U16 height, U16 width, 
										tPixelFormat colorDepth, U8 *pBuffer)
{
	if(!pModule_ || !height || !width || !colorDepth)
		return kInvalidDisplayHandle;
	return pModule_->CreateHandle(height, width, colorDepth, pBuffer);
}

//----------------------------------------------------------------------------
U8* CDisplayMPI::GetBuffer(tDisplayHandle hndl) const
{
	if(!pModule_ || !hndl)
		return NULL;
	return pModule_->GetBuffer(hndl);
}

//----------------------------------------------------------------------------
U16 CDisplayMPI::GetHeight(tDisplayHandle hndl) const
{
	if(!pModule_ || !hndl)
		return 0;
	return pModule_->GetHeight(hndl);
}

//----------------------------------------------------------------------------
U16 CDisplayMPI::GetWidth(tDisplayHandle hndl) const
{
	if(!pModule_ || !hndl)
		return 0;
	return pModule_->GetWidth(hndl);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::Register(tDisplayHandle hndl, S16 xPos, S16 yPos, 
								tDisplayHandle insertAfter, tDisplayScreen screen)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	if (!hndl)
		return kInvalidParamErr;
	return pModule_->Register(hndl, xPos, yPos, insertAfter, screen);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::Register(tDisplayHandle hndl, S16 xPos, S16 yPos, 
							 tDisplayZOrder initialZOrder, 
                             tDisplayScreen screen)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	if (!hndl)
		return kInvalidParamErr;
	return pModule_->Register(hndl, xPos, yPos, initialZOrder, screen);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::UnRegister(tDisplayHandle hndl, tDisplayScreen screen)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	if (!hndl)
		return kInvalidParamErr;
	return pModule_->UnRegister(hndl, screen);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::DestroyHandle(tDisplayHandle hndl, Boolean destroyBuffer)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	if (!hndl)
		return kInvalidParamErr;
	return pModule_->DestroyHandle(hndl, destroyBuffer);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::LockBuffer(tDisplayHandle hndl)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	if (!hndl)
		return kInvalidParamErr;
	return pModule_->LockBuffer(hndl);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::UnlockBuffer(tDisplayHandle hndl, tRect *pDirtyRect)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	if (!hndl)
		return kInvalidParamErr;
	return pModule_->UnlockBuffer(hndl, pDirtyRect);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::SwapBuffers(tDisplayHandle hndl, Boolean waitVSync)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	if (!hndl)
		return kInvalidParamErr;
	return pModule_->SwapBuffers(hndl, waitVSync);
}

//----------------------------------------------------------------------------
Boolean CDisplayMPI::IsBufferSwapped(tDisplayHandle hndl)
{
	if(!pModule_ || !hndl)
		return false;
	return pModule_->IsBufferSwapped(hndl);
}

//----------------------------------------------------------------------------
tDisplayHandle CDisplayMPI::GetCurrentDisplayHandle()
{
	if(!pModule_)
		return kInvalidDisplayHandle;
	return pModule_->GetCurrentDisplayHandle();
}

//----------------------------------------------------------------------------
tDisplayHandle CDisplayMPI::GetCurrentDisplayHandle(tPixelFormat pixelformat)
{
	if(!pModule_)
		return kInvalidDisplayHandle;
	return pModule_->GetCurrentDisplayHandle(pixelformat);
}

//----------------------------------------------------------------------------
tPixelFormat CDisplayMPI::GetPixelFormat(tDisplayHandle hndl) const
{
	if(!pModule_ || !hndl)
		return kPixelFormatError;
	return pModule_->GetPixelFormat(hndl);
}

//----------------------------------------------------------------------------
U16 CDisplayMPI::GetPitch(tDisplayHandle hndl) const
{
	if(!pModule_ || !hndl)
		return 0;
	return pModule_->GetPitch(hndl);
}

//----------------------------------------------------------------------------
U16 CDisplayMPI::GetDepth(tDisplayHandle hndl) const
{
	if(!pModule_ || !hndl)
		return 0;
	return pModule_->GetDepth(hndl);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::SetAlpha(tDisplayHandle hndl, U8 level, Boolean enable)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	if (!hndl)
		return kInvalidParamErr;
	return pModule_->SetAlpha(hndl, level, enable);
}

//----------------------------------------------------------------------------
U8 CDisplayMPI::GetAlpha(tDisplayHandle hndl) const
{
	if(!pModule_ || !hndl)
		return 0;
	return pModule_->GetAlpha(hndl);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::SetWindowPosition(tDisplayHandle hndl, S16 x, S16 y, U16 width, U16 height, Boolean visible)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	if (!hndl)
		return kInvalidParamErr;
	return pModule_->SetWindowPosition(hndl, x, y, width, height, visible);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::GetWindowPosition(tDisplayHandle hndl, S16& x, S16& y, U16& width, U16& height, Boolean& visible)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	if (!hndl)
		return kInvalidParamErr;
	return pModule_->GetWindowPosition(hndl, x, y, width, height, visible);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::SetVideoScaler(tDisplayHandle hndl, U16 width, U16 height, Boolean centered)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	if (!hndl)
		return kInvalidParamErr;
	return pModule_->SetVideoScaler(hndl, width, height, centered);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::GetVideoScaler(tDisplayHandle hndl, U16& width, U16& height, Boolean& centered)
{
	if(!pModule_)
		return kMPINotConnectedErr;
	if (!hndl)
		return kInvalidParamErr;
	return pModule_->GetVideoScaler(hndl, width, height, centered);
}

//----------------------------------------------------------------------------
tPixelFormat CDisplayMPI::GetAvailableFormat()
{
	if(!pModule_)
		return kPixelFormatError;
	return pModule_->GetAvailableFormat();
}
//----------------------------------------------------------------------------
void CDisplayMPI::InitOpenGL(void* pCtx)
{
	if (!pModule_)
		return;
	pModule_->InitOpenGL(pCtx);
}

//----------------------------------------------------------------------------
void CDisplayMPI::DeinitOpenGL()
{
	if (!pModule_)
		return;
	pModule_->DeinitOpenGL(NULL);
}

//----------------------------------------------------------------------------
void CDisplayMPI::DeinitOpenGL(void* pCtx)
{
	if (!pModule_)
		return;
	pModule_->DeinitOpenGL(pCtx);
}

//----------------------------------------------------------------------------
void CDisplayMPI::EnableOpenGL(void* pCtx)
{
	if (!pModule_)
		return;
	pModule_->EnableOpenGL(pCtx);
}

//----------------------------------------------------------------------------
void CDisplayMPI::UpdateOpenGL()
{
	if (!pModule_)
		return;
	pModule_->UpdateOpenGL(NULL);
}

//----------------------------------------------------------------------------
void CDisplayMPI::UpdateOpenGL(void* pCtx)
{
	if (!pModule_)
		return;
	pModule_->UpdateOpenGL(pCtx);
}

//----------------------------------------------------------------------------
void CDisplayMPI::DisableOpenGL()
{
	if (!pModule_)
		return;
	pModule_->DisableOpenGL(NULL);
}

//----------------------------------------------------------------------------
void CDisplayMPI::DisableOpenGL(void* pCtx)
{
	if (!pModule_)
		return;
	pModule_->DisableOpenGL(pCtx);
}

//----------------------------------------------------------------------------
void CDisplayMPI::WaitForDisplayAddressPatched()
{
	if (!pModule_)
		return;
	pModule_->WaitForDisplayAddressPatched();
}

void CDisplayMPI::SetOpenGLDisplayAddress(const unsigned int DisplayBufferPhysicalAddress)
{
	if (!pModule_)
		return;
	pModule_->SetOpenGLDisplayAddress(DisplayBufferPhysicalAddress);
}

//----------------------------------------------------------------------------
U32	CDisplayMPI::GetDisplayMem(tDisplayMem memtype)
{
	if (!pModule_)
		return 0;
	return pModule_->GetDisplayMem(memtype);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::SetViewport(tDisplayHandle hndl, tDisplayViewport viewport)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SetViewport(hndl, viewport);
}

//----------------------------------------------------------------------------
tDisplayViewport CDisplayMPI::GetViewport(tDisplayHandle hndl)
{
	if (!pModule_)
		return static_cast<tDisplayViewport>(0);
	return pModule_->GetViewport(hndl);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::SetViewport(tDisplayHandle hndl, S16 x, S16 y, U16 width, U16 height)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SetViewport(hndl, x, y, width, height);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::SetViewport(tDisplayHandle hndl, S16 x, S16 y, U16 width, U16 height, U16 owidth, U16 oheight)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SetViewport(hndl, x, y, width, height, owidth, oheight);
}
//----------------------------------------------------------------------------
tErrType CDisplayMPI::GetViewport(tDisplayHandle hndl, S16& x, S16& y, U16& width, U16& height)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->GetViewport(hndl, x, y, width, height);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::GetViewport(tDisplayHandle hndl, S16& x, S16& y, U16& width, U16& height, U16& owidth, U16& oheight)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->GetViewport(hndl, x, y, width, height, owidth, oheight);
}
//----------------------------------------------------------------------------
tErrType CDisplayMPI::SetOrientation(tDisplayHandle hndl, tDisplayOrientation orient)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SetOrientation(hndl, orient);
}

//----------------------------------------------------------------------------
tDisplayOrientation	CDisplayMPI::GetOrientation(tDisplayHandle hndl)
{
	if (!pModule_)
		return static_cast<tDisplayOrientation>(0);
	return pModule_->GetOrientation(hndl);
}

//----------------------------------------------------------------------------
tErrType CDisplayMPI::SetAutoRotation(Boolean enable)
{
	if (!pModule_)
		return kMPINotConnectedErr;
	return pModule_->SetAutoRotation(enable);
}

//----------------------------------------------------------------------------
Boolean CDisplayMPI::GetAutoRotation()
{
	if (!pModule_)
		return false;
	return pModule_->GetAutoRotation();
}
//----------------------------------------------------------------------------
EGLClientBuffer CDisplayMPI::CreateEglClientBuffer(tDisplayHandle hndl)
{
	if (!pModule_)
		return 0;
	return pModule_->CreateEglClientBuffer(hndl);
}
//----------------------------------------------------------------------------
void CDisplayMPI::DestroyEglClientBuffer(EGLClientBuffer egl_client_buffer)
{
	if (!pModule_)
		return;
	return pModule_->DestroyEglClientBuffer(egl_client_buffer);
}
LF_END_BRIO_NAMESPACE()
// EOF
