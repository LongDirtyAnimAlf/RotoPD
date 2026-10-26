#include "pdo.hpp"

#include "pdo.hpp"

// ---------------------------------------------------------------------------
// Free the object that starts at the given absolute byte position.
// Intermediate bytes belonging to that object are also cleared.
// Returns false if position is out of range or no object starts there.
// ---------------------------------------------------------------------------
bool PDO::clear(uint8_t position)
{
    if (position >= CO_PDO_MAX_SIZE) return false;
    if (objList[position] == nullptr) return false;   // nothing starts here

    uint8_t size = objList[position]->getTypeSize();
    if (position + size > CO_PDO_MAX_SIZE) size = CO_PDO_MAX_SIZE - position;

    for (uint8_t i = 0; i < size; ++i) {
        objList[position + i] = nullptr;
        data[position + i]    = 0;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Free the first occurrence of the given OD object.
// ---------------------------------------------------------------------------
bool PDO::clear(uint16_t index, uint8_t subIndex)
{
    int32_t objIndex = Object::findIndex(index, subIndex);
    if (objIndex < 0) return false;

    Object* target = &dictionary[objIndex];

    for (uint8_t pos = 0; pos < CO_PDO_MAX_SIZE; ++pos) {
        if (objList[pos] == target) {
            return clear(pos);          // re-use the absolute version
        }
    }
    return false;   // object not mapped in this PDO
}

// ---------------------------------------------------------------------------
// Remove every mapped object from this PDO.
// ---------------------------------------------------------------------------
void PDO::clearAll()
{
    for (uint8_t i = 0; i < CO_PDO_MAX_SIZE; ++i) {
        objList[i] = nullptr;
        data[i]    = 0;
    }
    numObjects = 0;
}

// ---------------------------------------------------------------------------
// Internal helper: find the first free contiguous region that can hold 'size'
// bytes. Returns the starting position, or 0xFF if no suitable slot exists.
// ---------------------------------------------------------------------------
static uint8_t findFreeSlot(Object* const* objList, uint8_t size)
{
    if (size == 0 || size > CO_PDO_MAX_SIZE) return 0xFF;

    for (uint8_t pos = 0; pos <= CO_PDO_MAX_SIZE - size; ++pos) {
        bool free = true;
        for (uint8_t i = 0; i < size; ++i) {
            if (objList[pos + i] != nullptr) {
                free = false;
                break;
            }
        }
        if (free) return pos;
    }
    return 0xFF;   // no free region large enough
}

// ---------------------------------------------------------------------------
// Map an already-resolved Object to the PDO at an absolute byte position.
// Returns false on bounds / overlap error.
// ---------------------------------------------------------------------------
bool PDO::set(Object &obj, uint8_t position)
{
    uint8_t size = obj.getTypeSize();

    if (position >= CO_PDO_MAX_SIZE)               return false;
    if (position + size > CO_PDO_MAX_SIZE)         return false;

    // Refuse to overwrite an existing mapping that is not the start of this object
    for (uint8_t i = 0; i < size; ++i) {
        if (objList[position + i] != nullptr && i != 0)
            return false;
        // Also refuse if we would overwrite the start of another object
        if (i == 0 && objList[position] != nullptr && objList[position] != &obj)
            return false;
    }

    objList[position] = &obj;
    for (uint8_t i = 0; i < size; ++i) {
        data[position + i] = obj.data[i];
        if (i != 0) objList[position + i] = nullptr;   // mark intermediate bytes
    }
    return true;
}

// ---------------------------------------------------------------------------
// Map an Object (looked up by index/sub-index) to an absolute byte position.
// ---------------------------------------------------------------------------
bool PDO::set(uint16_t index, uint8_t subIndex, uint8_t position)
{
    int32_t objIndex = Object::findIndex(index, subIndex);
    if (objIndex < 0) return false;

    return set(dictionary[objIndex], position);
}

// ---------------------------------------------------------------------------
// Map an already-resolved Object into the first free contiguous slot that
// can hold it.  Returns false if no free region of sufficient size exists.
// ---------------------------------------------------------------------------
bool PDO::set(Object &obj)
{
    uint8_t size = obj.getTypeSize();
    uint8_t pos  = findFreeSlot(objList, size);
    if (pos == 0xFF) return false;
    return set(obj, pos);
}

// ---------------------------------------------------------------------------
// Map an Object (looked up by index/sub-index) into the first free slot.
// ---------------------------------------------------------------------------
bool PDO::set(uint16_t index, uint8_t subIndex)
{
    int32_t objIndex = Object::findIndex(index, subIndex);
    if (objIndex < 0) return false;

    return set(dictionary[objIndex]);
}

// ---------------------------------------------------------------------------
// Refresh the PDO data buffer from the currently mapped Object Dictionary
// entries.
// ---------------------------------------------------------------------------
void PDO::updateData()
{
    for (uint8_t position = 0; position < CO_PDO_MAX_SIZE; ++position) {
        if (objList[position] != nullptr) {
            uint8_t size = objList[position]->getTypeSize();
            for (uint8_t i = 0; i < size; ++i) {
                data[position + i] = objList[position]->data[i];
            }
        }
    }
}
