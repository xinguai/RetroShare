/*
 * bitdht/bdconnection.cc
 *
 * BitDHT: An Flexible DHT library.
 *
 * Copyright 2011 by Robert Fernie
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

#include "bitdht/bdiface.h"

#include "bitdht/bdnode.h"
#include "bitdht/bdconnection.h"
#include "bitdht/bdmsgs.h"

#if 0
#include "bitdht/bencode.h"
#include "bitdht/bdmsgs.h"

#include "util/bdnet.h"

#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#endif


/************************************************************************************************************
******************************************** Message Interface **********************************************
************************************************************************************************************/

/* Outgoing Messages */
std::string getConnectMsgType(int msgtype)
{
	switch(msgtype)
	{
		case BITDHT_MSG_TYPE_CONNECT_REQUEST:
			return "ConnectRequest";
			break;
		case BITDHT_MSG_TYPE_CONNECT_REPLY:
			return "ConnectReply";
			break;
		case BITDHT_MSG_TYPE_CONNECT_START:
			return "ConnectStart";
			break;
		case BITDHT_MSG_TYPE_CONNECT_ACK:
			return "ConnectAck";
			break;
		default:
			return "ConnectUnknown";
			break;
	}
}

void bdNode::msgout_connect_genmsg(bdId *id, bdToken *transId, int msgtype, bdId *srcAddr, bdId *destAddr, int mode, int status)
{
#ifdef DEBUG_NODE_MSGOUT
	std::cerr << "bdNode::msgout_connect_genmsg() Type: " << getConnectMsgType(msgtype);
	std::cerr << " TransId: ";
	bdPrintTransId(std::cerr, transId);
	std::cerr << " From: ";
	mFns->bdPrintId(std::cerr, id);
	std::cerr << " SrcAddr: ";
	mFns->bdPrintId(std::cerr, srcAddr);
	std::cerr << " DestAddr: ";
	mFns->bdPrintId(std::cerr, destAddr);
	std::cerr << " Mode: " << mode;
	std::cerr << " Status: " << status;
	std::cerr << std::endl;
#endif

	registerOutgoingMsg(id, transId, msgtype);
	
        /* create string */
        char msg[10240];
        int avail = 10240;

        int blen = bitdht_connect_genmsg(transId, &(mOwnId), msgtype, srcAddr, destAddr, mode, status, msg, avail-1);
        sendPkt(msg, blen, id->addr);


}

void bdNode::msgin_connect_genmsg(bdId *id, bdToken *transId, int msgtype, 
					bdId *srcAddr, bdId *destAddr, int mode, int status)
{
	std::list<bdId>::iterator it;

#ifdef DEBUG_NODE_MSGS
	std::cerr << "bdNode::msgin_connect_genmsg() Type: " << getConnectMsgType(msgtype);
	std::cerr << " TransId: ";
	bdPrintTransId(std::cerr, transId);
	std::cerr << " From: ";
	mFns->bdPrintId(std::cerr, id);
	std::cerr << " SrcAddr: ";
	mFns->bdPrintId(std::cerr, srcAddr);
	std::cerr << " DestAddr: ";
	mFns->bdPrintId(std::cerr, destAddr);
	std::cerr << " Mode: " << mode;
	std::cerr << " Status: " << status;
	std::cerr << std::endl;
#else
	(void) transId;
#endif

	/* switch to actual work functions */
	uint32_t peerflags = 0;
	switch(msgtype)
	{
		case BITDHT_MSG_TYPE_CONNECT_REQUEST:
			peerflags = BITDHT_PEER_STATUS_RECV_CONNECT_MSG; 
			mCounterRecvConnectRequest++;

			recvedConnectionRequest(id, srcAddr, destAddr, mode);

			break;
		case BITDHT_MSG_TYPE_CONNECT_REPLY:
			peerflags = BITDHT_PEER_STATUS_RECV_CONNECT_MSG; 
			mCounterRecvConnectReply++;

			recvedConnectionReply(id, srcAddr, destAddr, mode, status);

			break;
		case BITDHT_MSG_TYPE_CONNECT_START:
			peerflags = BITDHT_PEER_STATUS_RECV_CONNECT_MSG; 
			mCounterRecvConnectStart++;

			recvedConnectionStart(id, srcAddr, destAddr, mode, status);

			break;
		case BITDHT_MSG_TYPE_CONNECT_ACK:
			peerflags = BITDHT_PEER_STATUS_RECV_CONNECT_MSG; 
			mCounterRecvConnectAck++;

			recvedConnectionAck(id, srcAddr, destAddr, mode);

			break;
		default:
			break;
	}

	/* received message - so peer must be good */
	addPeer(id, peerflags);

}


/************************************************************************************************************
****************************************** Connection Initiation ********************************************
************************************************************************************************************/


/* This is called to initialise a connection.
 * the callback could be with regard to:
 * a Direct EndPoint.
 * a Proxy Proxy, or an Proxy EndPoint.
 * a Relay Proxy, or an Relay EndPoint.
 *
 * We have two alternatives:
 *  1) Direct Endpoint.
 *  2) Using a Proxy.
 */
 
int bdNode::requestConnection(struct sockaddr_in *laddr, bdNodeId *target, uint32_t mode)
{
	/* check if connection obj already exists */
#ifdef DEBUG_NODE_CONNECTION
	std::cerr << "bdNode::requestConnection() Mode: " << mode;
	std::cerr << " Target: ";
	mFns->bdPrintNodeId(std::cerr, id);
	std::cerr << " Local NetAddress: ";
	mFns->bdPrintAddr(std::cerr, laddr);
	std::cerr << std::endl;
#endif

	if (mode == BITDHT_CONNECT_MODE_DIRECT)
	{
		return requestConnection_direct(laddr, target);
	}
	else
	{
		return requestConnection_proxy(laddr, target, mode);
	}
}

