#ifndef ENTITY_H
#define ENTITY_H

#include <vector>
#include <unordered_map>

// Entities are the basic objects in the game.
// While it would be perhaps faster to implement a
// more robust ECS architecture, we aren't exactly
// playing a high-speed game that necessarily
// requires it, yet (though that may change).
// We are going to bundle components into objects.
// They won't contain any of the logic, just data---
// that much will still be contained in systems---
// but we won't worry overly much about having that
// information accessed in a contiguous fashion.

class Component;

class Entity
{
private:
    // Basic Info
    int ID;
    int scene;
    std::string name;

public:
    // Components
    std::unordered_map<int, Component*> componentIDMap;
    std::vector<Component*> components;

    int         Get_ID();
    int         Get_Scene();
    std::string Get_Name();

    void        Set_ID(int newID);
    void        Set_Scene(int newScene);
    void        Set_Name(std::string newName);

    Entity(int ID, int scene, std::string name);
};


#endif
