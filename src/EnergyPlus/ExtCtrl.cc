// C++ Headers
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Fmath.hh>
#include <ObjexxFCL/gio.hh>
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <CommandLineInterface.hh>
#include <ExtCtrl.hh>
#include <DataEnvironment.hh>
#include <DataPrecisionGlobals.hh>
#include <General.hh>
#include <UtilityRoutines.hh>
#include <DisplayRoutines.hh>

namespace EnergyPlus {

namespace ExtCtrl {
	// Data
	// MODULE PARAMETER DEFINITIONS:
	// For sending observation
	int const CMD_OBS_INIT( 0 );
	int const NUM_OBSS( 100 );
	int const CMD_OBS_INDEX_LOW( 1 );
	int const CMD_OBS_INDEX_HIGH( NUM_OBSS );
	Real64 obss[ NUM_OBSS ];
	Real64 const OBS_DATA_NULL( -123.0 );

	// For receiving action
	int const CMD_ACT_REQ( 0 );
	int const NUM_ACTS ( 100 );
	int const CMD_ACT_INDEX_LOW ( 1 );
	int const CMD_ACT_INDEX_HIGH ( NUM_ACTS );
	Real64 acts[ NUM_ACTS ];
	Real64 const ACT_DATA_NULL( -456.0 );

	std::string const blank_string;

	// MODULE VARIABLE DECLARATIONS:
	// na

	// MODULE VARIABLE DEFINITIONS:
	//std::string String;
	bool ReportErrors( true );

	// Object Data
	std::ifstream act_ifs;
	std::ofstream obs_ofs;
	char *act_filename;
	char *obs_filename;
	int act_seq = 0;
	int obs_seq = 0;

	// Subroutine Specifications for the Module

	// Functions

	int
	InitializeExtCtrlRoutines()
	{
		static bool firstCall (true);
		if ( firstCall ) {
			firstCall = false;
			//DisplayString( "InitializeExtCtrlRoutine(): First call" );

			if ((act_filename = getenv("ACT_PIPE_FILENAME")) == NULL) {
				ShowFatalError( "InitializeExtCtrlActRoutines: Environment variable ACT_PIPE_FILENAME not specified" );
				DisplayString( "InitializeExtCtrlRoutine: ACT file not specified" );
				return 1;
			}
			if ((obs_filename = getenv("OBS_PIPE_FILENAME")) == NULL) {
				ShowFatalError( "InitializeExtCtrlActRoutines: Environment variable OBS_PIPE_FILENAME not specified" );
				return 1;
			}
		}
		return 0;
	}

	std::string
	ExtCtrlRead()
	{
		if (!act_ifs.is_open()) {
			act_ifs.open(act_filename);
			act_ifs.rdbuf()->pubsetbuf(0, 0); // Making unbuffered
			if (!act_ifs.is_open()) {
				ShowFatalError( "ExtCtrlRead: ACT file could not open" );
				return "";
			}
			DisplayString( "ExtCtrlRead: Opened ACT file: " + std::string(act_filename) );
		}
		std::string line;
	  again:
		act_ifs >> line;
		size_t idx = line.find(",");
		if (idx == std::string::npos) {
			goto again;
		}
		std::string seq = line.substr(0, idx);
		std::string val = line.substr(idx + 1, std::string::npos);
		assert(act_seq == seq);
		act_seq++;
		return val;
	}

	void
	ExtCtrlWrite(std::string str)
	{
		if (!obs_ofs.is_open()) {
			obs_ofs.open(obs_filename);
			if (!obs_ofs.is_open()) {
				ShowFatalError( "ExtCtrlWrite: InitializeExtCtrlRoutine: OBS file could not open" );
				return;
			}
			DisplayString( "ExtCtrlWrite: Opened OBS file: " + std::string(obs_filename) );
		}
		obs_ofs << obs_seq << "," << str << std::endl;
		obs_seq++;
	}

	void
	ExtCtrlFlush()
	{
		obs_ofs << "DELIMITER" << std::endl;
		obs_ofs.flush();
	}

	void
	InitializeObsData()
	{
		for (int i = 0; i < NUM_OBSS; i++)
			obss[i] = OBS_DATA_NULL;
	}

	void
	InitializeActData()
	{
		for (int i = 0; i < NUM_ACTS; i++)
			acts[i] = ACT_DATA_NULL;
	}

	Real64
	ExtCtrlObs(
		Real64 const cmd, // command code
		Real64 const arg  // command value
	)
	{
		Int64 cmdInt = cmd;

		if (InitializeExtCtrlRoutines()) {
			return -1.0;
		}
		if (cmdInt >= CMD_OBS_INDEX_LOW && cmdInt <= CMD_OBS_INDEX_HIGH) {
			//DisplayString( "ExtCtrlObs: set obs[" + std::to_string( cmdInt ) + "] = " + std::to_string( arg ) );
			obss[cmdInt - 1] = arg;
			return 0.0;
		}
		else if (cmdInt == CMD_OBS_INIT ) {
			//DisplayString( "ExtCtrlObs: INIT" );
			// If not connected to the server, try to connect.
			// TODO:
			//ShowFatalError( "Failed to connect to external service" );
			return 0.0;
		}
		// TODO: Show error code
		ShowWarningMessage( "Obs index " + std::to_string( cmdInt ) + " is out of range [" + std::to_string( CMD_OBS_INDEX_LOW ) + "..." + std::to_string( CMD_OBS_INDEX_HIGH ) + "]" );
		return -1.0;
	}

	Real64
	ExtCtrlAct(
		Real64 const cmd, // command code
		Real64 const arg  // command value
	)
	{
		Int64 cmdInt = cmd;
		Int64 ArgInt = arg;

		if (InitializeExtCtrlRoutines()) {
			return -1.0;
		}
		if (cmdInt >= CMD_ACT_INDEX_LOW && cmdInt <= CMD_ACT_INDEX_HIGH) {
			//DisplayString( "ExtCtrlAct: get acts[" + std::to_string( cmdInt ) + "] = " + std::to_string( acts[cmdInt - 1] ) );
			return acts[cmdInt - 1];
		}
		else if (cmdInt == CMD_ACT_REQ ) {
			if ( !(ArgInt >= 0 && ArgInt <= CMD_ACT_INDEX_HIGH) ) {
				ShowWarningMessage( "ExtCtrlAct: Number of obss " + std::to_string( ArgInt ) + " must be in range [0..." + std::to_string( CMD_ACT_INDEX_HIGH ) + "]" );
				return -1.0;
			}
			// Send observation data to the server, and receive next action.
			ExtCtrlWrite(std::to_string(ArgInt));
			for (int i = CMD_ACT_INDEX_LOW; i <= ArgInt; i++) {
				ExtCtrlWrite(std::to_string(obss[i - 1]));
			}
			ExtCtrlFlush();

			// Get action data
			std::string line;
			line = ExtCtrlRead();
			int NumActsReceived = std::stoi(line);
			assert(NumActsReceived >= 0 && MumActsReceived <= CMD_ACT_INDEX_HIGH);
			for (int i = 1; i <= NumActsReceived; i++) {
				line = ExtCtrlRead();
				double val = std::stod(line);
				if (i <= CMD_ACT_INDEX_HIGH) {
					acts[i - 1] = val;
				}
			}

			return 0.0;
		}
		ShowWarningMessage( "Act index "+ std::to_string( cmdInt ) + " is out of range [" + std::to_string( CMD_ACT_INDEX_LOW ) + " to " + std::to_string( CMD_ACT_INDEX_HIGH ) + "]" );
		return -1.0;
	}

} // ExtCtrl

} // EnergyPlus