/*
 * libretroshare/src/ft: ftserver.cc
 *
 * File Transfer for RetroShare.
 *
 * Copyright 2008 by Robert Fernie.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License Version 2 as published by the Free Software Foundation.
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
 * Please report all bugs and problems to "retroshare@lunamutt.com".
 *
 */

#include "util/rsdebug.h"
const int ftserverzone = 29539;

#include "ft/ftserver.h"
#include "ft/ftextralist.h"
#include "ft/ftfilesearch.h"
#include "ft/ftcontroller.h"
#include "ft/ftdatamultiplex.h"

#include "dbase/cachestrapper.h"
#include "dbase/fimonitor.h"
#include "dbase/fistore.h"

#include "pqi/pqi.h"
#include "pqi/p3connmgr.h"

#include <iostream>
#include <sstream>

	/* Setup */
ftServer::ftServer(CacheStrapper *cStrapper, p3ConnectMgr *connMgr)
	:mCacheStrapper(cStrapper), mConnMgr(connMgr)
{


}

void	ftServer::setConfigDirectory(std::string path)
{
	mConfigPath = path;
}

void	ftServer::setPQInterface(PQInterface *pqi)
{
	
}

	/* Control Interface */

	/* add Config Items (Extra, Controller) */
void	ftServer::addConfigComponents(p3ConfigMgr *mgr)
{
	/* NOT SURE ABOUT THIS ONE */
}

	/* Final Setup (once everything is assigned) */
void ftServer::SetupFtServer()
{
	/* make Controller */
	mFtController = new ftController(config_dir);
	NotifyBase *cb = getNotify();

	/* setup FiStore/Monitor */
	std::string localcachedir = config_dir + "/cache/local";
	std::string remotecachedir = config_dir + "/cache/remote";
	std::string ownId = mConnMgr->getOwnId();

	mFiStore = new FileIndexStore(mCacheStrapper, mFtController, cb, ownId, remotecachedir);
        mFiMon = new FileIndexMonitor(mCacheStrapper, localcachedir, ownId);

	/* now add the set to the cachestrapper */
	CachePair cp(mFiMon, mFiStore, CacheId(RS_SERVICE_TYPE_FILE_INDEX, 0));
	mCacheStrapper -> addCachePair(cp);

	/* extras List */
	mFtExtra = new ftExtraList();

	mFtSearch = new ftFileSearch();
	//mFtSearch->addSearchMode(mCacheStrapper, RS_FILE_HINTS_CACHE);
	mFtSearch->addSearchMode(mFtExtra, RS_FILE_HINTS_EXTRA);
	//mFtSearch->addSearchMode(mFiMon, RS_FILE_HINTS_LOCAL);
	//mFtSearch->addSearchMode(mFiStore, RS_FILE_HINTS_REMOTE);

	mFtController -> setFtSearch(mFtSearch);
	mFtController -> setSaveBasePath(save_dir);

        mFtDataplex = ftDataMultiplex(this, mFtSearch);
	
	return;
}


void    ftServer::StartupThreads()
{
	/* start up order - important for dependencies */

	/* self contained threads */
	/* startup ExtraList Thread */
	mFtExtra->start();

	/* startup Monitor Thread */
	/* startup the FileMonitor (after cache load) */
	mFiMon->setPeriod(600); /* 10 minutes */
	/* start it up */
	//mFiMon->setSharedDirectories(dbase_dirs);
	mFiMon->start();

	/* start own thread */
	//start();

	/* Controller thread */
	mFtController->start();
}

CacheStrapper *ftServer::getCacheStrapper()
{
	return mCacheStrapper;
}

CacheTransfer *ftServer::getCacheTransfer()
{
	return mFtController;
}


	/***************************************************************/
	/********************** RsFiles Interface **********************/
	/***************************************************************/


	/***************************************************************/
	/********************** Controller Access **********************/
	/***************************************************************/

bool ftServer::FileRequest(std::string fname, std::string hash, 
				uint32_t size, std::string dest, uint32_t flags)
{
	return mFtController->FileRequest(fname, hash, size, dest, flags);
}

bool ftServer::FileCancel(std::string hash)
{
	return mFtController->FileCancel(hash);
}

