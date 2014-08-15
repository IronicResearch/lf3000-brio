#include <cxxtest/TestSuite.h>
#include <UnitTestUtils.h>

#include <Vision/VNVisionTypes.h>

#include <memory>

using namespace LeapFrog::Brio;
using namespace LF::Vision;

class TestPoint : public CxxTest::TestSuite, TestSuiteBase {
	public:
		void testConstructorWithNoArgumentShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNPoint> Point (new VNPoint);
			TS_ASSERT(Point.get());
			ASSERT_POINT_EQUALS(Point, 0, 0);
		}

		void testConstructorWithPointZeroXZeroYShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNPoint> Point (new VNPoint(0, 0));
			TS_ASSERT(Point.get());
			ASSERT_POINT_EQUALS(Point, 0, 0);
		}

		void testConstructorWithPointShortMinXShortMinYShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNPoint> Point (new VNPoint(SHRT_MIN, SHRT_MIN));
			TS_ASSERT(Point.get());
			ASSERT_POINT_EQUALS(Point, SHRT_MIN, SHRT_MIN);
		}

		void testConstructorWithPointShortMinXShortMaxYShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNPoint> Point (new VNPoint(SHRT_MIN, SHRT_MAX));
			TS_ASSERT(Point.get());
			ASSERT_POINT_EQUALS(Point, SHRT_MIN, SHRT_MAX);
		}

		void testConstructorWithPointShortMaxXShortMinYShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNPoint> Point (new VNPoint(SHRT_MAX, SHRT_MIN));
			TS_ASSERT(Point.get());
			ASSERT_POINT_EQUALS(Point, SHRT_MAX, SHRT_MIN);
		}

		void testConstructorWithPointShortMaxXShortMaxYShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNPoint> Point (new VNPoint(SHRT_MAX, SHRT_MAX));
			TS_ASSERT(Point.get());
			ASSERT_POINT_EQUALS(Point, SHRT_MAX, SHRT_MAX);
		}

		void testConstructorWithPointShortMaxPlusOneXShortMinMinusOneYShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNPoint> Point (new VNPoint((S16)(SHRT_MAX + 1), (S16)(SHRT_MIN - 1)));
			TS_ASSERT(Point.get());
			ASSERT_POINT_EQUALS(Point, SHRT_MIN, SHRT_MAX);
		}

	private:
		void ASSERT_POINT_EQUALS(std::auto_ptr<VNPoint>& point, S16 X, S16 Y) {
			TS_ASSERT_EQUALS(X, point->x);
			TS_ASSERT_EQUALS(Y, point->y);
		}

	private:

};
