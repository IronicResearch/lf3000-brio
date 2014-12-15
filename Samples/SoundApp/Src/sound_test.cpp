#include "sound_test.h"
#include "EgPickerState.h"
#include "EgApp.h"

#include <AudioMPI.h>

#include <CSystemData.h>
#include <Fixed.h>
#include <AppManager.h>
#include <LogFile.h>
#include <CGraphics2D.h>
#include <StringTypes.h>
#include <GameStateHandler.h>
#include <KeyboardManager.h>

#include <ButtonMPI.h>
#include <EventMPI.h>
#include <BaseUtils.h>
#include <CDrawManager.h>
#include <time.h>

#define  NUM_SPRITES 8

typedef struct
{
    tAudioID         audio_id;
    char            path[1024];
    char            is_playing;
    unsigned char  priority;
} SOUND;

/*********************************************************************/


	CBillboard					* sprites[NUM_SPRITES];
	CBillboard                  *scroll_1, *scroll_2;

   int                        x1,y1,x2,y2;

	CText					   *channel_indicator;
	CText					   *sine_text;
	CText                      *fps_display;
	CAudioMPI                  audioMPI;
	SOUND                       adpcm[17];
	SOUND                       ogg[4];
	tAudioID                   wav_sounds[16];
	tAudioID                   ogg_sounds[3];
	
	tAudioID bgm;

LF_USING_BRIO_NAMESPACE()

SOUND_TEST* SOUND_TEST::mState = NULL;



/*********************************************************************
*
*
*/

SOUND_TEST::SOUND_TEST()
{
}

/*********************************************************************
*
*
*/

SOUND_TEST::~SOUND_TEST()
{
}

/*********************************************************************
*
*
*/
void SOUND_TEST::prepare_gfx(void)
{
   int  i;
   char buffer[1024];

    // load images
   for(i = 0; i < NUM_SPRITES; i++)
   {
      sprites[i] = new CBillboard();
      sprintf(buffer,"%simages/texture%i.png",CSystemData::Instance()->GetCurrentTitlePath().c_str(),i+1);
      sprites[i]->loadTexture(buffer);
   }
   
   scroll_1 = new CBillboard();
   sprintf(buffer,"%simages/sky1.png",CSystemData::Instance()->GetCurrentTitlePath().c_str());
   scroll_1->loadTexture(buffer);
   
   scroll_2 = new CBillboard();
   sprintf(buffer,"%simages/sky2.png",CSystemData::Instance()->GetCurrentTitlePath().c_str());
   scroll_2->loadTexture(buffer);
   
   // prepare sound pathnames
   for(i = 0; i < 17; i++)
   {
       sprintf(adpcm[i].path, "%ssounds/%02i.wav",CSystemData::Instance()->GetCurrentTitlePath().c_str(),i);
       adpcm[i].is_playing = 0;
   }       
       
   fps_display        = new CText();
   channel_indicator  = new CText();
   sine_text          = new CText();
}

/*********************************************************************
*
*
*/
void SOUND_TEST::delete_gfx(void)
{
   int i;

   for(i = 0; i < NUM_SPRITES; i++)
   {
      delete(sprites[i]);
   }
   
   delete(scroll_1);
   delete(scroll_2);

   delete(fps_display); 
   delete(channel_indicator); 	
   delete(sine_text); 	
}


/*********************************************************************
*
*
*/
void SOUND_TEST::Enter(CStateData* userData)
{
   int   i;
   char  buffer[1024];

   prepare_gfx();


		bgm = audioMPI.StartAudio(
		    CSystemData::Instance()->GetCurrentTitlePath()+"sounds/roygbiv.ogg", 
            60, 
            1, 
            0, 
            NULL,	
            kAudioRepeat_Infinite, kAudioOptionsLooped);

  	CPath myFontPath(CSystemData::Instance()->FindFont("DidjPropBold.ttf"));
	
	KeyboardManager::Instance()->clearButtonPressedMask(kButtonsAll);


	fps_display->setFont(myFontPath);
	fps_display->setSize(24);
	fps_display->setColor(tColor(255, 127, 192, 255));
	fps_display->moveTo(tVect3(160, 200 ,10));

	channel_indicator->setFont(myFontPath);
	channel_indicator->setSize(16);
	channel_indicator->setColor(tColor(255, 255, 255, 255));
	channel_indicator->moveTo(tVect3(16, 16 ,10));

	sine_text->setFont(myFontPath);
	sine_text->setSize(16);
	sine_text->setColor(tColor(127, 127, 255, 255));
	sine_text->moveTo(tVect3(16, 32 ,10));
}

/*********************************************************************
*
*
*/
void SOUND_TEST::Exit()
{

	audioMPI.StopAudio(bgm, 1);

   delete_gfx();

	// Log the event
	if (CAppManager::Instance()->pLogFile->IsProfileSet())
	{
		// TODO: score and reason
		LogData::Lightning::CActivityExit activityExit(0,0);
		CAppManager::Instance()->pLogFile->Append(*(dynamic_cast<CLogData*>(&activityExit)));
	}
}


