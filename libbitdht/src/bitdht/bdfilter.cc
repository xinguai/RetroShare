
/*
 * bitdht/bdfilter.cc
 *
 * BitDHT: An Flexible DHT library.
 *
 * Copyright 2010 by Robert Fernie
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License Version 3 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems to "bitdht@lunamutt.com".
 *
 */


#include "bitdht/bdfilter.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

/**
 * #define DEBUG_FILTER 1
**/


bdFilter::bdFilter(const bdNodeId *ownId, std::list<bdFilteredPeer> &startList, 
		uint32_t filterFlags, bdDhtFunctions *fns)
{
	/* */
	mOwnId = *ownId;
	mFns = fns;

	time_t now = time(NULL);
	std::list<bdFilteredPeer>::iterator it;

	for(it = startList.begin(); it != startList.end(); it++)
	{
		mFiltered.push_back(*it);
	}

	mFilterFlags = filterFlags;
}

bool bdFilter::filtered(std::list<bdFilteredPeer> &answer)
{
	answer = mFiltered;
	return (answer.size() > 0);
}


int bdFilter::checkPeer(const bdId *id, uint32_t mode)
{
	bool add = false;
	uint32_t flags = 0;
	if ((mFilterFlags & BITDHT_FILTER_REASON_OWNID) && 
			isOwnIdWithoutBitDhtFlags(id, mode))
	{
		add = true;
		flags |= BITDHT_FILTER_REASON_OWNID;
	}

	if (add)
	{
		bool isNew = addPeerToFilter(id, flags);
		if (isNew)
		{
			return 1;
		}
	}

	return 0;
}

int bdFilter::addPeerToFilter(const bdId *id, uint32_t flags)
{
	std::list<bdFilteredPeer>::iterator it;
	bool found = false;
	for(it = mFiltered.begin(); it != mFiltered.end(); it++)
	{
		if (id->addr.sin_addr.s_addr == it->mAddr.sin_addr.s_addr)
		{
			found = true;
			it->mLastSeen = time(NULL);
			it->mFilterFlags |= flags;
			break;
		}
	}

	if (!found)
	{
		time_t now = time(NULL);
		bdFilteredPeer fp;

		fp.mAddr = id->addr;
		fp.mAddr.sin_port = 0;
		fp.mFilterFlags = flags;
		fp.mFilterTS = now;	
		fp.mLastSeen = now;

		mFiltered.push_back(fp);
	}

	return found;
}



bool bdFilter::isOwnIdWithoutBitDhtFlags(const bdId *id, uint32_t peerFlags)
{
	if (peerFlags & BITDHT_PEER_STATUS_RECV_PONG)
	{
		if (peerFlags & BITDHT_PEER_STATUS_DHT_ENGINE)
		{
			/* okay! */
			return false;
		}

		/* now check distance */
		bdMetric dist;
		mFns->bdDistance(&mOwnId, &(id->id), &dist);
		int bucket = mFns->bdBucketDistance(&dist);

		/* if they match us... kill it */
		if (bucket == 0)
		{
			return true;
		}
	}
	return false;
}



