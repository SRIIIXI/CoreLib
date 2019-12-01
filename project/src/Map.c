/*
BSD 2-Clause License

Copyright (c) 2017, Subrato Roy (subratoroy@hotmail.com)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Map.h"

#include <string.h>
#include <memory.h>
#include <stdlib.h>

void mapAllocate(Map* mptr)
{

}

void mapClear(Map* mptr)
{

}

void mapInsert(Map* mptr, Node key, const Node data)
{

}

void mapRemove(Map* mptr, const Node key)
{

}

size_t mapItemCount()
{
	return 0;
}

size_t mapIndexOf(const Node key)
{
	return 0;
}

Node* mapGetkey(Map* mptr, const Node data)
{
	return NULL;
}

Node* mapGetvalue(Map* mptr, const Node key)
{
	return NULL;
}

KeyValue* mapGetAt(Map* mptr, unsigned int atpos)
{
	return NULL;
}

KeyValue* mapGetFirst(Map* mptr)
{
	return NULL;
}

KeyValue* mapGetLast(Map* mptr)
{
	return NULL;
}

KeyValue* mapGetNext(Map* mptr)
{
	return NULL;
}

KeyValue* mapGetPrevious(Map* mptr)
{
	return NULL;
}

/*
namespace CoreLibrary
{
	#ifndef MAP_RESIZE_FACTOR
	#define MAP_RESIZE_FACTOR 8
	#endif

	template <class K, class V>
	class Map
	{
	public:
		Map();
		virtual ~Map();

		void insert(const K key, const V data);
		void remove(const K key);

		unsigned long count();
		unsigned long indexOf(const K key);
		void clear();
		K* key(const V data) const;
		V* value(const K key);

		V* getAt(unsigned int atpos);
		V* getFirst();
		V* getLast();
		V* getNext();
		V* getPrevious();

		V* operator[](K key);
	private:
		void allocate();

		unsigned long _Count;
		unsigned long _MapMemorySize;
		KeyValue<K, V>* _MapMemory;
		long _IteratorPosition;
	};

	template <class K, class V>
	Map<K, V>::Map()
	{
		_Count = 0;
		_IteratorPosition = 1;
		_MapMemorySize = MAP_RESIZE_FACTOR;
		_MapMemory = new KeyValue<K, V>[_MapMemorySize * sizeof(KeyValue<K, V>)];
		memset((void*)_MapMemory, 0, _MapMemorySize * sizeof(KeyValue<K, V>));
	}

	template <class K, class V>
	void Map<K, V>::allocate()
	{
		_Count = 0;
		_IteratorPosition = 1;
		_MapMemorySize = MAP_RESIZE_FACTOR;
		_MapMemory = new KeyValue<K, V>[_MapMemorySize * sizeof(KeyValue<K, V>)];
		memset((void*)_MapMemory, 0, _MapMemorySize * sizeof(KeyValue<K, V>));
	}


	template <class K, class V>
	Map<K, V>::~Map()
	{
		clear();
	}

	template <class K, class V>
	void Map<K, V>::insert(const K key, const V data)
	{
		KeyValue<K, V> newitem(key, data);

		//Special case for first entry
		if (_Count == 0)
		{
			allocate();
			_MapMemory[_Count] = newitem;
			_Count++;
			return;
		}

		//Check whether we have reached the end of array already or not
		if (_Count == _MapMemorySize)
		{
			// Double the array size
			unsigned long newsz = _MapMemorySize + MAP_RESIZE_FACTOR;
			KeyValue<K, V>* temparray = new KeyValue<K, V>[newsz * sizeof(KeyValue<K, V>)];
			memset((void*)temparray, 0, newsz * sizeof(KeyValue<K, V>));
			memcpy((void*)temparray, (void*)_MapMemory, _MapMemorySize * sizeof(KeyValue<K, V>));
			delete[] _MapMemory;
			_MapMemory = temparray;
			_MapMemorySize = newsz;
		}

		int lowerbound = 0;
		int upperbound = _Count - 1;
		int middle = -1;

		while (lowerbound <= upperbound)
		{
			//Middle element
			middle = (lowerbound + upperbound) / 2;

			//If the search key on the left half
			if (newitem.Key < _MapMemory[middle].Key)
			{
				//Update right index
				upperbound = middle - 1;
			}
			//If the search key on the right half
			else
			{
				if (newitem.Key > _MapMemory[middle].Key)
				{
					//Update the left index
					lowerbound = middle + 1;
				}
				//We found the key
				else
				{
					break;
				}
			}
		}

		memcpy((void*)&_MapMemory[lowerbound + 1], (void*)&_MapMemory[lowerbound], (_Count - lowerbound) * sizeof(KeyValue<K, V>));
		_MapMemory[lowerbound] = newitem;
		_Count++;
	}

	template <class K, class V>
	void Map<K, V>::remove(const K key)
	{
		unsigned long idx = indexOf(key);
		if (idx != -1)
		{
			memcpy((void*)&_MapMemory[idx], (void*)&_MapMemory[idx + 1], (_Count - idx) * sizeof(KeyValue<K, V>));
			memset((void*)&_MapMemory[_Count - 1], 0, sizeof(KeyValue<K, V>));
			_Count--;
		}
	}

	template <class K, class V>
	void Map<K, V>::clear()
	{
		if (_MapMemory != nullptr && _MapMemorySize != 0)
		{
			delete[] _MapMemory;
		}
		_MapMemory = nullptr;
		_MapMemorySize = 0;
		_Count = 0;
		_IteratorPosition = -1;
	}

	template <class K, class V>
	unsigned long Map<K, V>::count()
	{
		return _Count;
	}

	template <class K, class V>
	V* Map<K, V>::value(const K key)
	{
		unsigned long idx = indexOf(key);

		if (idx == -1)
		{
			return nullptr;
		}

		return &_MapMemory[idx].Value;
	}

	template <class K, class V>
	K* Map<K, V>::key(const V data) const
	{
		for (int ctr = 0; ctr < _Count; ctr++)
		{
			if (_MapMemory[ctr].Value == data)
			{
				return &_MapMemory[ctr].Key;
			}
		}

		return nullptr;
	}

	template <class K, class V>
	unsigned long Map<K, V>::indexOf(const K key)
	{
		int lowerbound = 0;
		int upperbound = _Count - 1;
		int middle = -1;

		K tempkey = key;

		while (lowerbound <= upperbound)
		{
			//Middle element
			middle = (lowerbound + upperbound) / 2;

			//If the search key on the left half
			if (tempkey < _MapMemory[middle].Key)
			{
				//Update right index
				upperbound = middle - 1;
			}
			//If the search key on the right half
			else
			{
				if (tempkey > _MapMemory[middle].Key)
				{
					//Update the left index
					lowerbound = middle + 1;
				}
				//We found the key
				else
				{
					return middle;
				}
			}
		}

		return -1;
	}

	template <class K, class V>
	V* Map<K, V>::getAt(unsigned int atpos)
	{
		if (atpos > _Count - 1)
		{
			return nullptr;
		}

		return &_MapMemory[atpos].Value;
	}

	template <class K, class V>
	V* Map<K, V>::getFirst()
	{
		_IteratorPosition = 0;

		return &_MapMemory[_IteratorPosition].Value;
	}

	template <class K, class V>
	V* Map<K, V>::getLast()
	{
		_IteratorPosition = _Count - 1;

		return &_MapMemory[_IteratorPosition].Value;
	}

	template <class K, class V>
	V* Map<K, V>::getNext()
	{
		_IteratorPosition++;

		return &_MapMemory[_IteratorPosition].Value;
	}

	template <class K, class V>
	V* Map<K, V>::getPrevious()
	{
		_IteratorPosition--;

		return &_MapMemory[_IteratorPosition].Value;
	}

	template <class K, class V>
	V* Map<K, V>::operator[](K key)
	{
		unsigned long idx = indexOf(key);

		if (idx == -1)
		{
			return nullptr;
		}

		return &_MapMemory[idx].Value;
	}
}
*/

