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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
    MA 02110-1301 USA.

    $Id: RingBuffer.cpp,v 1.2 2004/11/10 17:16:08 trutkin Exp $
*/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/mman.h>
#include <cstring>
#include "RingBuffer.hpp"

RingBuffer::~RingBuffer ()
{
	if (mlocked) {
		munlock (buf, size);
	}
	delete [] buf;
}

size_t
RingBuffer::read (char *dest, size_t cnt)

{
	size_t free_cnt;
	size_t cnt2;
	size_t to_read;
	size_t n1, n2;

	if ((free_cnt = read_space ()) == 0) {
		return 0;
	}

	to_read = cnt > free_cnt ? free_cnt : cnt;
	
	cnt2 = read_ptr + to_read;

	if (cnt2 > size) {
		n1 = size - read_ptr;
		n2 = cnt2 & size_mask;
	} else {
		n1 = to_read;
		n2 = 0;
	}
	
	memcpy (dest, &buf[read_ptr], n1);
	read_ptr += n1;
	read_ptr &= size_mask;

	if (n2) {
		memcpy (dest+n1, &buf[read_ptr], n2);
		read_ptr += n2;
		read_ptr &= size_mask;
	}

	return to_read;
}

size_t
RingBuffer::write (char *src, size_t cnt)

{
	size_t free_cnt;
	size_t cnt2;
	size_t to_write;
	size_t n1, n2;

	if ((free_cnt = write_space ()) == 0) {
		return 0;
	}

	to_write = cnt > free_cnt ? free_cnt : cnt;
	
	cnt2 = write_ptr + to_write;

	if (cnt2 > size) {
		n1 = size - write_ptr;
		n2 = cnt2 & size_mask;
	} else {
		n1 = to_write;
		n2 = 0;
	}

	memcpy (&buf[write_ptr], src, n1);
	write_ptr += n1;
	write_ptr &= size_mask;

	if (n2) {
		memcpy (&buf[write_ptr], src+n1, n2);
		write_ptr += n2;
		write_ptr &= size_mask;
	}

	return to_write;
}

int
RingBuffer::mlock ()

{
	if (::mlock (buf, size)) {
		return -1;
	} 
	mlocked = true;
	return 0;
}

void RingBuffer::mem_set ( char val)
{
	::memset ( buf, val, size ); 
}


void
RingBuffer::get_read_vector (rw_vector *vec)

{
	size_t free_cnt;
	size_t cnt2;
	size_t w, r;
	
	w = write_ptr;
	r = read_ptr;
	
	if (w > r) {
		free_cnt = w - r;
	} else {
		free_cnt = (w - r + size) & size_mask;
	}

	cnt2 = r + free_cnt;

	if (cnt2 > size) {
		/* Two part vector: the rest of the buffer after the
		   current write ptr, plus some from the start of 
		   the buffer.
		*/

		vec[0].buf = &buf[r];
		vec[0].len = size - r;
		vec[1].buf = buf;
		vec[1].len = cnt2 & size_mask;

	} else {
		
		/* Single part vector: just the rest of the buffer */
		
		vec[0].buf = &buf[r];
		vec[0].len = free_cnt;
		vec[1].len = 0;
	}
}

void
RingBuffer::get_write_vector (rw_vector *vec)

{
	size_t free_cnt;
	size_t cnt2;
	size_t w, r;
	
	w = write_ptr;
	r = read_ptr;
	
	if (w > r) {
		free_cnt = ((r - w + size) & size_mask) - 1;
	} else if (w < r) {
		free_cnt = (r - w) - 1;
	} else {
		free_cnt = size - 1;
	}
	
	cnt2 = w + free_cnt;

	if (cnt2 > size) {
		
		/* Two part vector: the rest of the buffer after the
		   current write ptr, plus some from the start of 
		   the buffer.
		*/

		vec[0].buf = &buf[w];
		vec[0].len = size - w;
		vec[1].buf = buf;
		vec[1].len = cnt2 & size_mask;
	} else {
		vec[0].buf = &buf[w];
		vec[0].len = free_cnt;
		vec[1].len = 0;
	}
}