int bdNode::checkExistingConnectionAttempt(bdNodeId *target)
{
	std::map<bdNodeId, bdConnectionRequest>::iterator it;
	it = mConnectionRequests.find(*target);
	if (it != mConnectionRequests.end())
	{
#ifdef DEBUG_NODE_CONNECTION
		std::cerr << "bdNode::checkExistingConnectAttempt() Found Existing Connection!";
		std::cerr << std::endl;
#endif
		return 1;
	}
	return 0;
}

int bdNode::requestConnection_direct(struct sockaddr_in *laddr, bdNodeId *target)
{

#ifdef DEBUG_NODE_CONNECTION
	std::cerr << "bdNode::requestConnection_direct()";
	std::cerr << std::endl;
#endif
	/* create a bdConnect, and put into the queue */
	int mode = BITDHT_CONNECT_MODE_DIRECT;
	bdConnectionRequest connreq;
	
	if (checkExistingConnectionAttempt(target))
	{
		return 0;
	}

	connreq.setupDirectConnection(laddr, target);

	/* grab any peers from any existing query */
	std::list<bdQuery *>::iterator qit;
	for(qit = mLocalQueries.begin(); qit != mLocalQueries.end(); qit++)
	{
		if (!((*qit)->mId == (*target)))
		{
			continue;
		}

		/* matching query */
		/* find any potential proxies (must be same DHT type XXX TODO) */
		(*qit)->result(connreq.mPotentialProxies);		

		/* will only be one matching query.. so end loop */
		break;
	}

	/* now look in the bdSpace as well */

#ifdef DEBUG_NODE_CONNECTION
	std::cerr << "bdNode::requestConnection_direct() Init Connection State";
	std::cerr << std::endl;
	std::cerr << connreq;
	std::cerr << std::endl;
#endif

	/* push connect onto queue, for later completion */

	mConnectionRequests[*target] = connreq;

	/* connection continued via iterator */
	return 1;
}

 
int bdNode::requestConnection_proxy(struct sockaddr_in *laddr, bdNodeId *target, uint32_t mode)
{

#ifdef DEBUG_NODE_CONNECTION
	std::cerr << "bdNode::requestConnection_proxy()";
	std::cerr << std::endl;
#endif

	/* create a bdConnect, and put into the queue */

	bdConnectionRequest connreq;
	connreq.setupProxyConnection(laddr, target, mode);

	/* grab any peers from any existing query */
	std::list<bdQuery *>::iterator qit;
	for(qit = mLocalQueries.begin(); qit != mLocalQueries.end(); qit++)
	{
		if (!((*qit)->mId == (*target)))
		{
			continue;
		}

		/* matching query */
		/* find any potential proxies (must be same DHT type XXX TODO) */
		(*qit)->proxies(connreq.mPotentialProxies);		

		/* will only be one matching query.. so end loop */
		break;
	}

	/* find closest acceptable peers, 
	 * and trigger a search for target...
	 * this will hopefully find more suitable proxies.
	 */

	std::list<bdId> excluding;
	std::multimap<bdMetric, bdId> nearest;

#define CONNECT_NUM_PROXY_ATTEMPTS	10
	int number = CONNECT_NUM_PROXY_ATTEMPTS;

	mNodeSpace.find_nearest_nodes_with_flags(target, number, excluding, nearest, 
			BITDHT_PEER_STATUS_DHT_FOF       |
			BITDHT_PEER_STATUS_DHT_FRIEND);

	number = CONNECT_NUM_PROXY_ATTEMPTS - number;

	mNodeSpace.find_nearest_nodes_with_flags(target, number, excluding, nearest, 
			BITDHT_PEER_STATUS_DHT_APPL      |
			BITDHT_PEER_STATUS_DHT_VERSION);

	std::multimap<bdMetric, bdId>::iterator it;
	for(it = nearest.begin(); it != nearest.end(); it++)
	{
		bdNodeId midId;
		mFns->bdRandomMidId(target, &(it->second.id), &midId);
		/* trigger search */
		send_query(&(it->second), &midId);
	}

	/* push connect onto queue, for later completion */
	mConnectionRequests[*target] = connreq;
	
	return 1;
}


/** This function needs the Potential Proxies to be tested against DHT_APPL flags **/
void bdNode::addPotentialConnectionProxy(bdId *srcId, bdId *target)
{
	std::map<bdNodeId, bdConnectionRequest>::iterator it;
	for(it = mConnectionRequests.begin(); it != mConnectionRequests.end(); it++)
	{
		if (target->id == it->first)
		{
			it->second.addPotentialProxy(srcId);
		}
	}
}

void bdNode::iterateConnectionRequests()
{
	std::map<bdNodeId, bdConnectionRequest>::iterator it;
	for(it = mConnectionRequests.begin(); it != mConnectionRequests.end(); it++)
	{
		/* check status of connection */
		if (it->second.mState == BITDHT_CONNREQUEST_INIT)
		{
			/* kick off the connection if possible */
			//startConnectionAttempt(it->second);
		}
	}
}


/************************************************************************************************************
****************************************** Outgoing Triggers ************************************************
************************************************************************************************************/

