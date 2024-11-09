#include <iostream>
#include <random>
#include "TrafficLight.h"

#include <stdio.h>      // For NULL
#include <stdlib.h>     // For srand, rand
#include <time.h>       // For time

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a: Using unique_lock and _condition.wait to wait for new messages
    // and pull them from the queue with move semantics.
    // The function should return the received object.
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait(lock, [this] { return !_queue.empty(); });
    T message = std::move(_queue.front());
    _queue.pop_front();
    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&message) // move semantics are used here to avoid copying
{
    // FP.4a: Lock the mutex with lock_guard, add the message to the queue,
    // and notify one waiting thread
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.clear();  // Clear queue to optimize performance
    _queue.emplace_back(std::move(message));
    _condition.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b: Implements an infinite loop that waits until the receive function
    // returns TrafficLightPhase::green, then exits.
    while (true)
    {
        if (_messages.receive() == TrafficLightPhase::green)
        {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b: Starts cycleThroughPhases in a thread when simulate is called,
    // using the thread queue inherited from the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// Executes in a separate thread to simulate traffic light cycling
void TrafficLight::cycleThroughPhases()
{
    // FP.2a: Implements an infinite loop that alternates the traffic light phase
    // between red and green, sends an update to the message queue, and
    // randomly chooses the cycle duration between 4 and 6 seconds.
    int cycleDuration = rand() % (6000 - 4000 + 1) + 4000;
    std::chrono::time_point<std::chrono::system_clock> lastUpdate = std::chrono::system_clock::now();

    while (true)
    {
        // Sleep to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // Calculate the time since the last phase update
        auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - lastUpdate).count();

        if (timeSinceLastUpdate >= cycleDuration)
        {
            // Reset cycle duration for the next iteration
            cycleDuration = rand() % (6000 - 4000 + 1) + 4000;

            // Toggle phase between red and green
            _currentPhase = (_currentPhase == TrafficLightPhase::red) ? TrafficLightPhase::green : TrafficLightPhase::red;

            // Update the message queue with the new phase
            _messages.send(std::move(_currentPhase));

            // Reset the last update time
            lastUpdate = std::chrono::system_clock::now();
        }
    }
}