#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include "TrafficObject.h"

// Define an enum representing the traffic light phases as either "red" or "green"
enum TrafficLightPhase { red, green };

// Forward declaration to avoid dependency cycle
class Vehicle;

// FP.3: Template class "MessageQueue" providing public methods for sending and receiving messages.
// It includes a deque to store TrafficLightPhase messages, alongside a condition variable and a mutex.
template <class T>
class MessageQueue
{
public:
    // Adds a message to the queue using rvalue reference
    void send(T&& message); 
    // Retrieves and removes a message from the queue
    T receive();

private:
    std::deque<T> _queue;               // Queue to store messages
    std::condition_variable _condition; // Condition variable for message handling
    std::mutex _mutex;                  // Mutex to ensure thread-safe access
};

// FP.1: Define the "TrafficLight" class, derived from TrafficObject. 
// This class provides public methods to wait for the green light, simulate traffic light behavior, 
// and get the current phase. Private method "cycleThroughPhases" controls light phase changes.
class TrafficLight : public TrafficObject
{
public:
    // Constructor
    TrafficLight();

    // Method to get the current phase of the traffic light
    TrafficLightPhase getCurrentPhase();

    // Methods to manage traffic light operations
    void waitForGreen();
    void simulate();

private:
    // Method to cycle through traffic light phases in a continuous loop
    void cycleThroughPhases();
    TrafficLightPhase _currentPhase;  // Tracks the current traffic light phase

    // FP.4b: MessageQueue to store TrafficLightPhase messages; 
    // used in the phase cycle loop to send phase updates.
    MessageQueue<TrafficLightPhase> _messages;
};

#endif