/*** Called by iterator.
 * initiates the connection startup
 *
 * srcConnAddr must contain Own ID + Connection Port (DHT or TOU depending on Mode). 
 *
 * For a DIRECT Connection: proxyId == destination Id, and mode == DIRECT.
 * 
 * For RELAY | PROXY Connection: 
 *
 * In all cases, destConnAddr doesn't need to contain a valid address.
 */

int bdNode::startConnectionAttempt(bdId *proxyId, bdId *srcConnAddr, bdId *destConnAddr, int mode)
{
#ifdef DEBUG_NODE_CONNECTION
	std::cerr << "bdNode::startConnectionAttempt()";
	std::cerr << std::endl;
#endif

	/* Check for existing Connection */
	bdConnection *conn = findExistingConnectionBySender(proxyId, srcConnAddr, destConnAddr);
	if (conn)
	{
		/* ERROR */
#ifdef DEBUG_NODE_CONNECTION
		std::cerr << "bdNode::startConnectAttempt() ERROR EXISTING CONNECTION";
		std::cerr << std::endl;
#endif
		return 0;
	}

	/* INSTALL a NEW CONNECTION */
	// not offically playing by the rules, but it should work.
	conn = newConnectionBySender(proxyId, srcConnAddr, destConnAddr);

	conn->ConnectionSetup(proxyId, srcConnAddr, destConnAddr, mode);

	/* push off message */
	bdToken *transId;
	int msgtype =  BITDHT_MSG_TYPE_CONNECT_REQUEST;
	int status = BITDHT_CONNECT_ANSWER_OKAY;

	msgout_connect_genmsg(&(conn->mProxyId), transId, msgtype, &(conn->mSrcConnAddr), &(conn->mDestConnAddr), conn->mMode, status);

	return 1;
}

/* This will be called in response to a callback.
 * the callback could be with regard to:
 * a Direct EndPoint.
 * a Proxy Proxy, or an Proxy EndPoint.
 * a Relay Proxy, or an Relay EndPoint.
 *
 * If we are going to store the minimal amount in the bdNode about connections, 
 * then the parameters must contain all the information:
 *  
 * case 1:
 *
 */
 
void bdNode::AuthConnectionOk(bdId *srcId, bdId *proxyId, bdId *destId, int mode, int loc)
{

#ifdef DEBUG_NODE_CONNECTION
	std::cerr << "bdNode::AuthConnectionOk()";
	std::cerr << std::endl;
#endif

	/* Check for existing Connection */
	bdConnection *conn = findExistingConnection(&(srcId->id), &(proxyId->id), &(destId->id));
	if (!conn)
	{
		/* ERROR */
#ifdef DEBUG_NODE_CONNECTION
		std::cerr << "bdNode::AuthConnectionOk() ERROR NO EXISTING CONNECTION";
		std::cerr << std::endl;
#endif
		return;
	}

	/* we need to continue the connection */
	if (mode == BITDHT_CONNECT_MODE_DIRECT)
	{	
		if (conn->mState == BITDHT_CONNECTION_WAITING_AUTH)
		{	
			/* This pushes it into the START/ACK cycle, 
			 * which handles messages elsewhere
			 */
			conn->AuthoriseDirectConnection(srcId, proxyId, destId, mode, loc);
		}
		else
		{
			/* ERROR */

		}
		return;
	}

	if (loc == BD_PROXY_CONNECTION_END_POINT)
	{
		if (conn->mState == BITDHT_CONNECTION_WAITING_AUTH)
		{
			/*** XXX MUST RECEIVE THE ADDRESS FROM DEST for connection */
			conn->AuthoriseEndConnection(srcId, proxyId, destId, mode, loc);
			
			/* we respond to the proxy which will finalise connection */
			bdToken *transId;
			int msgtype = BITDHT_MSG_TYPE_CONNECT_REPLY;
			int status = BITDHT_CONNECT_ANSWER_OKAY;
			msgout_connect_genmsg(&(conn->mProxyId), transId, msgtype, &(conn->mSrcConnAddr), &(conn->mDestConnAddr), conn->mMode, status);
			
			return;
		}
		else
		{

			/* ERROR */
		}
	}

	if (conn->mState == BITDHT_CONNECTION_WAITING_AUTH)
	{
		/* otherwise we are the proxy (for either), pass on the request */
	
		/* XXX SEARCH for ID of peer */
	
		conn->AuthoriseProxyConnection(srcId, proxyId, destId, mode, loc);
	
		bdToken *transId;
		int msgtype = BITDHT_MSG_TYPE_CONNECT_REQUEST;
		int status = BITDHT_CONNECT_ANSWER_OKAY;
		msgout_connect_genmsg(&(conn->mDestId), transId, msgtype, &(conn->mSrcConnAddr), &(conn->mDestConnAddr), conn->mMode, status);
	}
	else
	{
		/* ERROR */
	}

	return;	
}
	
 
void bdNode::AuthConnectionNo(bdId *srcId, bdId *proxyId, bdId *destId, int mode, int loc)
{
	
#ifdef DEBUG_NODE_CONNECTION
	std::cerr << "bdNode::AuthConnectionNo()";
	std::cerr << std::endl;
#endif
	
	/* Check for existing Connection */
	bdConnection *conn = findExistingConnection(&(srcId->id), &(proxyId->id), &(destId->id));
	if (!conn)
	{
		/* ERROR */
#ifdef DEBUG_NODE_CONNECTION
		std::cerr << "bdNode::AuthConnectionNo() ERROR NO EXISTING CONNECTION";
		std::cerr << std::endl;
#endif
		return;
	}
	
	/* we need to continue the connection */
	
	if (mode == BITDHT_CONNECT_MODE_DIRECT)
	{
		/* we respond to the proxy which will finalise connection */
		bdToken *transId;
		int status = BITDHT_CONNECT_ANSWER_NOK;
		int msgtype = BITDHT_MSG_TYPE_CONNECT_REPLY;
		msgout_connect_genmsg(&(conn->mSrcId), transId, msgtype, 
							  &(conn->mSrcConnAddr), &(conn->mDestConnAddr), mode, status);
		
		cleanConnection(&(srcId->id), &(proxyId->id), &(destId->id));
		return;
	}

	if (loc == BD_PROXY_CONNECTION_END_POINT)
	{
		/* we respond to the proxy which will finalise connection */
		bdToken *transId;
		int status = BITDHT_CONNECT_ANSWER_NOK;
		int msgtype = BITDHT_MSG_TYPE_CONNECT_REPLY;
		msgout_connect_genmsg(&(conn->mProxyId), transId, msgtype, 
							  &(conn->mSrcConnAddr), &(conn->mDestConnAddr), mode, status);
		
		cleanConnection(&(srcId->id), &(proxyId->id), &(destId->id));

		return;
	}

	/* otherwise we are the proxy (for either), reply FAIL */
	bdToken *transId;
	int status = BITDHT_CONNECT_ANSWER_NOK;
	int msgtype = BITDHT_MSG_TYPE_CONNECT_REPLY;
	msgout_connect_genmsg(&(conn->mSrcId), transId, msgtype, 
						  &(conn->mSrcConnAddr), &(conn->mDestConnAddr), mode, status);

	cleanConnection(&(srcId->id), &(proxyId->id), &(destId->id));

	return;	
}


	


