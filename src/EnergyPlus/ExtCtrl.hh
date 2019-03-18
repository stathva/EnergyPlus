#ifndef ExtCtrl_hh_INCLUDED
#define ExtCtrl_hh_INCLUDED

// C++ Headers
#include <cassert>
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/bit.hh>
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus.hh>
#include <DataGlobals.hh>
#include <UtilityRoutines.hh>

namespace EnergyPlus {

namespace ExtCtrl {

	// Data
	// MODULE PARAMETER DEFINITIONS:
	// For SEND command
	extern int const CMD_OBS_INIT;
	extern int const NUM_OBSS;
	extern int const CMD_OBS_INDEX_LOW;
	extern int const CMD_OBS_INDEX_HIGH;
	extern Real64 stats[];
	extern Real64 const STAT_DATA_NULL;

	// For RECV command
	extern int const CMD_ACT_REQ;
	extern int const NUM_ACTS;
	extern int const CMD_ACT_INDEX_LOW;
	extern int const CMD_ACT_INDEX_HIGH;
	extern Real64 acts[];
	extern Real64 const ACT_DATA_NULL;

	extern std::string const blank_string;

	// MODULE VARIABLE DECLARATIONS:
	// na

	// MODULE VARIABLE DEFINITIONS:
	extern std::string String;
	extern bool ReportErrors;

	// DERIVED TYPE DEFINITIONS

	// Types

	// Object Data

	// Subroutine Specifications for the Module

	// Functions

	int
	InitializeExtCtrlRoutines();
	void
	InitializeObsData();
	void
	InitializeActData();

	Real64
	ExtCtrlObs(
		Real64 const cmd, // command code
		Real64 const arg  // command value
	);

	Real64
	ExtCtrlAct(
		Real64 const cmd, // command code
		Real64 const arg  // command value
	);

} // ExtCtrl

} // EnergyPlus

#endif