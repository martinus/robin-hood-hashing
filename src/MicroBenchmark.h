#include <chrono>
#include <algorithm>
#include <vector>
#include <iostream>

class MicroBenchmark {
private:
    typedef std::chrono::high_resolution_clock clock;

public:
    MicroBenchmark(double maxTotalMeasuredSeconds = 1, uint64_t maxNumMeasurements = 1000, uint64_t measurementCertaintyFactor = 1000)
        : mItersLeft(0)
        , mNumIters(0) {

        // performs automatic calibration
        uint64_t bestMeasurementPrecisionNanos = -1;

        for (size_t i = 0; i < 100; ++i) {
            // find out how precise we can measure time
            std::chrono::time_point<clock> finishedTime;
            const auto startTime = clock::now();
            do {
                finishedTime = clock::now();
            } while (startTime == finishedTime);

            std::chrono::duration<uint64_t, std::nano> duration = finishedTime - startTime;
            bestMeasurementPrecisionNanos = std::min(bestMeasurementPrecisionNanos, duration.count());
        }

        // we want to be well above the best measurement precision for each measurement
        const uint64_t measurementTimeNanos = std::min(UINT64_C(1000000000), bestMeasurementPrecisionNanos * measurementCertaintyFactor);

        // convert maxTotalMeasuredSeconds to nanos
        const uint64_t maxTotalMeasuredNanos = static_cast<uint64_t>(maxTotalMeasuredSeconds * UINT64_C(1000000000));

        mMaxNumMeasurements = static_cast<size_t>(std::min(maxNumMeasurements, maxTotalMeasuredNanos / measurementTimeNanos));
        mDesiredSecondsPerMeasurement = measurementTimeNanos / 1e9;
        mMaxTotalMeasuredSeconds = maxTotalMeasuredSeconds;

        //std::cout << "minNanos=" << minNanos << " , measurementTimeNanos=" << measurementTimeNanos << std::endl; 
    }

    MicroBenchmark(double maxTotalMeasuredSeconds, double secondsPerMeasurement, size_t minNumMeasurements, size_t maxNumMeasurements)
        : mItersLeft(0)
        , mNumIters(0)
        , mStartTime()
        , mMeasurements()
        , mMaxNumMeasurements(maxNumMeasurements)
        , mDesiredSecondsPerMeasurement(secondsPerMeasurement)
        , mMaxTotalMeasuredSeconds(maxTotalMeasuredSeconds) {
    }

    inline bool keepRunning() {
        // i-- is *much* faster than --i, thanks to parallel execution
        // empty loop: 2.78998e-10 vs. 1.67208e-09, 6 times faster.
        if (mItersLeft--) {
            return true;
        }

        if (!measure(clock::now())) {
            mNumIters = 0;
            mItersLeft = 0;
            return false;
        }
            
        // prepare for next measurement
        mStartTime = clock::now();
        return true;
    }

    double min() const {
        double v = std::numeric_limits<double>::max();
        for (const auto& m : mMeasurements) {
            v = std::min(v, m.second / m.first);
        }
        return v;
    }

    const std::vector<std::pair<size_t, double>>& measurements() const {
        return mMeasurements;
    }

private:
    // true if we should continue measuring
    bool measure(std::chrono::time_point<clock> finishedTime) {
        if (0 == mNumIters) {
            // cleanup (potential) previous stuff
            mMeasurements.clear();
            mTotalMeasuredSeconds = 0;
            mNumIters = 1;
        } else {
            std::chrono::duration<double> duration = finishedTime - mStartTime;
            const auto actualMeasurementSeconds = duration.count();
            auto actualToDesired = actualMeasurementSeconds / mDesiredSecondsPerMeasurement;

            if (actualToDesired < 0.1) {
                // we are far off, need more than 10 times as many iterations.
                actualToDesired = 0.1;
            } else if (actualToDesired > 0.6) {
                // e.g. 0.7 would not work, potential endless loop
                mMeasurements.push_back(std::make_pair(mNumIters, actualMeasurementSeconds));
                mTotalMeasuredSeconds += actualMeasurementSeconds;
            }

            // update number of measurements.
            // +0.5 for correct rounding
            mNumIters = static_cast<size_t>(mNumIters / actualToDesired + 0.5);
            if (0 == mNumIters) {
                mNumIters = 1;
            }
        }
        mItersLeft = mNumIters - 1;

        // stop if any criteria is met
        return mTotalMeasuredSeconds < mMaxTotalMeasuredSeconds && mMeasurements.size() < mMaxNumMeasurements;
    }

    size_t mItersLeft;
    size_t mNumIters;
    std::chrono::time_point<clock> mStartTime;
    std::vector<std::pair<size_t, double>> mMeasurements;

    size_t mMaxNumMeasurements;
    double mDesiredSecondsPerMeasurement;
    double mMaxTotalMeasuredSeconds;
    double mTotalMeasuredSeconds;
};