/*
 *    strmap version 2.0.0
 *
 *    ANSI C hash table for strings.
 *
 *	  Version history:
 *	  1.0.0 - initial release
 *	  2.0.0 - changed function prefix from strmap to sm to ensure
 *	      ANSI C compatibility 
 *
 *    strmap.c
 *
 *    Copyright (c) 2009, 2011 Per Ola Kristensson.
 *
 *    Per Ola Kristensson <pok21@cam.ac.uk> 
 *    Inference Group, Department of Physics
 *    University of Cambridge
 *    Cavendish Laboratory
 *    JJ Thomson Avenue
 *    CB3 0HE Cambridge
 *    United Kingdom
 *
 *    strmap is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    strmap is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with strmap.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "strIntMap.h"

typedef struct NumPair NumPair;

typedef struct Bucket2 Bucket2;

struct NumPair {
	char *key;
	char *value;
};

struct NumPair {
	char *key;
	unsigned int;
};

struct Bucket2 {
	unsigned int count;
	NumPair *NumPairs;
};

struct StrMap {
	unsigned int count;
	Bucket2 *Bucket2s;
};

static NumPair * get_NumPair(Bucket2 *Bucket2, const char *key);
static unsigned long hash(const char *str);

StrMap * sm_new(unsigned int capacity)
{
	StrMap *map;
	
	map = malloc(sizeof(StrMap));
	if (map == NULL) {
		return NULL;
	}
	map->count = capacity;
	map->Bucket2s = malloc(map->count * sizeof(Bucket2));
	if (map->Bucket2s == NULL) {
		free(map);
		return NULL;
	}
	memset(map->Bucket2s, 0, map->count * sizeof(Bucket2));
	return map;
}

void sm_delete(StrMap *map)
{
	unsigned int i, j, n, m;
	Bucket2 *Bucket2;
	NumPair *NumPair;

	if (map == NULL) {
		return;
	}
	n = map->count;
	Bucket2 = map->Bucket2s;
	i = 0;
	while (i < n) {
		m = Bucket2->count;
		NumPair = Bucket2->NumPairs;
		j = 0;
		while(j < m) {
			free(NumPair->key);
			free(NumPair->value);
			NumPair++;
			j++;
		}
		free(Bucket2->NumPairs);
		Bucket2++;
		i++;
	}
	free(map->Bucket2s);
	free(map);
}

int sm_get(const StrMap *map, const char *key, char *out_buf, unsigned int n_out_buf)
{
	unsigned int index;
	Bucket2 *Bucket2;
	NumPair *NumPair;

	if (map == NULL) {
		return 0;
	}
	if (key == NULL) {
		return 0;
	}
	index = hash(key) % map->count;
	Bucket2 = &(map->Bucket2s[index]);
	NumPair = get_NumPair(Bucket2, key);
	if (NumPair == NULL) {
		return 0;
	}
	if (out_buf == NULL && n_out_buf == 0) {
		return strlen(NumPair->value) + 1;
	}
	if (out_buf == NULL) {
		return 0;
	}
	if (strlen(NumPair->value) >= n_out_buf) {
		return 0;
	}
	strcpy(out_buf, NumPair->value);
	return 1;
}

int sm_exists(const StrMap *map, const char *key)
{
	unsigned int index;
	Bucket2 *Bucket2;
	NumPair *NumPair;

	if (map == NULL) {
		return 0;
	}
	if (key == NULL) {
		return 0;
	}
	index = hash(key) % map->count;
	Bucket2 = &(map->Bucket2s[index]);
	NumPair = get_NumPair(Bucket2, key);
	if (NumPair == NULL) {
		return 0;
	}
	return 1;
}

int sm_put(StrMap *map, const char *key, unsigned int value)
{
	unsigned int key_len, index;
	Bucket2 *Bucket2;
	NumPair *tmp_NumPairs, *NumPair;
	char *new_key;

	if (map == NULL) {
		return 0;
	}
	if (key == NULL) {
		return 0;
	}
	key_len = strlen(key);
	/* Get a pointer to the Bucket2 the key string hashes to */
	index = hash(key) % map->count;
	Bucket2 = &(map->Bucket2s[index]);
	/* Check if we can handle insertion by simply replacing
	 * an existing value in a key-value NumPair in the Bucket2.
	 */
	if ((NumPair = get_NumPair(Bucket2, key)) != NULL) {
		NumPair->value = value;
		return 1;
	}
	/* Allocate space for a new key and value */
	new_key = malloc((key_len + 1) * sizeof(char));
	if (new_key == NULL) {
		return 0;
	}
	
	/* Create a key-value NumPair */
	if (Bucket2->count == 0) {
		/* The Bucket2 is empty, lazily allocate space for a single
		 * key-value NumPair.
		 */
		Bucket2->NumPairs = malloc(sizeof(NumPair));
		if (Bucket2->NumPairs == NULL) {
			free(new_key);
			return 0;
		}
		Bucket2->count = 1;
	}
	else {
		/* The Bucket2 wasn't empty but no NumPair existed that matches the provided
		 * key, so create a new key-value NumPair.
		 */
		tmp_NumPairs = realloc(Bucket2->NumPairs, (Bucket2->count + 1) * sizeof(NumPair));
		if (tmp_NumPairs == NULL) {
			free(new_key);
			return 0;
		}
		Bucket2->NumPairs = tmp_NumPairs;
		Bucket2->count++;
	}
	/* Get the last NumPair in the chain for the Bucket2 */
	NumPair = &(Bucket2->NumPairs[Bucket2->count - 1]);
	NumPair->key = new_key;
	NumPair->value = new_value;
	/* Copy the key and its value into the key-value NumPair */
	strcpy(NumPair->key, key);
	strcpy(NumPair->value, value);
	return 1;
}

