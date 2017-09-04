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

#ifndef __FTSPECTRUMMODIFIER_HPP__
#define __FTSPECTRUMMODIFIER_HPP__

#include "FTtypes.hpp"

#include "xml++.hpp"

#include <string>
#include <list>
using namespace std;



class FTspectrumModifier
{
  public:

	enum ModifierType
	{
		GAIN_MODIFIER = 0,
		TIME_MODIFIER,
		UNIFORM_MODIFIER,
		RATIO_MODIFIER,
		FREQ_MODIFIER,
		DB_MODIFIER,
		SEMITONE_MODIFIER,
		POS_GAIN_MODIFIER,
		// must be last!
		NULL_MODIFIER, 
	};

	class Listener {
	  public:
	  	virtual ~Listener() {}
		virtual void goingAway(FTspectrumModifier * ft) = 0;
	};
	
	
	FTspectrumModifier(const string & name, const string &configName, int group,
			   ModifierType mtype, SpecModType smtype, int length=512, float initval=0.0);

	virtual ~FTspectrumModifier();

	void setId (int id) { _id = id; }
	int getId () { return _id; }
	
	void setLength(int length);
	int getLength() { return _length; }

	string getName() { return _name; }
	void setName(const string & name) { _name = name; }

	string getConfigName() { return _configName; }
	void setConfigName(const string & name) { _configName = name; }

	int getGroup() { return _group; }
	void setGroup(int grp) { _group = grp; }
	
	float * getValues();

	ModifierType getModifierType() { return _modType; }
	SpecModType getSpecModifierType() { return _specmodType; }

	FTspectrumModifier * getLink() { return _linkedTo; }
	bool link (FTspectrumModifier *specmod);
	void unlink (bool unlinksources=true);

	void setRange(float min, float max) { _min = min; _max = max; }
	void getRange(float &min, float &max) { min = _min; max = _max; }
	float getMin() const { return _min;}
	float getMax() const { return _max;}


	void setBypassed (bool flag) { _bypassed = flag; }
	bool getBypassed () { return _bypassed; }

	void setDirty (bool val) { _dirty = val; }
	// this is as close of a test-and-set as I need
	bool getDirty (bool tas=false, bool val=false)
		{
			if (_linkedTo) {
				return _linkedTo->getDirty(false);
			}
				
			if (_dirty) {
				if (tas) {
					_dirty = val;
				}
				return true;
			}
			return false;
		}

	
	// resets all bins to constructed value
	void reset();

	void copy (FTspectrumModifier *specmod);
	
	list<FTspectrumModifier*> & getLinkedFrom() { return _linkedFrom; }

	bool isLinkedFrom (FTspectrumModifier *specmod);

	// user notification
	void registerListener (Listener * listener);
	void unregisterListener (Listener *listener);

	XMLNode * getExtraNode();
	void setExtraNode(XMLNode * node);

	
  protected:

	void addedLinkFrom (FTspectrumModifier * specmod);
	void removedLinkFrom (FTspectrumModifier * specmod);
	
	ModifierType _modType;
	SpecModType _specmodType;

	string _name;
	string _configName;
	int _group;
	
	// might point to a linked value array
	float * _values;

	float * _tmpvalues; // used for copying
	
	int _length;

	FTspectrumModifier *_linkedTo;

	float _min, _max;
	float _initval;

	list<FTspectrumModifier*> _linkedFrom;

	int _id;
	bool _bypassed;
	bool _dirty;
	
	list<Listener *> _listenerList;

	XMLNode * _extra_node;

};


#endif
