/*
** Copyright (C) 2002 Jesse Chappell <jesse@essej.net>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**  
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**  
*/

/**
 *  Supports operations concerning I/O with JACK
 */

#ifndef __FTIOSUPPORT_HPP__
#define __FTIOSUPPORT_HPP__

#include <string>
using namespace std;

#include "FTtypes.hpp"

class FTprocessPath;


class FTioSupport
{
  public:
	virtual ~FTioSupport() {};

	virtual bool init() = 0;
	virtual bool reinit(bool rebuild=true) = 0;

	virtual bool isInited() = 0;
	
	//virtual bool setProcessPath(FTprocessPath *ppath, int index) = 0;
	virtual FTprocessPath * getProcessPath(int index) = 0;
	virtual FTprocessPath * setProcessPathActive(int index, bool flag) = 0;

	virtual int getActivePathCount () = 0;
	
	virtual bool startProcessing() = 0;
	virtual bool stopProcessing() = 0;
	virtual bool close() = 0;

    	virtual bool connectPathInput (int index, const char *inname) = 0;
        virtual bool connectPathOutput (int index, const char *outname) = 0;
    	virtual bool disconnectPathInput (int index, const char *inname) = 0;
        virtual bool disconnectPathOutput (int index, const char *outname) = 0;

	virtual const char ** getConnectedInputPorts(int index) = 0;
	virtual const char ** getConnectedOutputPorts(int index) = 0;
	
	virtual const char ** getInputConnectablePorts(int index) = 0;
	virtual const char ** getOutputConnectablePorts(int index) = 0;

	virtual const char * getInputPortName(int index) = 0;
	virtual const char * getOutputPortName(int index) = 0;
	

	virtual const char ** getPhysicalInputPorts() = 0;
	virtual const char ** getPhysicalOutputPorts() = 0;

	
	virtual nframes_t getSampleRate() = 0;
	virtual nframes_t getTransportFrame() = 0;

	
	virtual bool getPortsChanged() = 0;

	virtual void setName (const string & name);
	virtual const char * getName() { return _name.c_str(); }

	virtual bool inAudioThread() { return false; }

        virtual void setProcessingBypassed (bool val) = 0;
    
	enum IOtype
	{
		IO_JACK,
	};

	// set io type for this session
	// call before the first instance is called
	static void setIOtype (IOtype it) { _iotype = it; }
	
	// singleton retrieval
	static FTioSupport * instance() { if (!_instance) _instance = createInstance(); return _instance; }

	static void setDefaultName(const string & name) { _defaultName = name; }
	static void setDefaultServer(const string & dir) { _defaultServ = dir; }
	
  protected:

	// this is our singleton
	static FTioSupport * _instance;
	static IOtype _iotype;
	
	static FTioSupport * createInstance();
	static string _defaultName;
	static string _defaultServ;
	
	string _name;
};


#endif
