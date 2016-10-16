#include <chrono>
#include <algorithm>
#include <vector>

class MicroBenchmark {
public:
    MicroBenchmark(size_t numMeasurements = 4, double secondsPerMeasurement = 1.0)
        : mItersLeft(0)
        , mNumIters(0)
        , mNumMeasurements(numMeasurements)
        , mDesiredSecondsPerMeasurement(secondsPerMeasurement)
    {
        mMeasurements.reserve(mNumMeasurements);
    }

    inline bool keepRunning() {
        // i-- is *much* faster than --i, thanks to parallel execution
        // empty loop: 2.78998e-10 vs. 1.67208e-09, 6 times faster.
        if (mItersLeft--) {
            return true;
        }

        if (!measure(std::chrono::high_resolution_clock::now())) {
            mNumIters = 0;
            mItersLeft = 0;
            return false;
        }
            
        // prepare for next measurement
        mStartTime = std::chrono::high_resolution_clock::now();
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
    bool measure(std::chrono::time_point<std::chrono::high_resolution_clock> finishedTime) {
        if (0 == mNumIters) {
            // cleanup (potential) previous stuff
            mMeasurements.clear();
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
            }

            // update number of measurements.
            // +0.5 for correct rounding
            mNumIters = static_cast<size_t>(mNumIters / actualToDesired + 0.5);
            if (0 == mNumIters) {
                mNumIters = 1;
            }
        }
        mItersLeft = mNumIters - 1;
        return mMeasurements.size() < mNumMeasurements;
    }

    size_t mItersLeft;
    size_t mNumIters;
    std::chrono::time_point<std::chrono::high_resolution_clock> mStartTime;
    std::vector<std::pair<size_t, double>> mMeasurements;
    const size_t mNumMeasurements;
    const double mDesiredSecondsPerMeasurement;

};