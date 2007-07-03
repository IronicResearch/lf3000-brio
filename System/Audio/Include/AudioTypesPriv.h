#ifndef LF_BRIO_AUDIOTYPESPRIV_H_
#define LF_BRIO_AUDIOTYPESPRIV_H_

#include <SystemTypes.h>
#include <AudioTypes.h>

// kAudioCmdMsgTypeSetMasterVolume
struct tAudioMasterVolume {
	U8				 	volume;
};

// kAudioCmdMsgTypeStartAudio
struct tAudioStartAudioInfo { 
	tRsrcHndl			hRsrc;				// Resource Handle, provided by app, returned from FindResource()
	U8				 	volume;
	tAudioPriority	 	priority;
	S8				 	pan;
	IEventListener		*pListener;
	tAudioPayload		payload;
	tAudioOptionsFlags	flags;
	tAudioHeader*		pAudioHeader;		// For Brio Raw types, pointer to header and data, provided by AudioTask, returned from GetPtr()
	
	tAudioStartAudioInfo(tRsrcHndl r = kInvalidRsrcHndl, 
						U8 v = 0, 
						tAudioPriority p = 0, 
						S8 pn = 0, 
						IEventListener* l = NULL, 
						tAudioPayload pl = 0, 
						tAudioOptionsFlags f = 0, 
						tAudioHeader* h = NULL)
		: hRsrc(r), volume(v), priority(p), pan(pn), pListener(l),
		payload(pl), flags(f), pAudioHeader(h) {}
};

// kAudioCmdMsgTypeStopAudio
struct tAudioStopAudioInfo {
	tAudioID			id;
	Boolean				suppressDoneMsg;
};

// kAudioCmdMsgTypePauseAudio
struct tAudioPauseAudioInfo {
	tAudioID			id;
};

// kAudioCmdMsgTypeResumeAudio
struct tAudioResumeAudioInfo {
	tAudioID			id;
};

// kAudioCmdMsgTypeMidiNoteOn
struct tAudioMidiNoteInfo {
	U8					channel;
	U8					noteNum;
	U8				 	velocity;
	tAudioPriority	 	priority;
	tAudioPayload		payload;
	tAudioOptionsFlags	flags;
};

// kAudioCmdMsgTypePlayMidiFile
struct tAudioStartMidiFileInfo {
	tMidiID				id;					// fixme/dg: make midiPlayerID?
	tRsrcHndl			hRsrc;				// Resource Handle, provided by app, returned from FindResource()
	U8*					pMidiFileImage;		// MIDI file loaded into RAM
	U32					imageSize;			// Size of MIDI file in RAM
	U8				 	volume;
	tAudioPriority		priority;
	IEventListener*		pListener;
	tAudioPayload		payload;
	tAudioOptionsFlags	flags;
};

// kAudioCmdMsgTypePauseMidiFile
struct tAudioPauseMidiFileInfo {
	tMidiID			id;
};

// kAudioCmdMsgTypeResumeMidiFile
struct tAudioResumeMidiFileInfo {
	tMidiID			id;
};

// kAudioCmdMsgTypeStopMidiFile
struct tAudioStopMidiFileInfo {
	tMidiID			id;
	Boolean			suppressDoneMsg;
};
#endif /*LF_BRIO_AUDIOTYPESPRIV_H_*/
