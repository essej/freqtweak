/*
** Copyright (C) 2003 Jesse Chappell <jesse@essej.net>
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

#ifndef __FTMODULATORI_HPP__
#define __FTMODULATORI_HPP__

#include "FTtypes.hpp"
#include <string>
#include <list>
#include <algorithm>
#include <sigc++/sigc++.h>
#include <iostream>


#include "LockMonitor.hpp"
#include "FTspectrumModifier.hpp"


class FTmodulatorI
	: public FTspectrumModifier::Listener
{
  public:
	virtual ~FTmodulatorI();

	virtual FTmodulatorI * clone() = 0;
	
	virtual void initialize() = 0;

	virtual void modulate (nframes_t current_frame, fft_data * fftdata, unsigned int fftn, sample_t * timedata, nframes_t nframes) = 0;


	virtual void goingAway(FTspectrumModifier * ft);

	typedef std::list<FTspectrumModifier *> SpecModList;
	
	virtual void addSpecMod (FTspectrumModifier * specmod);
	virtual void removeSpecMod (FTspectrumModifier * specmod);
	virtual void clearSpecMods ();
	virtual void getSpecMods (SpecModList & mods);
	virtual bool hasSpecMod (FTspectrumModifier *specmod);
	
	virtual std::string getUserName() { return _userName; }
	virtual void setUserName (std::string username) { _userName = username; }

	virtual std::string getName() { return _name; }

	virtual void setFFTsize (unsigned int fftn) { _fftN = fftn; }
		
	virtual void setSampleRate (nframes_t rate) { _sampleRate = rate; }
	virtual nframes_t getSampleRate() { return _sampleRate; }

	virtual const string & getConfName() { return _confname; }


	virtual bool getBypassed() { return _bypassed; }
	virtual void setBypassed(bool byp) { _bypassed = byp; }

	SigC::Signal1<void, FTmodulatorI *> GoingAway;


	
	class Control
	{
	public:
		enum Type {
			BooleanType = 1,
			IntegerType,
			FloatType,
			StringType,
			EnumType
		};

		Control (Type t, std::string confname, std::string name, std::string units) : _type(t), _confname(confname), _name(name), _units(units) {}
		
		Type getType() { return _type; }
		std::string getConfName() { return _confname; }
		std::string getName() { return _name; }
		std::string getUnits() { return _units; }
		
		inline bool getValue(bool & val);
		inline bool getValue(int & val);
		inline bool getValue(float & val);
		// for string or enum type
		inline bool getValue(std::string & val);

		inline bool getEnumStrings (std::list<std::string> & vals);

		inline bool getBounds(int & lb, int & ub);
		inline bool getBounds(float & lb, float & ub);
		
		
		inline bool setValue(bool val);
		inline bool setValue(int val);
		inline bool setValue(float val);
		// for string or enum type
		inline bool setValue(const std::string & val);

		int _intLB, _intUB;
		float _floatLB, _floatUB;
		std::list<std::string> _enumList;

		friend class FTmodulatorI;		
		
	protected:

		
		Type _type;
		std::string _confname;
		std::string _name;
		std::string _units;
		
		std::string _stringVal;
		float  _floatVal;
		int    _intVal;
		bool   _boolVal;

	};

	typedef std::list<Control *> ControlList;

	virtual void getControls (ControlList & conlist) {
		conlist.insert (conlist.begin(), _controls.begin(), _controls.end());
	}


   protected:

	FTmodulatorI(std::string confname, std::string name, nframes_t samplerate, unsigned int fftn);
	
	ControlList _controls;

	SpecModList _specMods;
	PBD::NonBlockingLock _specmodLock;

	bool _inited;
	std::string _name;
	std::string _confname;
	std::string _userName;
	bool _bypassed;
	nframes_t _sampleRate;
	unsigned int _fftN;
	int _id;
};


inline bool FTmodulatorI::Control::getValue(bool & val)
{
	if (_type != BooleanType) return false;
	val = _boolVal;
	return true;
}

inline bool FTmodulatorI::Control::getValue(int & val)
{
	if (_type != IntegerType) return false;
	val = _intVal;
	return true;
}

inline bool FTmodulatorI::Control::getValue(float & val)
{
	if (_type != FloatType) return false;
	val = _floatVal;
	return true;
}

// for string or enum type
inline bool FTmodulatorI::Control::getValue(std::string & val)
{
	if (_type != StringType && _type != EnumType) return false;
	val = _stringVal;
	return true;
}

inline bool FTmodulatorI::Control::getEnumStrings (std::list<std::string> & vals)
{
	if (_type != EnumType) return false;
	
	vals.insert(vals.begin(), _enumList.begin(), _enumList.end());
	return true;
}

inline bool FTmodulatorI::Control::getBounds(int & lb, int & ub)
{
	if (_type != IntegerType) return false;
	lb = _intLB;
	ub = _intUB;
	return true;
}

inline bool FTmodulatorI::Control::getBounds(float & lb, float & ub)
{
	if (_type != FloatType) return false;
	lb = _floatLB;
	ub = _floatUB;
	return true;
}


inline bool FTmodulatorI::Control::setValue(bool val)
{
	if (_type != BooleanType) return false;
	_boolVal = val;
	return true;
}

inline bool FTmodulatorI::Control::setValue(int val)
{
	if (_type != IntegerType) return false;
	_intVal = val;
	return true;
}

inline bool FTmodulatorI::Control::setValue(float val)
{
	if (_type != FloatType) return false;
	_floatVal = val;
	return true;
}

// for string or enum type
inline bool FTmodulatorI::Control::setValue(const std::string & val)
{
	if (_type == StringType) {
		_stringVal = val;
		return true;
	}
	else if (_type == EnumType && std::find(_enumList.begin(), _enumList.end(), val) != _enumList.end()) {
		_stringVal = val;
		return true;
	}

	return false;
}



#endif
