// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		15aug04	initial version
 
        minimal base class for objects not derived from CObject
 
*/

class WObject {
public:
	WObject() {}	// default ctor required

private:
	WObject(const WObject&);	// prevent bitwise copy
	WObject& operator=(const WObject&);	// prevent bitwise assignment
};
