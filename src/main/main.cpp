#include "threadpool/threadpool.h"
void InitGlog(int argc, char ** argv) {
    FLAGS_log_dir = "/home/ubuntu/git/ThreadPool/build/log";
    google::InitGoogleLogging(argv[0]);
    google::SetLogDestination(google::GLOG_INFO, (FLAGS_log_dir + "/INFO_").data());
    google::SetStderrLogging(google::GLOG_INFO);
    google::SetLogFilenameExtension("log_");
    FLAGS_colorlogtostderr = true;  // Set log color
    FLAGS_logbufsecs = 0;  // Set log output speed(s)
    FLAGS_max_log_size = 1024;  // Set max log file size
    FLAGS_stop_logging_if_full_disk = true;  // If disk is full
}
void ShutDownGlog() {
    google::ShutdownGoogleLogging();
}
int main(int argc, char ** argv) {
    InitGlog(argc, argv);
    threadpool::ThreadPool pool(5);
    auto do_something = [](int a, int b) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return a + b;
    };
    for (int i = 0; i < 100; ++i) {
        pool.Submit(do_something, 10, 20);
    }
    std::this_thread::sleep_for(std::chrono::seconds(30));
    ShutDownGlog();
    return 0;
}