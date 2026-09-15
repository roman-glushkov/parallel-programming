#include <charconv>
#include <cstring>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
using namespace std;
mutex coutMutex;

void Worker(int index)
{
    lock_guard lock(coutMutex);
    cout << "Flow " << index << " is doing its job.\n";
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        cerr << "Error: Usage <N>\n";
        return 1;
    }

    int threadCount = 0;
    const char* first = argv[1];
    const char* last  = argv[1] + strlen(argv[1]);

    auto [ptr, ec] = from_chars(first, last, threadCount);
    if (ec != errc{} || ptr != last || threadCount <= 0)
    {
        cerr << "Error: '" << argv[1] << "' is not a valid number of threads (integer > 0 required)\n";
        return 1;
    }

    vector<jthread> threads;
    threads.reserve(threadCount);

    for (int i = 1; i <= threadCount; ++i)
    {
        threads.emplace_back(Worker, i);
    }

    return 0;
}