/*********************************************************************
*
*
*/
void SOUND_TEST::Update(CGameStateHandler* sh)
{
   static int channel_number;
   
   static int A_held;
   static int UP_held;
   static int DN_held;
   
   static time_t       old_time = time(NULL);
   static time_t       new_time;

   static int updates; 
   
   double      frames_per_sec;

   static int scroll;
   static int framecount;
   int i;
   char buffer[1024];
   
   

	KeyboardManager* keyManager = KeyboardManager::Instance();
	U32 keyboardKeysPressed = keyManager->getButtonPressedMask();
	U32 keyboardKeysHeld = keyManager->getButtonDownMask();

    framecount++;

   	CGraphics2D::Instance()->clearBuffer();

    // draw sky
    scroll++;
    if(scroll > 511) scroll = 0;
    
    scroll_1->moveTo(tVect3(scroll >> 1 , 0,1));
    scroll_1->draw();    

    scroll_1->moveTo(tVect3((scroll >> 1 )+256, 0,1));
    scroll_1->draw();    

    scroll_1->moveTo(tVect3((scroll >> 1 )-256, 0,1));
    scroll_1->draw();    

    scroll_2->moveTo(tVect3(scroll, 0,2));
    scroll_2->draw();    

    scroll_2->moveTo(tVect3(scroll-256, 0,2));
    scroll_2->draw();    

    scroll_2->moveTo(tVect3(scroll+256, 0,2));
    scroll_2->draw();    



   for(i = 0; i < NUM_SPRITES; i++)
   {
      // this is really pretty silly and should be fixed.
      switch(i)
      {
          case 0: sprites[i]->moveTo(tVect3(512-((framecount * 3) % 512), i << 5,3)); break;
          case 1: sprites[i]->moveTo(tVect3(512-(framecount % 512), i << 5,4)); break;
          case 2: sprites[i]->moveTo(tVect3(((framecount << 1) % 512)-128, scroll,5)); break;
          case 3: sprites[i]->moveTo(tVect3(((framecount << 2) % 512)-128, scroll >> 1,6)); break;
          case 4: sprites[i]->moveTo(tVect3(512-(framecount % 512), i << 5,4)); break;
          case 5: sprites[i]->moveTo(tVect3(10,scroll,5)); break;
          case 6: sprites[i]->moveTo(tVect3(((framecount << 2) % 512)-128, scroll >> 1,6)); break;
          case 7: sprites[i]->moveTo(tVect3(((framecount << 2) % 512)-128, 256 - scroll >> 1,6)); break;
      }
         
      sprites[i]->draw();   

   }


   sprintf(buffer,"ADPCM channel %i selected",channel_number);
   channel_indicator->setText(buffer);
   channel_indicator->draw();

   if(adpcm[channel_number].is_playing)
   { 
      sprintf(buffer,"playing...");
      sine_text->setText(buffer);
      sine_text->draw();  
   }
   
   // count fps - sort of
   updates++;
   
   if(updates > 50)
   {
       updates = 0;
       new_time = time(NULL);
       frames_per_sec = 50 / (difftime(new_time,old_time)) ;
       old_time = new_time; 

       sprintf(buffer,"fps: %.2f",frames_per_sec);
       
       fps_display->setText(buffer); 
   }

   fps_display->draw();     

   CGraphics2D::Instance()->swapBuffer();


   ////////////////////////////////////////////////////////////////////////////   
 	if (keyboardKeysPressed & kButtonA)
	{
      if(!A_held)
      {
         if(adpcm[channel_number].is_playing)
         {
             audioMPI.StopAudio(adpcm[channel_number].audio_id,false);
             adpcm[channel_number].is_playing = 0;
         }
         else
         {
            adpcm[channel_number].audio_id = audioMPI.StartAudio(
		      adpcm[channel_number].path, 
            63, 
            60 - channel_number, 
            (channel_number << 7) + 64, 
            NULL,	
            kAudioRepeat_Infinite, kAudioOptionsLooped);
            
            adpcm[channel_number].is_playing = 1;
         }
      }
      
      A_held = true;
	}
	else A_held = false;

   ////////////////////////////////////////////////////////////////////////////   
 	if (keyboardKeysPressed & kButtonUp)
	{
      if(!UP_held)
      {
         channel_number++;
      }
      
      UP_held = true;
	}
   else UP_held = false;
   ////////////////////////////////////////////////////////////////////////////   
 	if (keyboardKeysPressed & kButtonDown)
	{
      if(!DN_held)
      {
         channel_number--;
      }
      
      DN_held = true;
	}
   else DN_held = false;
    
    if(channel_number < 0) channel_number = 16;
    if(channel_number > 16) channel_number = 0;

	if (keyboardKeysPressed & kButtonB)
	{
		    		EgApp::RequestNewState(EgPickerState::Instance());
	}


	if (keyboardKeysPressed & kButtonB)
	{
		    		EgApp::RequestNewState(EgPickerState::Instance());
	}

    keyManager->clearButtonPressedMask(keyboardKeysPressed);

}

