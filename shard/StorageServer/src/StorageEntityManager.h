/*
 * \file: StorageEntityManager.h
 * \brief: Created by hushouguo at 11:11:49 May 06 2019
 */
 
#ifndef __STORAGEENTITYMANAGER_H__
#define __STORAGEENTITYMANAGER_H__

BEGIN_NAMESPACE_SLAM {
	class StorageEntityManager : public Manager<StorageEntity> {
	};
}

#define sStorageEntityManager slam::Singleton<slam::StorageEntityManager>::getInstance()

#endif
