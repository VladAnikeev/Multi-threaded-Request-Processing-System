#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <random>

// Инициализируем генератор случайных чисел
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis(500, 5000); // Диапазон от 0,5 до 5 секунд

// хранит время работы
class Request
{
};

// максимум может вернуть 10 раз указатель на Request, а больше nullptr
Request* GetRequest() throw()
{
    const auto max_count = 9;

    static int count = 0;
    if (count > max_count)
        return nullptr;
    ++count;

    // имитация работы
    std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
    return new Request();
}

void ProcessRequest(Request* request) throw()
{
    // имитация работы
    std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
}




const int NumberOfThreads = 2;

std::queue<Request*> requestQueue;
std::mutex queueMutex;
std::condition_variable queueCondVar;
std::atomic<bool> shouldStop(false); // команда на завершение работы

void workerThread()
{
    // std::cout - не лучшие решение вывода лога, возможно комбинирование строк
    std::cout << "start#" << std::this_thread::get_id() << std::endl;
    while (true)
    {
        Request* request;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            std::cout << "block#" << std::this_thread::get_id() << std::endl;
            queueCondVar.wait(lock, []
                { return !requestQueue.empty() || shouldStop.load(); });

            std::cout << "unblock#" << std::this_thread::get_id() << std::endl;


            if (shouldStop.load())
            {
                std::cout << "end#" << std::this_thread::get_id() << std::endl;
                return;
            }
            request = requestQueue.front();
            requestQueue.pop();

        }
        std::cout << "job#" << std::this_thread::get_id() << std::endl;
        ProcessRequest(request);
        delete request;
    }
}

int main()
{
    // Создание потов
    std::vector<std::thread> workers;
    for (int i = 0; i < NumberOfThreads; ++i)
    {
        workers.emplace_back(workerThread);
    }

    // Загрузка задания для потоков
    while (true)
    {
        Request* request = GetRequest();
        if (request == nullptr)
        {
            break;
        }
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            requestQueue.push(request);
        }
        queueCondVar.notify_one();
    }

    // Завершение работы потоков и их ожидание 
    std::cout << "main close thread" << std::endl;
    shouldStop.store(true);
    queueCondVar.notify_all();
    for (auto& worker : workers)
    {
        worker.join();
    }

    return 0;
}