void bdNode::iterateConnections()
{
	std::map<bdProxyTuple, bdConnection>::iterator it;
	std::list<bdProxyTuple> eraseList;
	time_t now = time(NULL);
	
	for(it = mConnections.begin(); it != mConnections.end(); it++)
	{
		if (now - it->second.mLastEvent > BD_CONNECTION_MAX_TIMEOUT)
		{
			/* cleanup event */
			eraseList.push_back(it->first);
			continue;
		}

		if ((it->second.mState == BITDHT_CONNECTION_WAITING_ACK) &&
			(now - it->second.mLastStart > BD_CONNECTION_START_RETRY_PERIOD))
		{
			if (it->second.mRetryCount > BD_CONNECTION_START_MAX_RETRY)
			{
				/* connection failed! cleanup */
				callbackConnect(&(it->second.mSrcId),&(it->second.mProxyId),&(it->second.mDestId),
							it->second.mMode, it->second.mPoint, BITDHT_CONNECT_CB_FAILED);

				/* add to erase list */
				eraseList.push_back(it->first);
			}
			else
			{
				it->second.mLastStart = now;
				it->second.mRetryCount++;
				if (!it->second.mSrcAck)
				{
					bdToken *transId;
					int msgtype = BITDHT_MSG_TYPE_CONNECT_START;
					msgout_connect_genmsg(&(it->second.mSrcId), transId, msgtype, 
										  &(it->second.mSrcConnAddr), &(it->second.mDestConnAddr), 
										  it->second.mMode, it->second.mBandwidth);
				}
				if (!it->second.mDestAck)
				{
					bdToken *transId;
					int msgtype = BITDHT_MSG_TYPE_CONNECT_START;
					msgout_connect_genmsg(&(it->second.mDestId), transId, msgtype, 
										  &(it->second.mSrcConnAddr), &(it->second.mDestConnAddr), 
										  it->second.mMode, it->second.mBandwidth);
				}
			}
		}
	}

	/* clean up */
	while(eraseList.size() > 0)
	{
		bdProxyTuple tuple = eraseList.front();
		eraseList.pop_front();

		std::map<bdProxyTuple, bdConnection>::iterator eit = mConnections.find(tuple);
		mConnections.erase(eit);
	}
}




/************************************************************************************************************
****************************************** Callback Functions    ********************************************
************************************************************************************************************/


void bdNode::callbackConnect(bdId *srcId, bdId *proxyId, bdId *destId, int mode, int point, int cbtype)
{
	/* This is overloaded at a higher level */
}


/************************************************************************************************************
************************************** ProxyTuple + Connection State ****************************************
************************************************************************************************************/

int operator<(const bdProxyTuple &a, const bdProxyTuple &b)
{
	if (a.srcId < b.srcId)
	{
		return 1;
	}
	
	if (a.srcId == b.srcId)
	{
 		if (a.proxyId < b.proxyId)
		{
			return 1;
		}
 		else if (a.proxyId == b.proxyId)
		{
			if (a.destId < b.destId)
			{
				return 1;
			}
		}
	}
	return 0;
}

int operator==(const bdProxyTuple &a, const bdProxyTuple &b)
{
	if ((a.srcId == b.srcId) && (a.proxyId == b.proxyId) && (a.destId == b.destId))
	{
		return 1;
	}
	return 0;
}

bdConnection::bdConnection()
{
	///* Connection State, and TimeStamp of Update */
	//int mState;
	//time_t mLastEvent;
	//
	///* Addresses of Start/Proxy/End Nodes */
	//bdId mSrcId;
	//bdId mDestId;
	//bdId mProxyId;
	//
	///* Where we are in the connection,
	//* and what connection mode.
	//*/
	//int mPoint;
	//int mMode;
	//
	///* must have ip:ports of connection ends (if proxied) */
	//bdId mSrcConnAddr;
	//bdId mDestConnAddr;
	//
	//int mBandwidth;
	//
	///* START/ACK Finishing ****/
	//time_t mLastStart;   /* timer for retries */
	//int mRetryCount;     /* retry counter */
	//
	//bool mSrcAck;
	//bool mDestAck;
	//
	//// Completion TS.
	//time_t mCompletedTS;
}


