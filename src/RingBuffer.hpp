/*
    Copyright (C) 2000 Paul Barton-Davis 

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    $Id: RingBuffer.hpp,v 1.1.1.1 2002/10/13 04:13:34 essej Exp $
*/

#ifndef __pbd_ringbuffer_h__
#define __pbd_ringbuffer_h__

#include <sys/types.h>

class RingBuffer 
{
public:
    RingBuffer (int sz) {
	    int power_of_two;

	    for (power_of_two = 1; 
		 1<<power_of_two < sz; 
		 power_of_two++);

	    size = 1<<power_of_two;
	    size_mask = size;
	    size_mask -= 1;
	    write_ptr = 0;
	    read_ptr = 0;
	    buf = new char[size];
	    mlocked = false;
    };

    virtual ~RingBuffer();
    
    void reset () {
	    /* How can this be thread safe ? */
	    read_ptr = 0;
	    write_ptr = 0;
    }

    void mem_set ( char val);
	
    int     mlock ();
    size_t  read (char *dest, size_t cnt);
    size_t  write (char *src, size_t cnt);

    struct rw_vector {
	char *buf;
	size_t len;
    };

    void get_read_vector (rw_vector *);
    void get_write_vector (rw_vector *);

    void write_advance (size_t cnt) {
	    write_ptr += cnt;
	    write_ptr &= size_mask;
    }

    void read_advance (size_t cnt) {
	    read_ptr += cnt;
	    read_ptr &= size_mask;
    }

    size_t write_space () {
	    size_t w, r;

	    w = write_ptr;
	    r = read_ptr;

	    if (w > r) {
		    return ((r - w + size) & size_mask) - 1;
	    } else if (w < r) {
		    return (r - w) - 1;
	    } else {
		    return size - 1;
	    }
    }

    size_t read_space () {
	    size_t w, r;

	    w = write_ptr;
	    r = read_ptr;

	    if (w > r) {
		    return w - r;
	    } else {
		    return (w - r + size) & size_mask;
	    }
    }

    
  protected:
    char *buf;
    volatile size_t write_ptr;
    volatile size_t read_ptr;
    size_t size;
    size_t size_mask;
    bool mlocked;
};


#endif /* __pbd_ringbuffer_h__ */
