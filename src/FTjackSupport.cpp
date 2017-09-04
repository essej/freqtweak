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

#if HAVE_CONFIG_H
#include <config.h>
#endif


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <list>
using namespace std;

#include "FTjackSupport.hpp"
#include "FTprocessPath.hpp"
#include "FTspectralEngine.hpp"
#include "FTtypes.hpp"

#include <jack/jack.h>


FTjackSupport::FTjackSupport(const char * name, const char * dir)
	:  _inited(false), _jackClient(0), _activePathCount(0), _activated(false), _bypassed(false)
{
	// init process path info
	for (int i=0; i < FT_MAXPATHS; i++) {
		_pathInfos[i] = 0;
	}

	_name = name;

	_jackserv = dir;
}

FTjackSupport::~FTjackSupport()
{
	printf ("jack support destruct\n");
	// init process path info
	for (int i=0; i < FT_MAXPATHS; i++) {
		if (_pathInfos[i]) {
			delete _pathInfos[i];
		}
	}

	if (_inited && _jackClient) {
		close();
		//jack_client_close ( _jackClient );
	}
}

/**
 *  Initialize and connect to jack server.
 *  Returns false if failed
 */
bool FTjackSupport::init()
{

#ifdef HAVE_JACK_CLIENT_OPEN
	jack_options_t options = JackNullOption;
	jack_status_t status;
	const char *server_name = NULL;

	if (!_jackserv.empty()) {
		server_name = _jackserv.c_str();
	}
	
	// jack_client_name = client_name; /* might be reset below */
	if (_name.empty()) {
		_name = "freqtweak";
	}
	
	_jackClient = jack_client_open (_name.c_str(), options, &status, server_name);
	
	if (!_jackClient) {
		fprintf (stderr, "JACK Error: No good client name or JACK server %s not running?\n", _jackserv.c_str());
		_inited = false;
		return false;
	}
	
	if (status & JackServerStarted) {
		fprintf(stderr,"JACK server started\n");
	}

	if (status & JackNameNotUnique) {
		_name = jack_get_client_name (_jackClient);
	}

#else
	char namebuf[100];
	
	/* try to become a client of the JACK server */
	if (_name.empty()) {
		// find a name predictably
		for (int i=1; i < 10; i++) {
			snprintf(namebuf, sizeof(namebuf)-1, "freqtweak_%d", i);
			if ((_jackClient = jack_client_new (namebuf)) != 0) {
				_name = namebuf;
				break;
			}
		}
	}
	else {
		// try the passed name, or base a predictable name from it
		if ((_jackClient = jack_client_new (_name.c_str())) == 0) {
			for (int i=1; i < 10; i++) {
				snprintf(namebuf, sizeof(namebuf)-1, "%s_%d", _name.c_str(), i);
				if ((_jackClient = jack_client_new (namebuf)) != 0) {
					_name = namebuf;
					break;
				}
			}
		}
	}
	
	if (!_jackClient) {
		fprintf (stderr, "JACK Error: No good client name or JACK server not running?\n");
		_inited = false;
		return false;
	}

#endif

	
	/* tell the JACK server to call `process()' whenever
	   there is work to be done.
	*/

	jack_set_process_callback (_jackClient, FTjackSupport::processCallback, 0);

	/* tell the JACK server to call `srate()' whenever
	   the sample rate of the system changes.
	*/


	jack_set_sample_rate_callback (_jackClient, FTjackSupport::srateCallback, 0);

	/* tell the JACK server to call `jack_shutdown()' if
	   it ever shuts down, either entirely, or if it
	   just decides to stop calling us.
	*/

	jack_on_shutdown (_jackClient, FTjackSupport::jackShutdown, 0);

	/* display the current sample rate. once the client is activated 
	   (see below), you should rely on your own sample rate
	   callback (see above) for this value.
	*/

	_sampleRate = jack_get_sample_rate (_jackClient);
	//printf ("engine sample rate: %lu\n", _sampleRate);

	_inited = true;
	return true;
}

bool FTjackSupport::startProcessing()
{
	if (!_jackClient) return false;
	
	if (jack_activate (_jackClient)) {
		fprintf (stderr, "Error: cannot activate jack client!\n");
		return false;
	}

	_activated = true;

	return true;
}

