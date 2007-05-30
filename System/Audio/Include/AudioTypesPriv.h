#ifndef LF_BRIO_AUDIOTYPESPRIV_H_
#define LF_BRIO_AUDIOTYPESPRIV_H_

#include <SystemTypes.h>
#include <AudioTypes.h>

// kAudioCmdMsgTypeSetMasterVolume
struct tAudioMasterVolume {
	U8				 	volume;
};

// kAudioCmdMsgTypePlayAudio
struct tAudioPlayAudioInfo {
	tRsrcHndl			hRsrc;				// Resource Handle, provided by app, returned from FindResource()
	tAudioHeader*		pAudioHeader;		// Pointer to header and data, provided by AudioTask, returned from GetRsrcPtr()
	U8				 	volume;
	tAudioPriority	 	priority;
	S8				 	pan;
	IEventListener		*pListener;
	tAudioPayload		payload;
	tAudioOptionsFlags	flags;
};

// kAudioCmdMsgTypeStopAudio
struct tAudioStopAudioInfo {
	tAudioID			audioID;
	U8				 	suppressDoneMsg;
};

// kAudioCmdMsgTypePauseAudio
struct tAudioPauseAudioInfo {
	tAudioID			audioID;
};

// kAudioCmdMsgTypeResumeAudio
struct tAudioResumeAudioInfo {
	tAudioID			audioID;
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
struct tAudioMidiFileInfo {
	tMidiID				midiID;				// fixme/dg: make midiPlayerID?
	tRsrcHndl			hRsrc;				// Resource Handle, provided by app, returned from FindResource()
	U8*					pMidiFileImage;		// MIDI file loaded into RAM
	U32					imageSize;			// Size of MIDI file in RAM
	U8				 	volume;
	tAudioPriority		priority;
	IEventListener*		pListener;
	tAudioPayload		payload;
	tAudioOptionsFlags	flags;
};

#endif /*LF_BRIO_AUDIOTYPESPRIV_H_*/