bool ftServer::FileControl(std::string hash, uint32_t flags)
{
	return mFtController->FileControl(hash, flags);
}

bool ftServer::FileClearCompleted()
{
	return mFtController->FileClearCompleted();
}

	/* get Details of File Transfers */
bool ftServer::FileDownloads(std::list<std::string> &hashs)
{
	return mFtController->FileDownloads(hashs);
}

	/* Directory Handling */
void ftServer::setDownloadDirectory(std::string path)
{
	mFtController->setDownloadDirectory(path);
}

std::string ftServer::getDownloadDirectory()
{
	return mFtController->getDownloadDirectory();
}

void ftServer::setPartialsDirectory(std::string path)
{
	mFtController->setPartialsDirectory(path);
}

std::string ftServer::getPartialsDirectory()
{
	return mFtController->getPartialsDirectory();
}


	/***************************************************************/
	/************************* Other Access ************************/
	/***************************************************************/

bool ftServer::FileUploads(std::list<std::string> &hashs)
{
	return mFtDataplex->FileUploads(hashes);
}

bool ftServer::FileDetails(std::string hash, uint32_t hintflags, FileInfo &info)
{
	bool found = false;
	if (hintflags | RS_FILE_HINTS_DOWNLOAD)
	{
		found = mFtController->FileDetails(hash, info);
	}
	else if (hintflags | RS_FILE_HINTS_UPLOAD)
	{
		found = mFtDataplex->FileDetails(hash, info);
	}

	if (!found)
	{
		mFtSearch->FileDetails(hash, hintflags, info);
	}
	return found;
}

	/***************************************************************/
	/******************* ExtraFileList Access **********************/
	/***************************************************************/

bool  ftServer::ExtraFileAdd(std::string fname, std::string hash, uint32_t size, 
						uint32_t period, uint32_t flags)
{
	return mFtExtra->addExtraFile(fname, hash, size, period, flags);
}

bool ftServer::ExtraFileRemove(std::string hash, uint32_t flags)
{
	return mFtExtra->removeExtraFile(hash, flags);
}

bool ftServer::ExtraFileHash(std::string localpath, uint32_t period, uint32_t flags)
{
	return mFtExtra->hashExtraFile(localpath, period, flags);
}

bool ftServer::ExtraFileStatus(std::string localpath, FileInfo &info)
{
	return mFtExtra->hashExtraFileDone(localpath, info);
}

	/***************************************************************/
	/******************** Directory Listing ************************/
	/***************************************************************/

int ftServer::RequestDirDetails(std::string uid, std::string path, DirDetails &details)
{
	return mFiStore->RequestDirDetails(uid, path, details);
}
	
int ftServer::RequestDirDetails(void *ref, DirDetails &details, uint32_t flags)
{
	return mFiStore->RequestDirDetails(ref, details, flags);
}
	
	/***************************************************************/
	/******************** Search Interface *************************/
	/***************************************************************/


int ftServer::SearchKeywords(std::list<std::string> keywords, std::list<FileDetail> &results)
{
	return mFiStore->SearchKeywords(keywords, results);
}
	
int ftServer::SearchBoolExp(Expression * exp, std::list<FileDetail> &results)
{
	return mFiStore->searchBoolExp(exp, results);
}

	
	/***************************************************************/
	/*************** Local Shared Dir Interface ********************/
	/***************************************************************/
	
bool    ftServer::ConvertSharedFilePath(std::string path, std::string &fullpath)
{
	return mFiMon->convertSharedFilePath(path, fullpath);
}
	
void    ftServer::ForceDirectoryCheck()
{
	mFiMon->forceDirectoryCheck();
	return;
}
	
bool    ftServer::InDirectoryCheck()
{
	return mFiMon->inDirectoryCheck();
}
	
bool	ftServer::getSharedDirectories(std::list<std::string> &dirs)
{
	return mFiMon->getSharedDirectories(dirs);
}

bool 	ftServer::addSharedDirectory(std::string dir)
{
	return mFiMon->addSharedDirectory(dir);
}

bool 	ftServer::removeSharedDirectory(std::string dir)
{
	return mFiMon->removeSharedDirectory(dir);
}


	/***************************************************************/
	/****************** End of RsFiles Interface *******************/
	/***************************************************************/


	/***************************************************************/
	/**************** Config Interface *****************************/
	/***************************************************************/

	protected:
        /* Key Functions to be overloaded for Full Configuration */