bdConnection *bdNode::findExistingConnection(bdNodeId *srcId, bdNodeId *proxyId, bdNodeId *destId)
{
	bdProxyTuple tuple(srcId, proxyId, destId);
	std::map<bdProxyTuple, bdConnection>::iterator it = mConnections.find(tuple);
	if (it == mConnections.end())
	{
		return NULL;
	}

	return &(it->second);
}

bdConnection *bdNode::newConnection(bdNodeId *srcId, bdNodeId *proxyId, bdNodeId *destId)
{
	bdProxyTuple tuple(srcId, proxyId, destId);
	bdConnection conn;

	mConnections[tuple] = conn;
	std::map<bdProxyTuple, bdConnection>::iterator it = mConnections.find(tuple);
	if (it == mConnections.end())
	{
		return NULL;
	}
	return &(it->second);
}

int bdNode::cleanConnection(bdNodeId *srcId, bdNodeId *proxyId, bdNodeId *destId)
{
	bdProxyTuple tuple(srcId, proxyId, destId);
	bdConnection conn;

	std::map<bdProxyTuple, bdConnection>::iterator it = mConnections.find(tuple);
	if (it == mConnections.end())
	{
		return 0;
	}
	mConnections.erase(it);

	return 1;
}


int bdNode::determinePosition(bdNodeId *sender, bdNodeId *src, bdNodeId *dest)
{
	int pos =  BD_PROXY_CONNECTION_UNKNOWN_POINT;
	if (mOwnId == *src)
	{
		pos = BD_PROXY_CONNECTION_START_POINT;
	}
	else if (mOwnId == *dest)
	{
		pos = BD_PROXY_CONNECTION_END_POINT;
	}
	else
	{
		pos = BD_PROXY_CONNECTION_MID_POINT;
	}
	return pos;	
}

int bdNode::determineProxyId(bdNodeId *sender, bdNodeId *src, bdNodeId *dest, bdNodeId *proxyId)
{
	int pos = determinePosition(sender, src, dest);
	switch(pos)
	{
		case BD_PROXY_CONNECTION_START_POINT:
		case BD_PROXY_CONNECTION_END_POINT:
			*proxyId = *sender;
			return 1;
			break;
		default:
		case BD_PROXY_CONNECTION_MID_POINT:
			*proxyId = mOwnId;
			return 1;
			break;
	}
	return 0;
}



bdConnection *bdNode::findExistingConnectionBySender(bdId *sender, bdId *src, bdId *dest)
{
	bdNodeId proxyId;
	bdNodeId *senderId = &(sender->id);
	bdNodeId *srcId = &(src->id);
	bdNodeId *destId = &(dest->id);
	determineProxyId(senderId, srcId, destId, &proxyId);

	return findExistingConnection(srcId, &proxyId, destId);
}

bdConnection *bdNode::newConnectionBySender(bdId *sender, bdId *src, bdId *dest)
{
	bdNodeId proxyId;
	bdNodeId *senderId = &(sender->id);
	bdNodeId *srcId = &(src->id);
	bdNodeId *destId = &(dest->id);
	determineProxyId(senderId, srcId, destId, &proxyId);

	return newConnection(srcId, &proxyId, destId);
}


int bdNode::cleanConnectionBySender(bdId *sender, bdId *src, bdId *dest)
{
	bdNodeId proxyId;
	bdNodeId *senderId = &(sender->id);
	bdNodeId *srcId = &(src->id);
	bdNodeId *destId = &(dest->id);
	determineProxyId(senderId, srcId, destId, &proxyId);

	return cleanConnection(srcId, &proxyId, destId);
}


/************************************************************************************************************
****************************************** Received Connect Msgs ********************************************
************************************************************************************************************/


/* This function is triggered by a CONNECT_REQUEST message.
 * it will occur on both the Proxy/Dest in the case of a Proxy (PROXY | RELAY) and on the Dest (DIRECT) nodes.
 *
 * In all cases, we store the request and ask for authentication.
 *
 */



