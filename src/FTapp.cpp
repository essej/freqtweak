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

#include <signal.h>
#include <stdio.h>
#include <pthread.h>
#include <iostream>

//#include <wx/wx.h>

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWindows headers
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/cmdline.h>

#include "version.h"

#include "FTapp.hpp"
#include "FTtypes.hpp"
#include "FTmainwin.hpp"
#include "FTioSupport.hpp"
#include "FTprocessPath.hpp"


// Create a new application object: this macro will allow wxWindows to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also declares the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(FTapp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------


static const wxCmdLineEntryDesc cmdLineDesc[] =
{
	{ wxCMD_LINE_SWITCH, wxT("h"), wxT("help"), wxT("show this help"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
	{ wxCMD_LINE_OPTION, wxT("c"), wxT("channels"), wxT("# processing channels (1-4) default is 2"), wxCMD_LINE_VAL_NUMBER },
	{ wxCMD_LINE_OPTION, wxT("i"), wxT("inputs"),
	  wxT("connect inputs from these jack ports (separate each channel with commas).\n")
	      "\t\t\tDefaults to 'alsa_pcm:capture_1,..." },
	{ wxCMD_LINE_OPTION, wxT("o"), wxT("outputs"),
	  wxT("connect outputs to these jack ports (separate each channel with commas).\n")
	      "\t\t\tDefaults to 'alsa_pcm:playback_1,...'" },
	{ wxCMD_LINE_OPTION, wxT("n"), wxT("jack-name"), wxT("jack name.   default is freqtweak_1")},
	{ wxCMD_LINE_OPTION, wxT("S"), wxT("jack-server"), wxT("jack server name")},
	{ wxCMD_LINE_OPTION, wxT("p"), wxT("preset"), wxT("load given preset initially")},
	{ wxCMD_LINE_OPTION, wxT("r"), wxT("rc-dir"), wxT("what directory to use for run-control state. default is ~/.freqtweak")},
	{ wxCMD_LINE_NONE }
};	

	
FTapp::FTapp()
	: _mainwin(0)
{
}


static void* watchdog_thread(void* arg)
{
  sigset_t signalset;
  //struct ecasound_state* state = reinterpret_cast<struct ecasound_state*>(arg);
  int signalno;
  bool exiting = false;
  
  /* register cleanup routine */
  //atexit(&ecasound_atexit_cleanup);

  // cerr << "Watchdog-thread created, pid=" << getpid() << "." << endl;

  while (!exiting)
  {
	  sigemptyset(&signalset);
	  
	  /* handle the following signals explicitly */
	  sigaddset(&signalset, SIGTERM);
	  sigaddset(&signalset, SIGINT);
	  sigaddset(&signalset, SIGHUP);
	  sigaddset(&signalset, SIGPIPE);
	  
	  /* block until a signal received */
	  sigwait(&signalset, &signalno);
	  
	  //cerr << endl << "freqtweak: watchdog-thread received signal " << signalno << ". Cleaning up..." << endl;

	  if (signalno == SIGHUP) {
		  // reinit iosupport
// 		  cerr << "freqtweak got SIGHUP... reiniting" << endl;
// 		  wxThread::Sleep(200);

// 		  FTioSupport * iosup = FTioSupport::instance();
// 		  if (!iosup->isInited()) {
// 			  iosup->init();
// 			  if (iosup->startProcessing()) {
// 				  iosup->reinit();
// 			  }
// 		  }

// 		  if (::wxGetApp().getMainwin()) {
// 			  ::wxGetApp().getMainwin()->updateDisplay();
// 		  }
		  exiting = true;
	  }
	  else {
		  exiting = true;
	  }
  }

  ::wxGetApp().getMainwin()->Close(TRUE);
  
  ::wxGetApp().ExitMainLoop();
  printf ("bye bye, hope you had fun...\n");

  /* to keep the compilers happy; never actually executed */
  return(0);
}



/**
 * Sets up a signal mask with sigaction() that blocks 
 * all common signals, and then launces an watchdog
 * thread that waits on the blocked signals using
 * sigwait().
 */
void FTapp::setupSignals()
{
  pthread_t watchdog;

  /* man pthread_sigmask:
   *  "...signal actions and signal handlers, as set with
   *   sigaction(2), are shared between all threads"
   */

  struct sigaction blockaction;
  blockaction.sa_handler = SIG_IGN;
  sigemptyset(&blockaction.sa_mask);
  blockaction.sa_flags = 0;

  /* ignore the following signals */
  sigaction(SIGTERM, &blockaction, 0);
  sigaction(SIGINT, &blockaction, 0);
  sigaction(SIGHUP, &blockaction, 0);
  sigaction(SIGPIPE, &blockaction, 0);

  int res = pthread_create(&watchdog, 
			   NULL, 
			   watchdog_thread, 
			   NULL);
  if (res != 0) {
    cerr << "freqtweak: Warning! Unable to create watchdog thread." << endl;
  }
}



// `Main program' equivalent: the program execution "starts" here
bool FTapp::OnInit()
{

// 	signal (SIGTERM, onTerminate);
// 	signal (SIGINT, onTerminate);

// 	signal (SIGHUP, onHangup);

	
	wxString inputports[FT_MAXPATHS];
	wxString outputports[FT_MAXPATHS];
	wxString jackname;
	wxString preset;
	wxString rcdir;
	wxString jackdir;
	int pcnt = 2;
	int icnt = 0;
	int ocnt = 0;
	bool connected = true;
	
	SetExitOnFrameDelete(TRUE);

	setupSignals();

	
	if (sizeof(sample_t) != sizeof(float)) {
		fprintf(stderr, "FFTW Mismatch!  You need to build FreqTweak against a single-precision\n");
		fprintf(stderr, "  FFTW library.  See the INSTALL file for instructions.\n");  		
		return FALSE;
	}
	
	// use stderr as log
	wxLog *logger=new wxLogStderr();
	logger->SetTimestamp(NULL);
	wxLog::SetActiveTarget(logger);
	
	wxCmdLineParser parser(argc, argv);
	parser.SetDesc(cmdLineDesc);
	parser.SetLogo(wxT("FreqTweak ") +
		       wxString::FromAscii (freqtweak_version) +
		       wxT("\nCopyright 2002-2004 Jesse Chappell\n")
		       wxT("FreqTweak comes with ABSOLUTELY NO WARRANTY\n")
		       wxT("This is free software, and you are welcome to redistribute it\n")
		       wxT("under certain conditions; see the file COPYING for details\n"));

	int ret = parser.Parse();

	if (ret != 0) {
		// help or error
		return FALSE;
	}

	wxString strval;
	long longval;

	if (parser.Found (wxT("c"), &longval)) {
		if (longval < 1 || longval > FT_MAXPATHS) {
			fprintf(stderr, "Error: channel count must be in range [1-%d]\n", FT_MAXPATHS);
			parser.Usage();
			return FALSE;
		}
		pcnt = (int) longval;
	}

	if (parser.Found (wxT("S"), &jackdir)) {
	       FTioSupport::setDefaultServer ((const char *) jackdir.ToAscii());
	}
	
	if (parser.Found (wxT("n"), &jackname)) {
		// FIXME: needs wchar_t->char conversion
       	       FTioSupport::setDefaultName ((const char *)jackname.ToAscii());
	}

	parser.Found (wxT("r"), &rcdir);
	parser.Found (wxT("p"), &preset);

	
	
	// initialize jack support
	FTioSupport * iosup = FTioSupport::instance();

	if (!iosup->init()) {
		fprintf(stderr, "Error connecting to jack!\n");
		return FALSE;
	}


	
	if (parser.Found (wxT("i"), &strval))
	{
		// parse comma separated values
		wxString port = strval.BeforeFirst(',');
		wxString remain = strval.AfterFirst(',');
		int id=0;
		while (!port.IsEmpty() && id < pcnt) {
			inputports[id++] = port;			
			port = remain.BeforeFirst(',');
			remain = remain.AfterFirst(',');
			++icnt;
		}
		
	}
	else {
		// Do not use default input ports anymore
		icnt = 0;
		
// 		const char ** ports = iosup->getPhysicalInputPorts();
// 		if (ports) {
// 			// default input ports
// 			for (int id=0; id < pcnt && ports[id]; ++id, ++icnt) {
// 				inputports[id] = ports[id];
// 			}

// 			free (ports);
// 		}
	}

	// OUTPUT PORTS
	if (parser.Found (wxT("o"), &strval))
	{
		// parse comma separated values
		wxString port = strval.BeforeFirst(',');
		wxString remain = strval.AfterFirst(',');
		int id=0;
		while (!port.IsEmpty() && id < pcnt) {
			outputports[id++] = port;			
			port = remain.BeforeFirst(',');
			remain = remain.AfterFirst(',');
			++ocnt;
		}
		
	}
	else {
		const char ** ports = iosup->getPhysicalOutputPorts();
		if (ports) {
			// default output ports
			for (int id=0; id < pcnt && ports[id]; ++id, ++ocnt) {
				// FIXME: needs wchar_t->char conversion
				outputports[id] = wxString::FromAscii (ports[id]);
			}
			
			free (ports);
		}
	}


	
	// Create the main application window
	_mainwin = new FTmainwin(pcnt, wxT("FreqTweak"), rcdir,
				 wxPoint(100, 100), wxDefaultSize);

	
	// Show it and tell the application that it's our main window
	_mainwin->SetSize(669,770);
	_mainwin->Show(TRUE);

	SetTopWindow(_mainwin);

	if (connected)
	{
		// only start processing after building mainwin and connected
		// to JACK
		iosup->startProcessing();
		if ( preset.IsEmpty()) {
			// connect initial I/O
			for (int id=0; id < icnt; ++id)
			{
				iosup->connectPathInput(id, (const char *) inputports[id].ToAscii());
			}
			for (int id=0; id < ocnt; ++id)
			{
				iosup->connectPathOutput(id, (const char *) outputports[id].ToAscii());
			}

			// load last settings
			_mainwin->loadPreset(wxT(""), true);
		}
		else {
			_mainwin->loadPreset(preset);
		}
	}
	
	_mainwin->updateDisplay();
	
		
	// success: wxApp::OnRun() will be called which will enter the main message
	// loop and the application will run. If we returned FALSE here, the
	// application would exit immediately.
	return TRUE;
}
