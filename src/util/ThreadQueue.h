//
//
//

#include <chrono>
#include <deque>
#include <mutex>


#include "../util/Util.h"



namespace thread_queue {

template<typename T>
class ThreadQueueProcessor {
    std::mutex              mutex;
    std::condition_variable cv;
    std::deque<T>           queue;
    bool                    stopThread;
public:
    ThreadQueueProcessor() : stopThread(false) {}

    virtual void process(const T& data) = 0;

    void stop() {
        stopThread = true;
    }
    void push(const T& data) {
        std::unique_lock<std::mutex> lock(mutex);
        queue.push_front(data);
        cv.notify_one();
    }

    void run() {
        stopThread = false;
        queue.clear();

        {
            std::unique_lock<std::mutex> lock(mutex);
            for(;;) {
                if (stopThread) goto exit;
                while(queue.empty()) {
                    cv.wait_for(lock, Util::ONE_SECOND);
                    if (stopThread) goto exit;
                }
                while(!queue.empty()) {
                    if (stopThread) goto exit;
                    T data = queue.back();
                    process(data);
                    queue.pop_back();
                }
            }
        }
    exit:
        //
    }
};

template<typename T>
class ThreadQueueProducer {
    std::mutex                mutex;
    std::condition_variable   cv;
    std::deque<T>             queue;
    bool                      stopThread;
public:
    ThreadQueueProducer() : stopThread(false) {}

    // produce return true when data has value
    virtual bool produce(T& data, std::chrono::milliseconds duration) = 0;

    void stop() {
        stopThread = true;
    }

    bool empty() {
        std::unique_lock<std::mutex> lock(mutex);
        return queue.empty();
    }

    // pop return true when data has value
    bool pop(T& data, std::chrono::milliseconds duration = Util::ONE_SECOND) {
        std::unique_lock<std::mutex> lock(mutex);
        if (queue.empty()) {
            cv.wait_for(lock, duration);
        }
        bool hasData = false;
        if (!queue.empty()) {
            data = queue.back();
            queue.pop_back();
            hasData = true;
        }
        return hasData;
    }

    void run() {
        stopThread = false;
        queue.clear();

        for(;;) {
            if (stopThread) goto exit;
            T data;
            if (produce(data, Util::ONE_SECOND)) {
                queue.push_front(data);
                cv.notify_one();
            }
        }
    exit:
        //
    }

};

}