int bdNode::recvedConnectionRequest(bdId *id, bdId *srcConnAddr, bdId *destConnAddr, int mode)
{
#ifdef DEBUG_NODE_CONNECTION
	std::cerr << "bdNode::recvedConnectionRequest()";
	std::cerr << std::endl;
#endif
	/* Check for existing Connection */
	bdConnection *conn = findExistingConnectionBySender(id, srcConnAddr, destConnAddr);
	if (conn)
	{
		/* ERROR */
#ifdef DEBUG_NODE_CONNECTION
		std::cerr << "bdNode::recvedConnectionRequest() ERROR EXISTING CONNECTION";
		std::cerr << std::endl;
#endif
		return 0;
	}

	/* INSTALL a NEW CONNECTION */
	conn = bdNode::newConnectionBySender(id, srcConnAddr, destConnAddr);

	int point = 0;
	if (mode == BITDHT_CONNECT_MODE_DIRECT)
	{
#ifdef DEBUG_NODE_CONNECTION
		std::cerr << "bdNode::recvedConnectionRequest() Installing DIRECT CONNECTION";
		std::cerr << std::endl;
#endif

		/* we are actually the end node, store stuff, get auth and on with it! */
		point = BD_PROXY_CONNECTION_END_POINT;

		conn->ConnectionRequestDirect(id, srcConnAddr, destConnAddr);

		callbackConnect(&(conn->mSrcId),&(conn->mProxyId),&(conn->mDestId),
							conn->mMode, conn->mPoint, BITDHT_CONNECT_CB_AUTH);
	}
	else
	{
		/* check if we are proxy, or end point */
		bool areProxy = (srcConnAddr->id == id->id);
		if (areProxy)
		{
			point = BD_PROXY_CONNECTION_MID_POINT;

			conn->ConnectionRequestProxy(id, srcConnAddr, destConnAddr, mode);

			callbackConnect(&(conn->mSrcId),&(conn->mProxyId),&(conn->mDestId),
							conn->mMode, conn->mPoint, BITDHT_CONNECT_CB_AUTH);
		}
		else
		{
			point = BD_PROXY_CONNECTION_END_POINT;

			conn->ConnectionRequestEnd(id, srcConnAddr, destConnAddr, mode);

			callbackConnect(&(conn->mSrcId),&(conn->mProxyId),&(conn->mDestId),
							conn->mMode, conn->mPoint, BITDHT_CONNECT_CB_AUTH);
		}
	}
	return 1;
}


/* This function is triggered by a CONNECT_REPLY message.
 * it will occur on either the Proxy or Source. And indicates YES / NO to the connection, 
 * as well as supplying address info to the proxy.
 *
 */

int bdNode::recvedConnectionReply(bdId *id, bdId *srcConnAddr, bdId *destConnAddr, int mode, int status)
{
	/* retrieve existing connection data */
	bdConnection *conn = findExistingConnectionBySender(id, srcConnAddr, destConnAddr);
	if (!conn)
	{
		/* ERROR */
#ifdef DEBUG_NODE_CONNECTION
		std::cerr << "bdNode::recvedConnectionReply() ERROR NO EXISTING CONNECTION";
		std::cerr << std::endl;
#endif
		return 0;
	}

	switch(conn->mPoint)
	{
		case BD_PROXY_CONNECTION_START_POINT:
		case BD_PROXY_CONNECTION_END_POINT:		/* NEVER EXPECT THIS */
		case BD_PROXY_CONNECTION_UNKNOWN_POINT:		/* NEVER EXPECT THIS */
		default:					/* NEVER EXPECT THIS */
		{

			/* Only situation we expect this, is if the connection is not allowed.
			 * DEST has sent back an ERROR Message
			 */

			if ((status == BITDHT_CONNECT_ANSWER_NOK) && (conn->mPoint == BD_PROXY_CONNECTION_START_POINT))
			{
				/* connection is killed */

			}
			else
			{
				/* ERROR in protocol */
			}

			/* do Callback for Failed Connection */
			callbackConnect(&(conn->mSrcId),&(conn->mProxyId),&(conn->mDestId),
							conn->mMode, conn->mPoint, BITDHT_CONNECT_CB_FAILED);

			/* Kill Connection always */
			cleanConnectionBySender(id, srcConnAddr, destConnAddr);

			return 0;
		}
			break;

		case BD_PROXY_CONNECTION_MID_POINT:
		{
			 /*    We are proxy. and OK / NOK for connection proceed.
			  */

			if ((status == BITDHT_CONNECT_ANSWER_OKAY) && (conn->mState == BITDHT_CONNECTION_WAITING_REPLY))
			{
				/* OK, continue connection! */

				/* Upgrade Connection to Finishing Mode */
				conn->upgradeProxyConnectionToFinish(id, srcConnAddr, destConnAddr, mode, status);

				/* do Callback for Pending Connection */
				callbackConnect(&(conn->mSrcId),&(conn->mProxyId),&(conn->mDestId),
								conn->mMode, conn->mPoint, BITDHT_CONNECT_CB_PENDING);
			}
			else
			{
				/* do Callback for Failed Connection */
				callbackConnect(&(conn->mSrcId),&(conn->mProxyId),&(conn->mDestId),
								conn->mMode, conn->mPoint, BITDHT_CONNECT_CB_FAILED);

				/* send on message to SRC */
				bdToken *transId;
				int msgtype = BITDHT_MSG_TYPE_CONNECT_REPLY;
				msgout_connect_genmsg(&(conn->mSrcId), transId, msgtype, &(conn->mSrcConnAddr), &(conn->mDestConnAddr), mode, status);

				/* connection is killed */
				cleanConnectionBySender(id, srcConnAddr, destConnAddr);
			}
			return 0;
		}
			break;
	}
	return 0;
}


/* This function is triggered by a CONNECT_START message.
 * it will occur on both the Src/Dest in the case of a Proxy (PROXY | RELAY) and on the Src (DIRECT) nodes.
 *
 * parameters are checked against pending connections.
 *  Acks are set, and connections completed if possible (including callback!).
 */