RsSerialiser *ftServer::setupSerialiser()
{
	return NULL;
}

std::list<RsItem *> ftServer::saveList(bool &cleanup)
{
	std::list<RsItem *> list;
	return list;
}

bool    ftServer::loadList(std::list<RsItem *> load)
{
	return true;
}

bool  ftServer::loadConfigMap(std::map<std::string, std::string> &configMap)
{
	return true;
}


	/***************************************************************/
	/**********************     Data Flow     **********************/
	/***************************************************************/

	/* Client Send */
bool	ftServer::sendDataRequest(std::string peerId, std::string hash, 
			uint64_t size, uint64_t offset, uint32_t chunksize)
{
	/* create a packet */
	/* push to networking part */
	RsFileRequest *rfi = new RsFileRequest();

	/* id */
	rfi->PeerId(peerId);

	/* file info */
	rfi->file.filesize   = size;
	rfi->file.hash       = hash; /* ftr->hash; */

	/* offsets */
	rfi->fileoffset = offset; /* ftr->offset; */
	rfi->chunksize  = chunksize; /* ftr->chunk; */

	mP3iface->SendFileRequest(rfi);
}

const uint32_t	MAX_FT_CHUNK  = 32 * 1024; /* 32K */

	/* Server Send */
bool	ftServer::sendData(std::string peerId, std::string hash, uint64_t size,
			uint64_t baseoffset, uint32_t chunksize, void *data)
{
	/* create a packet */
	/* push to networking part */
	uint32_t tosend = chunksize;
	uint64_t offset = 0;
	uint32_t chunk;


	while(tosend > 0)
	{
		/* workout size */
		chunk = MAX_FT_CHUNK;
		if (chunk > tosend)
		{
			chunk = tosend;
		}

		/******** New Serialiser Type *******/

		RsFileData *rfd = new RsFileData();

		/* set id */
		rfd->PeerId(peerId);

		/* file info */
		rfd->fd.file.filesize = size;
		rfd->fd.file.hash     = hash;
		rfd->fd.file.name     = ""; /* blank other data */
		rfd->fd.file.path     = "";
		rfd->fd.file.pop      = 0;
		rfd->fd.file.age      = 0;

		rfd->fd.file_offset = baseoffset + offset;

		/* file data */
		rfd->fd.binData.setBinData(
			&(((uint8_t *) data)[offset]), chunk);

		mP3iface->SendFileData(rfd);

		offset += chunk;
		tosend -= chunk;
	}

	/* clean up data */
	free(data);
}


int	ftServer::tick()
{
	rslog(RSL_DEBUG_BASIC, ftserverzone, 
		"filedexserver::tick()");

	/* the new Cache Hack() */
	FileStoreTick();

	if (mP3iface == NULL)
	{
		std::ostringstream out;
		rslog(RSL_DEBUG_BASIC, ftserverzone, 
			"filedexserver::tick() Invalid Interface()");

		return 1;
	}

	int moreToTick = 0;

	if (0 < mP3iface -> tick())
	{
		moreToTick = 1;
#ifdef DEBUG_TICK
		std::cerr << "filedexserver::tick() moreToTick from mP3iface" << std::endl;
#endif
	}

	if (0 < handleInputQueues())
	{
		moreToTick = 1;
#ifdef DEBUG_TICK
		std::cerr << "filedexserver::tick() moreToTick from InputQueues" << std::endl;
#endif
	}
	return moreToTick;
}


// This function needs to be divided up.
bool    ftServer::handleInputQueues()
{
	handleCacheData();
	handleFileData();
}

