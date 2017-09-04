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

#ifndef __FTJACKSUPPORT_HPP__
#define __FTJACKSUPPORT_HPP__

#include <jack/jack.h>

#include "FTtypes.hpp"
#include "FTioSupport.hpp"

#include <string>
#include <list>
using namespace std;

class FTprocessPath;


class FTjackSupport
	: public FTioSupport
{
  public:
	FTjackSupport(const char * name="", const char * dir="");

	virtual ~FTjackSupport();

	bool init();
	bool reinit(bool rebuild=true);

	bool isInited() { return _inited; }
	
	//bool setProcessPath(FTprocessPath *ppath, int index);
	FTprocessPath * setProcessPathActive(int index, bool flag);

	FTprocessPath * getProcessPath(int index)
		{ if(index >=0 && index<FT_MAXPATHS && _pathInfos[index]) return _pathInfos[index]->procpath;
		return 0; }

	int getActivePathCount () { return _activePathCount; }
	
	bool startProcessing();
	bool stopProcessing();
	bool close();

    	bool connectPathInput (int index, const char *inname);
        bool connectPathOutput (int index, const char *outname);
    	bool disconnectPathInput (int index, const char *inname);
        bool disconnectPathOutput (int index, const char *outname);

	const char ** getConnectedInputPorts(int index);
	const char ** getConnectedOutputPorts(int index);
	
	const char ** getInputConnectablePorts(int index);
	const char ** getOutputConnectablePorts(int index);

	const char ** getPhysicalInputPorts();
	const char ** getPhysicalOutputPorts();

	const char * getInputPortName(int index);
	const char * getOutputPortName(int index);
	
	bool inAudioThread();
    
	nframes_t getSampleRate() { return _sampleRate; }
	nframes_t getTransportFrame();
	bool getPortsChanged() { return _portsChanged; }

        void setProcessingBypassed (bool val);
	
  protected:


	// JACK callbacks are static
	static int processCallback (jack_nframes_t nframes, void *arg);
	static int srateCallback (jack_nframes_t nframes, void *arg);
	static void jackShutdown (void *arg);
	static int portsChanged (jack_port_id_t port, int blah, void *arg);
	
	bool _inited;
	jack_client_t * _jackClient;
	jack_nframes_t _sampleRate;
	jack_nframes_t _maxBufsize;

	struct PathInfo
	{
		FTprocessPath * procpath;
		jack_port_t * inputport;
		jack_port_t * outputport;
		bool active;

		list<string> inconn_list;
		list<string> outconn_list;
	};

	// FIXME: use real data structure
	PathInfo* _pathInfos[FT_MAXPATHS];

	int _activePathCount;
	//char _name[100];

	string _jackserv;
	
	bool _portsChanged;
	bool _activated;
        bool _bypassed;
};


#endif