int bdNode::recvedConnectionStart(bdId *id, bdId *srcConnAddr, bdId *destConnAddr, int mode, int bandwidth)
{
	/* retrieve existing connection data */
	bdConnection *conn = findExistingConnectionBySender(id, srcConnAddr, destConnAddr);
	if (!conn)
	{
		/* ERROR */
#ifdef DEBUG_NODE_CONNECTION
		std::cerr << "bdNode::recvedConnectionStart() ERROR NO EXISTING CONNECTION";
		std::cerr << std::endl;
#endif
		return 0;
	}


	if (conn->mPoint == BD_PROXY_CONNECTION_MID_POINT)
	{
		/* ERROR */
	}

	/* check state */
	if ((conn->mState != BITDHT_CONNECTION_WAITING_START) || (conn->mState != BITDHT_CONNECTION_COMPLETED))
	{
		/* ERROR */

		return 0;
	}

	/* ALL Okay, Send ACK */

	bdToken *transId;
	int msgtype = BITDHT_MSG_TYPE_CONNECT_ACK;
	int status = BITDHT_CONNECT_ANSWER_OKAY;
	msgout_connect_genmsg(id, transId, msgtype, &(conn->mSrcId), &(conn->mDestId), mode, status);

	/* do complete Callback */

	/* flag as completed */
	if (conn->mState != BITDHT_CONNECTION_COMPLETED)
	{
		/* Store Final Addresses */
		time_t now = time(NULL);

		conn->mSrcConnAddr = *srcConnAddr;
		conn->mDestConnAddr = *destConnAddr;
		conn->mState = BITDHT_CONNECTION_COMPLETED;
		conn->mCompletedTS = now;

		if (conn->mPoint == BD_PROXY_CONNECTION_START_POINT)
		{
			/* callback to connect to Dest address! */
			// Slightly different callback, use ConnAddr for start message!
			callbackConnect(&(conn->mSrcId),&(conn->mProxyId),&(conn->mDestConnAddr),
							conn->mMode, conn->mPoint, BITDHT_CONNECT_CB_START);

		}
		else
		{
			/* callback to connect to Src address! */
			// Slightly different callback, use ConnAddr for start message!
			callbackConnect(&(conn->mSrcConnAddr),&(conn->mProxyId),&(conn->mDestId),
							conn->mMode, conn->mPoint, BITDHT_CONNECT_CB_START);

		}
	}
	/* don't delete, if ACK is lost, we want to be able to re-respond */

	return 1;
}


/* This function is triggered by a CONNECT_ACK message.
 * it will occur on both the Proxy (PROXY | RELAY) and on the Dest (DIRECT) nodes.
 *
 * parameters are checked against pending connections.
 *  Acks are set, and connections completed if possible (including callback!).
 */

int bdNode::recvedConnectionAck(bdId *id, bdId *srcConnAddr, bdId *destConnAddr, int mode)
{
	/* retrieve existing connection data */
	bdConnection *conn = findExistingConnectionBySender(id, srcConnAddr, destConnAddr);
	if (!conn)
	{
		/* ERROR */
#ifdef DEBUG_NODE_CONNECTION
		std::cerr << "bdNode::recvedConnectionStart() ERROR NO EXISTING CONNECTION";
		std::cerr << std::endl;
#endif
		return 0;
	}

	if (conn->mPoint == BD_PROXY_CONNECTION_START_POINT)
	{
		/* ERROR */
	}

	/* check state */
	if (conn->mState != BITDHT_CONNECTION_WAITING_ACK)
	{
		/* ERROR */

		return 0;
	}

	if (id->id == srcConnAddr->id)
	{
		/* recved Ack from source */
		conn->mSrcAck = true;
	}
	else if (id->id == destConnAddr->id)
	{
		/* recved Ack from dest */
		conn->mDestAck = true;
	}

	if (conn->mSrcAck && conn->mDestAck)
	{
		/* connection complete! cleanup */
		if (conn->mMode == BITDHT_CONNECT_MODE_DIRECT)
		{
			int mode = conn->mMode | BITDHT_CONNECT_ANSWER_OKAY;
			/* callback to connect to Src address! */
			// Slightly different callback, use ConnAddr for start message!
			callbackConnect(&(conn->mSrcConnAddr),&(conn->mProxyId),&(conn->mDestId),
							conn->mMode, conn->mPoint, BITDHT_CONNECT_CB_START);

		}
		else
		{
			int mode = conn->mMode | BITDHT_CONNECT_ANSWER_OKAY;
			callbackConnect(&(conn->mSrcId),&(conn->mProxyId),&(conn->mDestId),
							conn->mMode, conn->mPoint, BITDHT_CONNECT_CB_PROXY);

		}

		/* Finished Connection! */
		cleanConnectionBySender(id, srcConnAddr, destConnAddr);
	}
	return 1;
}



/************************************************************************************************************
********************************* Connection / ConnectionRequest Functions **********************************
************************************************************************************************************/

// Initialise a new Connection (request by User)

// Any Connection initialised at Source (START_POINT), prior to Auth.
int bdConnection::ConnectionSetup(bdId *proxyId, bdId *srcConnAddr, bdId *destConnAddr, int mode)
{
	mState = BITDHT_CONNECTION_WAITING_START; /* or REPLY, no AUTH required */
	mLastEvent = time(NULL);
	mSrcId = *srcConnAddr;    /* self, IP unknown */
	mDestId = *destConnAddr;  /* dest, IP unknown */
	mProxyId =  *proxyId;  /* full proxy/dest address */

	mPoint = BD_PROXY_CONNECTION_START_POINT;
	mMode = mode;

	mSrcConnAddr = *srcConnAddr; /* self */
	mDestConnAddr = *destConnAddr; /* IP unknown */

	/* don't bother with START/ACK parameters */
	
	return 1;
}



