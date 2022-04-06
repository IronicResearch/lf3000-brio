// TestDisplayMPI.h

#include <cxxtest/TestSuite.h>
#include <UnitTestUtils.h>
#include <WirelessMPI.h>

#include <arpa/inet.h>

LF_USING_BRIO_NAMESPACE()

//============================================================================
// MyEventListener
//============================================================================
const tEventType kMyHandledTypes[] = { kAllWirelessEvents };

class MyWirelessEventListener : public IEventListener
{
public:
	MyWirelessEventListener( ) : IEventListener(kMyHandledTypes, ArrayCount(kMyHandledTypes)) {}
	
	virtual tEventStatus Notify( const IEventMessage &msg )
	{
		if( msg.GetEventType() == kRemotePlayerChanged )
		{
			U16 size		= msg.GetSizeInBytes();
			TS_ASSERT_EQUALS( size, sizeof(CRemotePlayerMessage) );
			
			const CRemotePlayerMessage& player_msg = reinterpret_cast<const CRemotePlayerMessage&>(msg);
			printf("Player %s %s\n", player_msg.GetPlayer().hostname.c_str(), player_msg.PlayerAdded() ? "found" : "left" );
		}
		
		return kEventStatusOKConsumed;
	}
};

//============================================================================
// TestDisplayMPI functions
//============================================================================
class TestWireless : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CWirelessMPI*	pWirelessMPI_;
public:
	//------------------------------------------------------------------------
	void setUp( )
	{
		pWirelessMPI_ = new CWirelessMPI;
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete pWirelessMPI_; 
	}

	//------------------------------------------------------------------------
	void testWasCreated( )
	{
		PRINT_TEST_NAME();
		
		TS_ASSERT( pWirelessMPI_ != NULL );
		TS_ASSERT( pWirelessMPI_->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		PRINT_TEST_NAME();
		
		tVersion		version;
		const CString*	pName;
		const CURI*		pURI;
		
		if ( pWirelessMPI_->IsValid() ) {
			pName = pWirelessMPI_->GetMPIName();
			TS_ASSERT_EQUALS( *pName, "WirelessMPI" );
			version = pWirelessMPI_->GetModuleVersion();
			TS_ASSERT_EQUALS( version, 2 );
			pName = pWirelessMPI_->GetModuleName();
			TS_ASSERT_EQUALS( *pName, "Wireless" );
			pURI = pWirelessMPI_->GetModuleOrigin();
			TS_ASSERT_EQUALS( *pURI, "/LF/System/Wireless" );
		}
	}

	void testDisableWireless( )
	{
		PRINT_TEST_NAME();
		
		TS_ASSERT_EQUALS( pWirelessMPI_->IsValid(), true );
		TS_ASSERT_EQUALS( pWirelessMPI_->SetWirelessPower(false), kNoErr);
		sleep(1);
		TS_ASSERT_EQUALS( pWirelessMPI_->GetWirelessPower(), false );
	}
	
	void testEnableWireless( )
	{
		PRINT_TEST_NAME();
		
		TS_ASSERT_EQUALS( pWirelessMPI_->IsValid(), true );
		TS_ASSERT_EQUALS( pWirelessMPI_->SetWirelessPower(true), kNoErr);
		sleep(1);
		TS_ASSERT_EQUALS( pWirelessMPI_->GetWirelessPower(), true );
	}
	
	void testJoinAdHoc( )
	{
		PRINT_TEST_NAME();
		
		TS_ASSERT_EQUALS( pWirelessMPI_->IsValid(), true );
		TS_ASSERT_EQUALS( pWirelessMPI_->JoinAdhocNetwork("MyRandomNetwork", false, ""), kNoErr );
		sleep(5);
		TS_ASSERT_EQUALS( pWirelessMPI_->GetState(), kWirelessConnected );
		TS_ASSERT_EQUALS( pWirelessMPI_->GetMode(), kWirelessAdHoc );
	}
	
	void testLeaveAdHoc( )
	{
		PRINT_TEST_NAME();
		
		TS_ASSERT_EQUALS( pWirelessMPI_->IsValid(), true );
		TS_ASSERT_EQUALS( pWirelessMPI_->LeaveAdhocNetwork(), kNoErr );
		sleep(5);
		TS_ASSERT_EQUALS( pWirelessMPI_->GetState(), kWirelessDisconnected );
		TS_ASSERT_EQUALS( pWirelessMPI_->GetMode(), kWirelessNone );
	}
	
	void testAvahi( )
	{
		PRINT_TEST_NAME();
		pWirelessMPI_->RegisterEventListener(new MyWirelessEventListener());
		PlayerList my_players;
		pWirelessMPI_->GetPossiblePlayers(my_players);
		char ip_string[INET_ADDRSTRLEN];
		for(int i = 0; i < my_players.size(); i++)
		{
			printf("Player: %s IP: %s\n", my_players[i].hostname.c_str(), inet_ntop(AF_INET, &(my_players[i].address), ip_string, INET_ADDRSTRLEN) );
		}
		sleep(60);
	}
		
};

// EOF
