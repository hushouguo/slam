/*
 * \file: StorageEntity.h
 * \brief: Created by hushouguo at 11:10:24 May 06 2019
 */
 
#ifndef __STORAGEENTITY_H__
#define __STORAGEENTITY_H__

BEGIN_NAMESPACE_SLAM {
	class StorageEntity : public Entry<u64, Entity*> {
		public:
			StorageEntity(Entity*);
			const char* getClassName() override { return "StorageEntity"; }

		public:
	};
}

#endif