// Initialise a new Connection. (receiving a Connection Request)
// Direct Connection initialised at Destination (END_POINT), prior to Auth.
int bdConnection::ConnectionRequestDirect(bdId *id, bdId *srcConnAddr, bdId *destConnAddr)
{
	mState = BITDHT_CONNECTION_WAITING_AUTH;
	mLastEvent = time(NULL);
	mSrcId = *id;
	mDestId = *destConnAddr;  /* self, IP unknown */
	mProxyId = *id;  /* src */

	mPoint = BD_PROXY_CONNECTION_END_POINT;
	mMode = BITDHT_CONNECT_MODE_DIRECT;

	mSrcConnAddr = *srcConnAddr;
	mDestConnAddr = *destConnAddr; /* self */

	/* don't bother with START/ACK parameters */
	
	return 1;
}


// Proxy Connection initialised at Proxy (MID_POINT), prior to Auth.
int bdConnection::ConnectionRequestProxy(bdId *id, bdId *srcConnAddr, bdId *destConnAddr, int mode)
{
	mState = BITDHT_CONNECTION_WAITING_AUTH;
	mLastEvent = time(NULL);
	mSrcId = *id;
	mDestId = *destConnAddr;  /* self, IP unknown */
	//mProxyId =  *id;  /* own id, doesn't matter */

	mPoint = BD_PROXY_CONNECTION_MID_POINT;
	mMode = mode;

	mSrcConnAddr = *srcConnAddr;
	mDestConnAddr = *destConnAddr; /* other peer, IP unknown */

	/* don't bother with START/ACK parameters */
	
	return 1;
}


// Proxy Connection initialised at Destination (END_POINT), prior to Auth.
int bdConnection::ConnectionRequestEnd(bdId *id, bdId *srcConnAddr, bdId *destConnAddr, int mode)
{
	mState = BITDHT_CONNECTION_WAITING_AUTH;
	mLastEvent = time(NULL);
	mSrcId = *srcConnAddr;   /* src IP unknown */
	mDestId = *destConnAddr;  /* self, IP unknown */
	mProxyId = *id;  /* src */

	mPoint = BD_PROXY_CONNECTION_END_POINT;
	mMode = mode;

	mSrcConnAddr = *srcConnAddr;
	mDestConnAddr = *destConnAddr; /* self */

	/* don't bother with START/ACK parameters */
	
	return 1;
}

// Received AUTH, step up to next stage.
int bdConnection::AuthoriseProxyConnection(bdId *srcId, bdId *proxyId, bdId *destId, int mode, int loc)
{
	mState = BITDHT_CONNECTION_WAITING_REPLY;
	mLastEvent = time(NULL);

	//mSrcId, (peer) should be okay.
	//mDestId (other peer) install now (just received)
	mDestId = *destId;
	//mProxyId (self) doesn't matter.

	// mPoint, mMode should be okay.

	// mSrcConnAddr should be okay.
	// mDestConnAddr is still pending.

	/* don't bother with START/ACK parameters */
	
	return 1;
}


int bdConnection::AuthoriseEndConnection(bdId *srcId, bdId *proxyId, bdId *destConnAddr, int mode, int loc)
{
	mState = BITDHT_CONNECTION_WAITING_START;
	mLastEvent = time(NULL);

	//mSrcId, (peer) should be okay.
	//mDestId (self) doesn't matter.
	//mProxyId (peer) should be okay.

	// mPoint, mMode should be okay.

	// mSrcConnAddr should be okay.
	// Install the correct destConnAddr. (just received)
	mDestConnAddr = *destConnAddr; 

	// Initialise the START/ACK Parameters.
	mRetryCount = 0;
	mLastStart = 0;
	mSrcAck = false;
	mDestAck = true; // Automatic ACK, as it is from us.
	mCompletedTS = 0;
	
	return 1;
}




// Auth of the Direct Connection, means we move straight to WAITING_ACK mode.
int bdConnection::AuthoriseDirectConnection(bdId *srcId, bdId *proxyId, bdId *destConnAddr, int mode, int loc)
{
	mState = BITDHT_CONNECTION_WAITING_ACK;
	mLastEvent = time(NULL);

	//mSrcId, (peer) should be okay.
	//mDestId (self) doesn't matter.
	//mProxyId (peer) should be okay.

	// mPoint, mMode should be okay.

	// mSrcConnAddr should be okay.
	// Install the correct destConnAddr. (just received)
	mDestConnAddr = *destConnAddr; 

	// Initialise the START/ACK Parameters.
	mRetryCount = 0;
	mLastStart = 0;
	mSrcAck = false;
	mDestAck = true; // Automatic ACK, as it is from us.
	mCompletedTS = 0;
	
	return 1;
}

// Proxy Connection => at Proxy, Ready to send out Start and get back ACKs!!
int bdConnection::upgradeProxyConnectionToFinish(bdId *id, bdId *srcConnAddr, bdId *destConnAddr, int mode, int status)
{
	mState = BITDHT_CONNECTION_WAITING_ACK;
	mLastEvent = time(NULL);

	//mSrcId,mDestId should be okay.
	//mProxyId, not set, doesn't matter.

	// mPoint, mMode should be okay.

	// mSrcConnAddr should be okay.
	// Install the correct destConnAddr. (just received)
	mDestConnAddr = *destConnAddr; 

	// Initialise the START/ACK Parameters.
	mRetryCount = 0;
	mLastStart = 0;
	mSrcAck = false;
	mDestAck = false;
	mCompletedTS = 0;

	return 1;
}





int bdConnectionRequest::setupDirectConnection(struct sockaddr_in *laddr, bdNodeId *target)
{
	return 0;
}

int bdConnectionRequest::setupProxyConnection(struct sockaddr_in *laddr, bdNodeId *target, uint32_t mode)
{
	return 0;
}

int bdConnectionRequest::addPotentialProxy(bdId *srcId)
{
	return 0;
}