bool     ftServer::handleCacheData()
{
	// get all the incoming results.. and print to the screen.
	RsCacheRequest *cr;
	RsCacheItem    *ci;

	// Loop through Search Results.
	int i = 0;
	int i_init = 0;

	//std::cerr << "filedexserver::handleInputQueues()" << std::endl;
	while((ci = mP3iface -> GetSearchResult()) != NULL)
	{
		//std::cerr << "filedexserver::handleInputQueues() Recvd SearchResult (CacheResponse!)" << std::endl;
		std::ostringstream out;
		if (i++ == i_init)
		{
			out << "Recieved Search Results:" << std::endl;
		}
		ci -> print(out);
		rslog(RSL_DEBUG_BASIC, ftserverzone, out.str());

		/* these go to the CacheStrapper! */
		CacheData data;
		data.cid = CacheId(ci->cacheType, ci->cacheSubId);
		data.hash = ci->file.hash;
		data.size = ci->file.filesize;
		data.name = ci->file.name;
		data.path = ci->file.path;
		data.pid = ci->PeerId();
		data.pname = mAuthMgr->getName(ci->PeerId());
		mCacheStrapper->recvCacheResponse(data, time(NULL));

		delete ci;
	}

	// now requested Searches.
	i_init = i;
	while((cr = mP3iface -> RequestedSearch()) != NULL)
	{
		/* just delete these */
		std::ostringstream out;
		out << "Requested Search:" << std::endl;
		cr -> print(out);
		rslog(RSL_DEBUG_BASIC, ftserverzone, out.str());
		delete cr;
	}


	// Now handle it replacement (pushed cache results) 
	{
		std::list<std::pair<RsPeerId, CacheData> > cacheUpdates;
		std::list<std::pair<RsPeerId, CacheData> >::iterator it;

		mCacheStrapper->getCacheUpdates(cacheUpdates);
		for(it = cacheUpdates.begin(); it != cacheUpdates.end(); it++)
		{
			/* construct reply */
			RsCacheItem *ci = new RsCacheItem();
	
			/* id from incoming */
			ci -> PeerId(it->first);

			ci -> file.hash = (it->second).hash;
			ci -> file.name = (it->second).name;
			ci -> file.path = ""; // (it->second).path;
			ci -> file.filesize = (it->second).size;
			ci -> cacheType  = (it->second).cid.type;
			ci -> cacheSubId =  (it->second).cid.subid;

#ifdef SERVER_DEBUG 
			std::ostringstream out2;
			out2 << "Outgoing CacheStrapper Update -> RsCacheItem:" << std::endl;
			ci -> print(out2);
			std::cerr << out2.str() << std::endl;
#endif

			//rslog(RSL_DEBUG_BASIC, ftserverzone, out2.str());
			mP3iface -> SendSearchResult(ci);
		}
	}
}


bool    ftServer::handleFileData()
{
	// now File Input.
	RsFileRequest *fr;
	RsFileData *fd;

	int i_init = 0;
	int i = 0;

	i_init = i;
	while((fr = mP3iface -> GetFileRequest()) != NULL )
	{
#ifdef SERVER_DEBUG 
		std::cerr << "filedexserver::handleInputQueues() Recvd ftFiler Request" << std::endl;
		std::ostringstream out;
		if (i == i_init)
		{
			out << "Incoming(Net) File Item:" << std::endl;
		}
		fr -> print(out);
		rslog(RSL_DEBUG_BASIC, ftserverzone, out.str());
#endif

		i++; /* count */
		mFtDataplex->recvDataRequest(fr->PeerId(), 
				fr->file.hash,  fr->file.filesize, 
				fr->fileoffset, fr->chunksize);

FileInfo(ffr);
		delete fr;

	}

	// now File Data.
	i_init = i;
	while((fd = mP3iface -> GetFileData()) != NULL )
	{
#ifdef SERVER_DEBUG 
		//std::cerr << "filedexserver::handleInputQueues() Recvd ftFiler Data" << std::endl;
		std::ostringstream out;
		if (i == i_init)
		{
			out << "Incoming(Net) File Data:" << std::endl;
		}
		fd -> print(out);
		rslog(RSL_DEBUG_BASIC, ftserverzone, out.str());
#endif
		i++; /* count */

		/* incoming data */
		mFtDataplex->recvData(fd->PeerId(),
				 fd->fd.file.hash,  fd->fd.file.filesize, 
				fd->fd.file_offset, 
				fd->fd.binData.bin_len, 
				fd->fd.binData.bin_data);

		/* we've stolen the data part -> so blank before delete 
		 */
		fd->fd.TlvShallowClear();
		delete fd;
	}

	if (i > 0)
	{
		return 1;
	}
	return 0;
}

/**********************************
 **********************************
 **********************************
 *********************************/