bool FTjackSupport::stopProcessing()
{
	if (!_jackClient) return false;

	// load up latest port connections for possible restoring
	for (int i=0; i < _activePathCount; ++i) {
		const char ** ports;
		if ((ports = getConnectedInputPorts (i)) != NULL)
			free (ports);
		if ((ports = getConnectedOutputPorts (i)) != NULL)
			free (ports);
	}
	
	if (jack_deactivate (_jackClient)) {
		fprintf (stderr, "Error: cannot deactivate jack client!\n");
		return false;
	}
	//printf ("deactivated jack\n");
	_activated = false;
	
	return true;
}

bool FTjackSupport::close()
{
	if (_inited && _jackClient) {
		stopProcessing();
		jack_client_close ( _jackClient );
		_jackClient = 0;
		_inited = false;
		return true;
	}
	return false;
}


FTprocessPath * FTjackSupport::setProcessPathActive (int index, bool active)
{
	if (!_inited || !_jackClient) return 0;

	char nbuf[30];

	PathInfo *tmppath;
	FTprocessPath * ppath;
	
	if (index >=0 && index < FT_MAXPATHS) {
		if (_pathInfos[index]) {
			tmppath = _pathInfos[index];
			ppath = tmppath->procpath;

			if (active) {
				if (tmppath->active) {
					//already active, do nothing
					return ppath;
				}
			}
			else {
				if (tmppath->active)  {
					//  detach ports
					jack_port_unregister (_jackClient, tmppath->inputport);
					jack_port_unregister (_jackClient, tmppath->outputport);
				
					// just mark it inactive but not destroy it
					tmppath->active = false;
					_activePathCount--;
					//we are not deleting old one, we reuse it later if necessary
				}
				return 0;
			}
		}
		else {
			// it is a new one, construct new processPath
			if (active) {
				tmppath = new PathInfo();
				ppath = new FTprocessPath();
			}
			else {
				return 0;
			}
		}

		// it only gets here if it is brand new, or going from inactive->active
		
		sprintf(nbuf,"in_%d", index + 1);
		tmppath->inputport = jack_port_register (_jackClient, nbuf, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);

		
		sprintf(nbuf,"out_%d", index + 1);
		tmppath->outputport = jack_port_register (_jackClient, nbuf, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
		jack_port_set_latency (tmppath->outputport, ppath->getSpectralEngine()->getLatency());

		tmppath->procpath = ppath;
		tmppath->active = true;
		
		_pathInfos[index] = tmppath;

		ppath->setId (index);
		_activePathCount++;
		
		return ppath;
	}
	
	return false;
}


const char * FTjackSupport::getInputPortName(int index)
{
	if (index >=0 && index < FT_MAXPATHS) {
		if (_pathInfos[index]) {
			return jack_port_name(_pathInfos[index]->inputport);
		}
	}

	return NULL;
}

const char * FTjackSupport::getOutputPortName(int index)
{
	if (index >=0 && index < FT_MAXPATHS) {
		if (_pathInfos[index]) {
			return jack_port_name(_pathInfos[index]->outputport);
		}
	}

	return NULL;
}


bool FTjackSupport::connectPathInput (int index, const char *inname)
{
	if (!_jackClient) return false;
	
	if (index >=0 && index < FT_MAXPATHS && _pathInfos[index])
	{
		if (jack_connect (_jackClient, inname, jack_port_name(_pathInfos[index]->inputport))) {
			fprintf (stderr, "JACK error: cannot connect input port: %s -> %s\n", inname,
				 jack_port_name(_pathInfos[index]->inputport));
			return false;
		}

		_pathInfos[index]->inconn_list.push_back (inname);
		return true;
	}

	return false;
}

bool FTjackSupport::connectPathOutput (int index, const char *outname)
{
	if (!_jackClient) return false;

	if (index >=0 && index < FT_MAXPATHS && _pathInfos[index])
	{
		if (jack_connect (_jackClient, jack_port_name(_pathInfos[index]->outputport), outname)) {
			fprintf (stderr, "JACK error: cannot connect output port: %s -> %s\n",
				 jack_port_name(_pathInfos[index]->outputport), outname);
			return false;
		}
		
		_pathInfos[index]->outconn_list.push_back (outname);
		return true;
	}

	return false;
}


bool FTjackSupport::disconnectPathInput (int index, const char *inname)
{
	if (!_jackClient) return false;

	if (index >=0 && index < FT_MAXPATHS && _pathInfos[index])
	{
		if (inname)
		{
			if (jack_disconnect (_jackClient, inname, jack_port_name(_pathInfos[index]->inputport))) {
				fprintf (stderr, "cannot disconnect input port\n");
				return false;
			}
			_pathInfos[index]->inconn_list.remove (inname);
			return true;
		}
		else {
			// disconnect all from our input port
			const char ** portnames = jack_port_get_connections (_pathInfos[index]->inputport);
			if (portnames) {
				for (int i=0; portnames[i]; i++) {
					jack_disconnect (_jackClient, portnames[i], jack_port_name(_pathInfos[index]->inputport));
				}
				free(portnames);
			}

			_pathInfos[index]->inconn_list.clear();

			return true;
		}
	}

	return false;
}

bool FTjackSupport::disconnectPathOutput (int index, const char *outname)
{
	if (!_jackClient) return false;
	
	if (index >=0 && index < FT_MAXPATHS && _pathInfos[index])
	{
		if (outname)
		{
			if (jack_disconnect (_jackClient, jack_port_name(_pathInfos[index]->outputport), outname)) {
				fprintf (stderr, "cannot disconnect output ports\n");
				return false;
			}
			_pathInfos[index]->outconn_list.remove (outname);
			return true;
		}
		else {
			// disconnect all from our output port
			const char ** portnames = jack_port_get_connections (_pathInfos[index]->outputport);
			if (portnames) {
				for (int i=0; portnames[i]; i++) {
					jack_disconnect (_jackClient, jack_port_name(_pathInfos[index]->outputport), portnames[i]);
				}
				free(portnames);
			}
			_pathInfos[index]->outconn_list.clear();
			return true;
		}

	}

	return false;
}


const char ** FTjackSupport::getInputConnectablePorts (int index)
{
	const char ** portnames = NULL;

	if (!_jackClient) return NULL;
	
	if (index >=0 && index < FT_MAXPATHS && _pathInfos[index])
	{
		//char regexstr[100];
		// anything but our own output port
		//snprintf(regexstr, 99, "^(%s)", jack_port_name (_pathInfos[index]->outputport) );
		
		portnames = jack_get_ports( _jackClient, NULL, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput);
	}

	return portnames;
}

const char ** FTjackSupport::getOutputConnectablePorts (int index)
{
	const char ** portnames = NULL;

	if (!_jackClient) return NULL;

	if (index >=0 && index < FT_MAXPATHS && _pathInfos[index])
	{
		//char regexstr[100];
		// anything but our own input port
		//snprintf(regexstr, 99, "^(%s)", jack_port_name (_pathInfos[index]->inputport) );
		
		portnames = jack_get_ports( _jackClient, NULL, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput);
	}

	return portnames;
}


const char ** FTjackSupport::getConnectedInputPorts(int index)
{
	const char ** portnames = NULL;
	if (!_jackClient) return NULL;

	if (index >=0 && index < FT_MAXPATHS && _pathInfos[index])
	{
		//char regexstr[100];
		// anything but our own input port
		//snprintf(regexstr, 99, "^(%s)", jack_port_name (_pathInfos[index]->inputport) );
		
		portnames = jack_port_get_connections( _pathInfos[index]->inputport);

		_pathInfos[index]->inconn_list.clear();
		if (portnames) {
			for (int i=0; portnames[i]; i++) {
				_pathInfos[index]->inconn_list.push_back (portnames[i]);
			}			
		}
	}

	return portnames;
}

const char ** FTjackSupport::getConnectedOutputPorts(int index)
{
	const char ** portnames = NULL;
	if (!_jackClient) return NULL;

	if (index >=0 && index < FT_MAXPATHS && _pathInfos[index])
	{
		//char regexstr[100];
		// anything but our own input port
		//snprintf(regexstr, 99, "^(%s)", jack_port_name (_pathInfos[index]->inputport) );
		
		portnames = jack_port_get_connections( _pathInfos[index]->outputport);

		_pathInfos[index]->outconn_list.clear();
		if (portnames) {
			for (int i=0; portnames[i]; i++) {
				_pathInfos[index]->outconn_list.push_back (portnames[i]);
			}			
		}
	}

	return portnames;
}

const char ** FTjackSupport::getPhysicalInputPorts()
{
	const char ** portnames = NULL;
	if (!_jackClient) return NULL;
	
	if ((portnames = jack_get_ports (_jackClient, NULL, NULL, JackPortIsPhysical|JackPortIsOutput)) == NULL) {
		fprintf(stderr, "Cannot find any physical capture ports");
	}

	return portnames;
}

const char ** FTjackSupport::getPhysicalOutputPorts()
{
	const char ** portnames = NULL;
	if (!_jackClient) return NULL;
	
	if ((portnames = jack_get_ports (_jackClient, NULL, NULL, JackPortIsPhysical|JackPortIsInput)) == NULL) {
		fprintf(stderr, "Cannot find any physical playback ports");
	}

	return portnames;
}

nframes_t FTjackSupport::getTransportFrame()
{
	if (!_jackClient) return 0;

	if (jack_transport_query (_jackClient, 0) == JackTransportRolling) {
		return jack_get_current_transport_frame (_jackClient);
	}
	else {
		return jack_frame_time (_jackClient);
	}
}


bool FTjackSupport::inAudioThread()
{
	if (_jackClient && (pthread_self() == jack_client_thread_id (_jackClient))) {
		return true;
	} 
	return false;
}

void FTjackSupport::setProcessingBypassed (bool val)
{
   if (_bypassed != val) {
      // TODO: flag some sort of cross-fade ramp
      _bypassed = val;
   }
}


bool FTjackSupport::reinit (bool rebuild)
{
	// assume that activePathCount is in the previous state
	// assume that the _pathInfos contain valid lists of connected ports
	// and active flags

	//printf ("reinit\n");
	if (!_jackClient) return false;

	
	for (int i=0; i < FT_MAXPATHS; i++)
	{
		if (_pathInfos[i] && _pathInfos[i]->active) {
			if (rebuild) {
				_pathInfos[i]->active = false;
				setProcessPathActive (i, true);
			}
			
			// reconnect to ports
			list<string> inlist (_pathInfos[i]->inconn_list); // copy			
			_pathInfos[i]->inconn_list.clear();

			for (list<string>::iterator port = inlist.begin(); port != inlist.end(); ++port)
			{
				// only do it if the port is not one of ours
				// those interconnected ports within ourself are added
				// only once below
				jack_port_t * tport = jack_port_by_name(_jackClient, (*port).c_str());
				if ( tport && ! jack_port_is_mine ( _jackClient, tport))
				{
					//fprintf(stderr, "reconnecting to input: %s\n", port.c_str());
					connectPathInput ( i, (*port).c_str() );
				}
				
			}

			list<string> outlist (_pathInfos[i]->outconn_list); // copy
			_pathInfos[i]->outconn_list.clear();
			
			for (list<string>::iterator port = outlist.begin(); port != outlist.end(); ++port)
			{
				//fprintf(stderr, "reconnecting to output: %s\n", port.c_str());
				connectPathOutput ( i, (*port).c_str() );
			}
		}
	}
	
	return true;
}
	     


/** static callbacks **/

int FTjackSupport::processCallback (jack_nframes_t nframes, void *arg)
{
	FTjackSupport * jsup = (FTjackSupport *) FTioSupport::instance();
	PathInfo * tmppath;

	// do processing for each path
	for (int i=0; i < FT_MAXPATHS; i++)
	{
		if (jsup->_pathInfos[i] && jsup->_pathInfos[i]->active) {
			tmppath = jsup->_pathInfos[i];
			
			sample_t *in = (sample_t *) jack_port_get_buffer (tmppath->inputport, nframes);
			sample_t *out = (sample_t *) jack_port_get_buffer (tmppath->outputport, nframes);

			if (jsup->_bypassed)
			{
				if (in != out) {
					memcpy (out, in, nframes * sizeof(sample_t));
				}
			}
			else
			{
				tmppath->procpath->processData(in, out, nframes);
			}
			
		}
	}

	return 0;	
}


int FTjackSupport::srateCallback (jack_nframes_t nframes, void *arg)
{
	FTjackSupport * jsup = (FTjackSupport *) FTioSupport::instance();

	for (int i=0; i < FT_MAXPATHS; i++)
	{
		if (jsup->_pathInfos[i]) {
			jsup->_pathInfos[i]->procpath->setSampleRate(nframes);
		}
	}
	
	return 0;
}

void FTjackSupport::jackShutdown (void *arg)
{
	FTjackSupport * jsup = (FTjackSupport *) FTioSupport::instance();

	fprintf (stderr, "Jack shut us down!\n");

	jsup->_inited = false;
	jsup->_jackClient = 0;
	
	/*
	//jsup->close();
	fprintf (stderr, "Trying to reconnect....\n");
	if (jsup->init()) {
		if (jsup->startProcessing()) {
			jsup->reinit ();
		}
	}
	*/
}

// This isn't in use yet.
int FTjackSupport::portsChanged (jack_port_id_t port, int blah, void *arg)
{
	FTjackSupport * jsup = (FTjackSupport *) FTioSupport::instance();

	fprintf (stderr, "Ports changed on us!\n");
 
	jsup->_portsChanged = true;

	return 0;
}

