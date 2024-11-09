#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <random>

#include "Street.h"
#include "Intersection.h"
#include "Vehicle.h"

/* Implementation of class "WaitingVehicles" */

int WaitingVehicles::getSize()
{
    std::lock_guard<std::mutex> guard(_mutex);
    return _vehicles.size();
}

void WaitingVehicles::pushBack(std::shared_ptr<Vehicle> vehicle, std::promise<void> &&promise)
{
    std::lock_guard<std::mutex> guard(_mutex);
    _vehicles.push_back(vehicle);
    _promises.push_back(std::move(promise));
}

void WaitingVehicles::permitEntryToFirstInQueue()
{
    std::lock_guard<std::mutex> guard(_mutex);

    // Access the first items in both queues
    auto firstPromise = _promises.begin();
    auto firstVehicle = _vehicles.begin();

    // Signal that permission has been granted to enter
    firstPromise->set_value();

    // Remove the first elements from both queues
    _vehicles.erase(firstVehicle);
    _promises.erase(firstPromise);
}

/* Implementation of class "Intersection" */

Intersection::Intersection()
{
    _type = ObjectType::objectIntersection;
    _isBlocked = false;
}

void Intersection::addStreet(std::shared_ptr<Street> street)
{
    _streets.push_back(street);
}

std::vector<std::shared_ptr<Street>> Intersection::queryStreets(std::shared_ptr<Street> incoming)
{
    std::vector<std::shared_ptr<Street>> outgoingStreets;
    for (auto &street : _streets)
    {
        if (incoming->getID() != street->getID())
        {
            outgoingStreets.push_back(street);
        }
    }
    return outgoingStreets;
}

// Adds a vehicle to the queue and waits until it's allowed to enter
void Intersection::addVehicleToQueue(std::shared_ptr<Vehicle> vehicle)
{
    std::unique_lock<std::mutex> lock(_mtx);
    std::cout << "Intersection #" << _id << "::addVehicleToQueue: thread id = " << std::this_thread::get_id() << std::endl;
    lock.unlock();

    std::promise<void> permissionPromise;
    std::future<void> futurePermission = permissionPromise.get_future();
    _waitingVehicles.pushBack(vehicle, std::move(permissionPromise));

    // Wait until entry permission is granted
    futurePermission.wait();
    lock.lock();
    std::cout << "Intersection #" << _id << ": Vehicle #" << vehicle->getID() << " is granted entry." << std::endl;

    // Block execution until the traffic light is green
    if (_trafficLight.getCurrentPhase() == TrafficLightPhase::red) {
        _trafficLight.waitForGreen();
    }

    lock.unlock();
}

void Intersection::vehicleHasLeft(std::shared_ptr<Vehicle> vehicle)
{
    this->setIsBlocked(false);
}

void Intersection::setIsBlocked(bool isBlocked)
{
    _isBlocked = isBlocked;
}

// Starts the traffic light simulation and processes the vehicle queue in a new thread
void Intersection::simulate()
{
    _trafficLight.simulate();
    threads.emplace_back(std::thread(&Intersection::processVehicleQueue, this));
}

void Intersection::processVehicleQueue()
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // Process queue only if there's at least one vehicle and the intersection is not blocked
        if (_waitingVehicles.getSize() > 0 && !_isBlocked)
        {
            this->setIsBlocked(true);
            _waitingVehicles.permitEntryToFirstInQueue();
        }
    }
}

bool Intersection::trafficLightIsGreen()
{
    return (_trafficLight.getCurrentPhase() == TrafficLightPhase::green);
}
