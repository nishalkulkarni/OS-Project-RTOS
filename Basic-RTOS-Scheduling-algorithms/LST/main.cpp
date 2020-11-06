#include <iostream>
#include <deque>
#include <utility>
#include <string>
#include <cstdio>

using namespace std;

class Task
{

private:
    int deadline;
    int computeTime;

public:
    string name;
    Task(int deadline, int computeTime, string name) : deadline(deadline), computeTime(computeTime), name(move(name)) {}
    void execute(int currentTime);
    int getSlackTime(int currentTime) const;
    bool isOver() const;
};

void Task::execute(int currentTime)
{
    printf("Time: %d, Running Task %s\n", currentTime, name.c_str());
    computeTime--;
}

int Task::getSlackTime(int currentTime) const
{
    return (deadline - currentTime) - computeTime;
}

bool Task::isOver() const
{
    return computeTime == 0;
}

class Scheduler
{

private:
    int currentTime = 0;
    deque<Task> tasks;

public:
    explicit Scheduler(deque<Task> tasks) : tasks(move(tasks)) {}

    void run();
};

void Scheduler::run()
{
    while (!tasks.empty())
    {
        auto task = this->tasks.begin();
        auto lowestSlackTimeTask = task;
        bool possible = false;

        while (task != this->tasks.end())
        {
            if (task->isOver())
            {
                task = tasks.erase(task);
            }
            else
            {
                possible = true;
                int lowestSlackTime = lowestSlackTimeTask->getSlackTime(currentTime);
                int slackTime = task->getSlackTime(currentTime);

                if (lowestSlackTime > slackTime)
                    lowestSlackTimeTask = task;

                task++;
            }
        }

        if (possible)
        {
            lowestSlackTimeTask->execute(currentTime);
        }
        currentTime++;
    }
}

int main()
{
    deque<Task> tasks = {
        Task(7, 3, "T1"),
        Task(4, 2, "T2"),
        Task(8, 2, "T3")};
    Scheduler scheduler = Scheduler(tasks);
    scheduler.run();

    return 0;
}