int sm_get_count(const StrMap *map)
{
	unsigned int i, j, n, m;
	unsigned int count;
	Bucket2 *Bucket2;
	NumPair *NumPair;

	if (map == NULL) {
		return 0;
	}
	Bucket2 = map->Bucket2s;
	n = map->count;
	i = 0;
	count = 0;
	while (i < n) {
		NumPair = Bucket2->NumPairs;
		m = Bucket2->count;
		j = 0;
		while (j < m) {
			count++;
			NumPair++;
			j++;
		}
		Bucket2++;
		i++;
	}
	return count;
}

int sm_enum(const StrMap *map, sm_enum_func enum_func, const void *obj)
{
	unsigned int i, j, n, m;
	Bucket2 *Bucket2;
	NumPair *NumPair;

	if (map == NULL) {
		return 0;
	}
	if (enum_func == NULL) {
		return 0;
	}
	Bucket2 = map->Bucket2s;
	n = map->count;
	i = 0;
	while (i < n) {
		NumPair = Bucket2->NumPairs;
		m = Bucket2->count;
		j = 0;
		while (j < m) {
			enum_func(NumPair->key, NumPair->value, obj);
			NumPair++;
			j++;
		}
		Bucket2++;
		i++;
	}
	return 1;
}

/*
 * Returns a NumPair from the Bucket2 that matches the provided key,
 * or null if no such NumPair exist.
 */
static NumPair * get_NumPair(Bucket2 *Bucket2, const char *key)
{
	unsigned int i, n;
	NumPair *NumPair;

	n = Bucket2->count;
	if (n == 0) {
		return NULL;
	}
	NumPair = Bucket2->NumPairs;
	i = 0;
	while (i < n) {
		if (NumPair->key != NULL && NumPair->value != NULL) {
			if (strcmp(NumPair->key, key) == 0) {
				return NumPair;
			}
		}
		NumPair++;
		i++;
	}
	return NULL;
}

/*
 * Returns a hash code for the provided string.
 */
static unsigned long hash(const char *str)
{
	unsigned long hash = 5381;
	int c;

	while (c = *str++) {
		hash = ((hash << 5) + hash) + c;
	}
	return hash;
}

