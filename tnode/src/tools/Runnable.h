/*
 * \file: Runnable.h
 * \brief: Created by hushouguo at 09:45:05 Apr 04 2019
 */
 
#ifndef __RUNNABLE_H__
#define __RUNNABLE_H__

BEGIN_NAMESPACE_TNODE {
	class Runnable : public Entry<u32>{
		public:
			Runnable(u32 id) : Entry<u32>(id) {}
			virtual void run() = 0;

		public:
			void schedule() {
				if (!this->_isrunning) {
					this->_isrunning = true;
					sThreadPool.add([this](Runnable* runnable) {
							System.cout("schedule: %s(%d) run", this->getClassName(), this->id);
							runnable->run();
							runnable->_isrunning = false;
							System.cout("schedule: %s(%d) run over", this->getClassName(), this->id);
							}, this);
				}
			}

		private:
			bool _isrunning = false;
	};
}

#endif
