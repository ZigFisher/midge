
/*
	Copyright 2001, 2002 Georges Menie (www.menie.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


/* waiting loop which periodically call a function
 * passed as argument. The loop exits when :
 * - the timeout t is reached (t in ms) or
 * - the int pointed to by f if <> 0
 */
void busyWait (int t, void (*work) (void), int *f)
{
    t >>= 6;
    if (t==0) t=1;
    
	while (t) {
			work ();
			if (f)
				if (*f)
					return;
			tick_Delay10ms (1);
		--t;
	}
}
