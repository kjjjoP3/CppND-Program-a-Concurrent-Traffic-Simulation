#ifndef INTERSECTION_H
#define INTERSECTION_H

#include <vector>
#include <future>
#include <mutex>
#include <memory>
#include "TrafficObject.h"
#include "TrafficLight.h" // FP.6a: Include traffic light control

// Forward declarations to avoid circular dependencies
class Street;
class Vehicle;

// Helper class to manage the queue of vehicles waiting to enter an intersection, ensuring thread safety
class WaitingVehicles
{
public:
    // Retrieve the current queue size
    int getSize();

    // Add a vehicle and its entry permission promise to the queue
    void pushBack(std::shared_ptr<Vehicle> vehicle, std::promise<void> &&promise);

    // Allow the first vehicle in the queue to enter and fulfill its promise
    void permitEntryToFirstInQueue();

private:
    std::vector<std::shared_ptr<Vehicle>> _vehicles; // Stores waiting vehicles
    std::vector<std::promise<void>> _promises;       // Promises corresponding to each waiting vehicle
    std::mutex _mutex;                               // Protects access to the queue
};

// Class for managing an intersection, derived from TrafficObject
class Intersection : public TrafficObject
{
public:
    // Constructor
    Intersection();

    // Control intersection's blocked status
    void setIsBlocked(bool isBlocked);

    // Add a vehicle to the entry queue and manage its entry
    void addVehicleToQueue(std::shared_ptr<Vehicle> vehicle);

    // Connect a street to this intersection
    void addStreet(std::shared_ptr<Street> street);

    // Return a list of outgoing streets, excluding the incoming street
    std::vector<std::shared_ptr<Street>> queryStreets(std::shared_ptr<Street> incoming);

    // Start simulation of intersection behavior
    void simulate();

    // Mark that a vehicle has exited the intersection
    void vehicleHasLeft(std::shared_ptr<Vehicle> vehicle);

    // Check if the traffic light is currently green
    bool trafficLightIsGreen();

private:
    // Function to handle queue processing in a thread
    void processVehicleQueue();

    // Connected streets at this intersection
    std::vector<std::shared_ptr<Street>> _streets;

    // Queue of vehicles waiting to enter, managed with promises
    WaitingVehicles _waitingVehicles;

    // Flag indicating whether the intersection is currently blocked
    bool _isBlocked;

    // FP.6a: Traffic light control for the intersection
    TrafficLight _trafficLight;
};

#endif
