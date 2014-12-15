#ifndef STUBAPP_H_
#define STUBAPP_H_

//==============================================================================
// Copyright 2012(c) LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// \file STUBApp.h
//
// This is a boilerplate class, that is meant to serve as a style guide
// as well as a starting point for your project.
//
//
// It defines the base app class for entry and exit into the app as well as
// handling the receiving of user data and class states.
//
// author: 	leapfrog
//  			tmartin - 10/19/12 - cleaned up code and indicated coding style.
//
//==============================================================================

#include <AppInterface.h>
#include <DebugMPI.h>
#include <GameStateHandler.h>
#include <LTM.h>

 /// Debug levels:
 ///
 /// example: DEBUG_LEVEL kDbgLvlVerbose
 ///
 /// \def kDbgLvlSilent
 /// \def kDbgLvlCritical
 /// \def kDbgLvlImportant
 /// \def kDbgLvlValuable
 /// \def kDbgLvlNoteable
 /// \def kDbgLvlVerbose
 /// \def kMaxDebugLevel
 
#define DEBUG_LEVEL kDbgLvlNoteable

//==============================================================================
//  STUBApp class.
//
//  This is a boilerplate stub class meant to be used as a starting point for
//  starting your project and/or defining a new class.
//==============================================================================
class STUBApp : public CAppInterface
{
public:
	 ///  STUBApp constructor
	 ///
	 ///  Creates an app instance.  Instead of allocating resources here, do it in the Enter() method.
	 ///
	 ///  @param userData
	STUBApp(void *userData);

	 ///  STUBApp destructor
	 ///
	 ///  Standard destructor, deallocates memory and cleanup of class object and members when the object is destroyed.
	~STUBApp();

	 /// This is called by the AppManager when the game or title is launched.  It is recommended that any resource allocation should be done here
	 /// rather then in the constructor.
	void Enter();

	 /// The AppManager calls this function frequently during the life of the game.  This provides a tick or heart-beat to the game.  Any calls to
	 /// CAppManager::PushApp() should be called from this method, or functions called by Update().
	void Update();

	 /// The AppManager calls this function upon shutting down the game.  The game should handle all logging first, before any other saving or clean-up.
	 /// This will help insulate against problems in the log files resulting from crashes while saving game data or attempting to clean up.
	 ///
	 /// Order of operations:
	 /// 1)Log TitleExit event
	 /// 2)Save game data
	 /// 3)If using OpenGL, clean up and destroy the OGL context
	void Exit();

	 /// Called by AppManager to pause the app and load another state on the app stack
	 /// such as a tutorial or the pause screen.
	void Suspend();

	 /// Called by AppManager when returning to the app from another state such
	 /// as a tutorial or the pause screen.
	void Resume();

	 /// @param state
	static void RequestNewState(CGameStateBase* state);

	//==============================================================================
	// Getters
	//==============================================================================
	LeapFrog::Brio::U32		getProductId()			{ return product_id_; }
	char *					getPartNumber()			{ return const_cast<char *>(part_number_.c_str()); }
	char *					getProductVersion()		{ return const_cast<char *>(pkg_version_.c_str()); }
	LeapFrog::Brio::U8		getLogDataVersion()		{ return log_data_version_; }
	tPlayerID				getPlayerId()			{ return player_id_; }
	LTM::CPlayerProfile*	getPlayerProfile()		{ return player_profile_;}


protected:
	LeapFrog::Brio::CDebugMPI			debug_mpi_;					///< manages debug output
	CGameStateHandler *					state_handler_;				///< manages the state of the app

	static bool							new_state_;					///< tracks whether there has been a state change
	static CGameStateBase*				new_state_requested_;    	///< pointer to the up-coming state object
	static CGameStateBase*				current_state_;				///< pointer to the current state object

	LeapFrog::Brio::U32					product_id_;				///< The Product ID for the title is a unique ID that is obtained from Dev Studio
	LeapFrog::Brio::CString				part_number_;				///< ID of physical part on which the package is stored at manufacturing.
	LeapFrog::Brio::CString				pkg_version_;				///< <prefix>-<ProductID>-000000 - Replace <prefix> LST3 for Leapster, LPAD for Leappad.
																	//	Replace <ProductID> with the Product ID obtained from Dev Studio.  The last 6 digits are
																	//	always 000000 for cartridges.
	LeapFrog::Brio::U8					log_data_version_;			///<
	tPlayerID							player_id_;					///< Id of player owning the data folder.  kPlayerID_ALL is shared among all player profiles.
																	// 	This is an optional parameter, if not set or equal to kPlayerID_CURRENT then the current player
																	// 	id is used.
	LTM::CPlayerProfile*				player_profile_;			///<

};


#endif // STUBAPP_H_

