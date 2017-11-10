#include "xbase\x_target.h"
#include "xbase\x_debug.h"
#include "xbase\x_allocator.h"

#include "xmthread\x_namedmutex.h"
#include "xmthread\x_thread.h"
#include "xmthread\x_runnable.h"
#include "xtime\x_datetime.h"

#include "xunittest\xunittest.h"

using xcore::xnmutex;
using xcore::xthread;
using xcore::xrunnable;
using xcore::xdatetime;


static xnmutex testMutex("TestMutex");


namespace
{
	class TestLock: public xrunnable
	{
	public:
		void run()
		{
			testMutex.lock();
			_timestamp = xdatetime::sNow();
			testMutex.unlock();
		}

		const xdatetime& timestamp() const
		{
			return _timestamp;
		}

	private:
		xdatetime _timestamp;
	};

	class TestTryLock: public xrunnable
	{
	public:
		TestTryLock(): _locked(false)
		{
		}
		
		void run()
		{
			if (testMutex.tryLock())
			{
				_locked = true;
				testMutex.unlock();
			}
		}

		bool locked() const
		{
			return _locked;
		}

	private:
		bool _locked;
	};
}

UNITTEST_SUITE_BEGIN(xnmutex)
{
    UNITTEST_FIXTURE(main)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(testLock)
		{
			testMutex.lock();
			xthread thr;
			TestLock tl;
			thr.start(tl);
			xdatetime now = xdatetime::sNow();
			xthread::sleep(2000);
			testMutex.unlock();
			thr.join();
			CHECK_TRUE (tl.timestamp() > now);
		}


		UNITTEST_TEST(testTryLock)
		{
			xthread thr1;
			TestTryLock ttl1;
			thr1.start(ttl1);
			thr1.join();
			CHECK_TRUE (ttl1.locked());
	
			testMutex.lock();
			xthread thr2;
			TestTryLock ttl2;
			thr2.start(ttl2);
			thr2.join();
			testMutex.unlock();
			CHECK_TRUE (!ttl2.locked());
		}

	}
}
UNITTEST_SUITE_END