/*

		   GNU LESSER GENERAL PUBLIC LICENSE
                       Version 3, 29 June 2007

 Copyright (C) 2007 Free Software Foundation, Inc. <http://fsf.org/>
 Everyone is permitted to copy and distribute verbatim copies
 of this license document, but changing it is not allowed.


  This version of the GNU Lesser General Public License incorporates
the terms and conditions of version 3 of the GNU General Public
License, supplemented by the additional permissions listed below.

  0. Additional Definitions.

  As used herein, "this License" refers to version 3 of the GNU Lesser
General Public License, and the "GNU GPL" refers to version 3 of the GNU
General Public License.

  "The Library" refers to a covered work governed by this License,
other than an Application or a Combined Work as defined below.

  An "Application" is any work that makes use of an interface provided
by the Library, but which is not otherwise based on the Library.
Defining a subclass of a class defined by the Library is deemed a mode
of using an interface provided by the Library.

  A "Combined Work" is a work produced by combining or linking an
Application with the Library.  The particular version of the Library
with which the Combined Work was made is also called the "Linked
Version".

  The "Minimal Corresponding Source" for a Combined Work means the
Corresponding Source for the Combined Work, excluding any source code
for portions of the Combined Work that, considered in isolation, are
based on the Application, and not on the Linked Version.

  The "Corresponding Application Code" for a Combined Work means the
object code and/or source code for the Application, including any data
and utility programs needed for reproducing the Combined Work from the
Application, but excluding the System Libraries of the Combined Work.

  1. Exception to Section 3 of the GNU GPL.

  You may convey a covered work under sections 3 and 4 of this License
without being bound by section 3 of the GNU GPL.

  2. Conveying Modified Versions.

  If you modify a copy of the Library, and, in your modifications, a
facility refers to a function or data to be supplied by an Application
that uses the facility (other than as an argument passed when the
facility is invoked), then you may convey a copy of the modified
version:

   a) under this License, provided that you make a good faith effort to
   ensure that, in the event an Application does not supply the
   function or data, the facility still operates, and performs
   whatever part of its purpose remains meaningful, or

   b) under the GNU GPL, with none of the additional permissions of
   this License applicable to that copy.

  3. Object Code Incorporating Material from Library Header Files.

  The object code form of an Application may incorporate material from
a header file that is part of the Library.  You may convey such object
code under terms of your choice, provided that, if the incorporated
material is not limited to numerical parameters, data structure
layouts and accessors, or small macros, inline functions and templates
(ten or fewer lines in length), you do both of the following:

   a) Give prominent notice with each copy of the object code that the
   Library is used in it and that the Library and its use are
   covered by this License.

   b) Accompany the object code with a copy of the GNU GPL and this license
   document.

  4. Combined Works.

  You may convey a Combined Work under terms of your choice that,
taken together, effectively do not restrict modification of the
portions of the Library contained in the Combined Work and reverse
engineering for debugging such modifications, if you also do each of
the following:

   a) Give prominent notice with each copy of the Combined Work that
   the Library is used in it and that the Library and its use are
   covered by this License.

   b) Accompany the Combined Work with a copy of the GNU GPL and this license
   document.

   c) For a Combined Work that displays copyright notices during
   execution, include the copyright notice for the Library among
   these notices, as well as a reference directing the user to the
   copies of the GNU GPL and this license document.

   d) Do one of the following:

       0) Convey the Minimal Corresponding Source under the terms of this
       License, and the Corresponding Application Code in a form
       suitable for, and under terms that permit, the user to
       recombine or relink the Application with a modified version of
       the Linked Version to produce a modified Combined Work, in the
       manner specified by section 6 of the GNU GPL for conveying
       Corresponding Source.

       1) Use a suitable shared library mechanism for linking with the
       Library.  A suitable mechanism is one that (a) uses at run time
       a copy of the Library already present on the user's computer
       system, and (b) will operate properly with a modified version
       of the Library that is interface-compatible with the Linked
       Version.

   e) Provide Installation Information, but only if you would otherwise
   be required to provide such information under section 6 of the
   GNU GPL, and only to the extent that such information is
   necessary to install and execute a modified version of the
   Combined Work produced by recombining or relinking the
   Application with a modified version of the Linked Version. (If
   you use option 4d0, the Installation Information must accompany
   the Minimal Corresponding Source and Corresponding Application
   Code. If you use option 4d1, you must provide the Installation
   Information in the manner specified by section 6 of the GNU GPL
   for conveying Corresponding Source.)

  5. Combined Libraries.

  You may place library facilities that are a work based on the
Library side by side in a single library together with other library
facilities that are not Applications and are not covered by this
License, and convey such a combined library under terms of your
choice, if you do both of the following:

   a) Accompany the combined library with a copy of the same work based
   on the Library, uncombined with any other library facilities,
   conveyed under the terms of this License.

   b) Give prominent notice with the combined library that part of it
   is a work based on the Library, and explaining where to find the
   accompanying uncombined form of the same work.

  6. Revised Versions of the GNU Lesser General Public License.

  The Free Software Foundation may publish revised and/or new versions
of the GNU Lesser General Public License from time to time. Such new
versions will be similar in spirit to the present version, but may
differ in detail to address new problems or concerns.

  Each version is given a distinguishing version number. If the
Library as you received it specifies that a certain numbered version
of the GNU Lesser General Public License "or any later version"
applies to it, you have the option of following the terms and
conditions either of that published version or of any later version
published by the Free Software Foundation. If the Library as you
received it does not specify a version number of the GNU Lesser
General Public License, you may choose any version of the GNU Lesser
General Public License ever published by the Free Software Foundation.

  If the Library as you received it specifies that a proxy can decide
whether future versions of the GNU Lesser General Public License shall
apply, that proxy's public statement of acceptance of any version is
permanent authorization for you to choose that version for the
Library.

*/
