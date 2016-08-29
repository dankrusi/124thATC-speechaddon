
/*
 * 124thATC-speechaddon.cpp
 * https://github.com/dankrusi/124thATC-speechaddon
 * Copyright (c) 2016 Dan Krusi, licensed under GNU GPLv3
 * 
 */

#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>

#include "XPLMDisplay.h"
#include "XPLMUtilities.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMProcessing.h"

// Constants
#define LOG_FILENAME "Log.txt"
#define UPDATE_INTERVAL_SECONDS 0.01
#define LOG_PREFIX_MONITOR_124THATC "124thATC v2"
#define LOG_PREFIX_MONITOR_COMM " : Communication: "

// Variables
static XPLMDataRef gDataRef = NULL;
static std::ostringstream gLogPath;
static std::ifstream gLogFileIFS;
std::ios::streampos gPos;
std::string gLine;
static XPLMDataRef gOwnedDataRef = NULL;
std::string gLatestComm;

// Forward declares




void _debug(const char *str) {
    std::ostringstream out;
    out << "12th4ATC-speechaddon:" << str << "\n";
    XPLMDebugString(out.str().c_str());
}
void _debug(std::ostringstream str) {
    _debug(str.str().c_str());
}
void _debug(std::string str) {
    _debug(str.c_str());
}

void _speakComm(std::string comm) {
	// Update latest comm dataref data
    gLatestComm = comm;

	//TODO: ...
}

float	_flightLoopCallback(
                                   float                inElapsedSinceLastCall,
                                   float                inElapsedTimeSinceLastFlightLoop,
                                   int                  inCounter,
                                   void *               inRefcon) {

    // Try to read line
    if(!std::getline(gLogFileIFS, gLine) || gLogFileIFS.eof())
    {
        // If we fail, clear stream, return to beginning of line
        gLogFileIFS.clear();
        gLogFileIFS.seekg(gPos);

        // Wait to try again
        return UPDATE_INTERVAL_SECONDS;
    }

    // Remember the position of the next line in case the next read fails
    gPos = gLogFileIFS.tellg();

    // Parse line...
    // First we search for the plugin prefix in the log
    std::string prefix(LOG_PREFIX_MONITOR_124THATC);
    if (!gLine.compare(0, prefix.size(), prefix)) {
        // Is it a communication?
        int index = gLine.find(LOG_PREFIX_MONITOR_COMM);
        if(index > -1) {
            // Communication!
            std::string comm = gLine.substr(index+strlen(LOG_PREFIX_MONITOR_COMM));
            std::cout << "******************COMM: " << comm << std::endl;
            _speakComm(comm);
        }

    }
    return UPDATE_INTERVAL_SECONDS;
}



int	_getDatabCallback(void *               inRefcon,
                          void *               outValue,    /* Can be NULL */
                          int                  inOffset,
                          int                  inMaxLength)
{
    if(outValue == NULL) return 0;
    _debug("_getDatabCallback");
    const char* latestCommCStr = gLatestComm.c_str();
    int len = strlen(latestCommCStr);
    for(int i = 0; i < len; i++) {
        ((char*)outValue)[i] = latestCommCStr[i];
    }
    return len;
}



PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc)
{
	// Set plugin info
	strcpy(outName, "124thATC-speechaddon");
	strcpy(outSig, "com.dankrusi.124thATC-speechaddon");
	strcpy(outDesc, "Addon for 124thATC-speechaddon for TTS and Voice Recognition.");
    _debug("XPluginStart!");

    gOwnedDataRef = XPLMRegisterDataAccessor(
                                    "atc/comm/latest",
                                    xplmType_Data,			/* The types we support */
                                    0,											/* Writable */
                                    NULL, NULL,									/* No accessors for ints */
                                    NULL, NULL,		/* Accessors for floats */
                                    NULL, NULL,		/* Accessors for doubles */
                                    NULL, NULL,									/* No accessors for int arrays */
                                    NULL, NULL,									/* No accessors for float arrays */
                                    _getDatabCallback, NULL,									/* No accessors for raw data */
                                    NULL, NULL);								/* Refcons not used */

	
	// Get the logfile path
	char sysPath[1024];
	XPLMGetSystemPath(sysPath);
    gLogPath << sysPath << LOG_FILENAME;
    _debug(sysPath);
    _debug(const_cast<char*>(gLogPath.str().c_str()));
	
    // Open file
    gLogFileIFS.open(const_cast<char*>(gLogPath.str().c_str()), std::ios::ate);

    if(!gLogFileIFS.is_open())
    {
        _debug("Warning: Could not open Log.txt");
        return 1;
    }

    // Remember file position
    gPos = gLogFileIFS.tellg();

	return 1;
}

PLUGIN_API void	XPluginStop(void) {
    _debug("XPluginStop");
    XPLMUnregisterFlightLoopCallback(_flightLoopCallback,NULL);
    XPLMUnregisterDataAccessor(gOwnedDataRef);
}

PLUGIN_API void XPluginDisable(void) {
    _debug("XPluginDisable");
}

PLUGIN_API int XPluginEnable(void) {
    _debug("XPluginEnable");

    XPLMRegisterFlightLoopCallback(
                _flightLoopCallback,	/* Callback */
                UPDATE_INTERVAL_SECONDS,					/* Interval */
                NULL);					/* refcon not used. */

	return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, int inMessage, void *inParam) {
}






