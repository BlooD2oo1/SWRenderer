#include <vector>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include "flat_hash_map.hpp"

// Requires TObject to have a default constructor and an accessible uint32_t/TID m_iID member variable.
template <typename TObject, typename TID = uint32_t>
class DenseMap
{
public:
    DenseMap()
        : m_iIDCounter(0)
    {
    }

    // Adds a default-constructed object and returns a reference to it.
    // Sets the generated ID directly into obj.m_iID.
    // Optionally outputs the assigned ID via pOutID.
    TObject& Add(TID* pOutID = NULL)
    {
        TID id = ++m_iIDCounter;

        m_aObjects.push_back(TObject());
        TObject& obj = m_aObjects.back();
        obj.m_iID = id;

        size_t index = m_aObjects.size() - 1;
        m_mObjects[id] = index;

        if (pOutID != NULL)
        {
            *pOutID = id;
        }

        return obj;
    }

    // Removes an object by ID using swap-with-last (O(1))
    bool Delete(TID ID)
    {
        typename ska::flat_hash_map<TID, size_t>::iterator it = m_mObjects.find(ID);
        if (it == m_mObjects.end())
        {
            return false;
        }

        size_t index_to_remove = it->second;
        size_t last_index = m_aObjects.size() - 1;

        if (index_to_remove != last_index)
        {
            // Move the last object to the erased slot
            m_aObjects[index_to_remove] = m_aObjects[last_index];

            // Update hash map index for the relocated object
            m_mObjects[m_aObjects[index_to_remove].m_iID] = index_to_remove;
        }

        m_aObjects.pop_back();
        m_mObjects.erase(it);
        return true;
    }

    // Retrieve object reference by ID (asserts that the ID exists)
    TObject& GetByID(TID ID)
    {
        typename ska::flat_hash_map<TID, size_t>::iterator it = m_mObjects.find(ID);
        assert(it != m_mObjects.end() && "ID does not exist in DenseMap!");
        return m_aObjects[it->second];
    }

    const TObject& GetByID(TID ID) const
    {
        typename ska::flat_hash_map<TID, size_t>::const_iterator it = m_mObjects.find(ID);
        assert(it != m_mObjects.end() && "ID does not exist in DenseMap!");
        return m_aObjects[it->second];
    }

    // Check whether an ID exists in the map
    bool Contains(TID ID) const
    {
        return m_mObjects.find(ID) != m_mObjects.end();
    }

    // Direct object access by index for iteration
    TObject& GetByInd(size_t iInd) { return m_aObjects[iInd]; }
    const TObject& GetByInd(size_t iInd) const { return m_aObjects[iInd]; }

    // Retrieve the ID associated with an array index
    TID GetIDByInd(size_t iInd) const { return m_aObjects[iInd].m_iID; }

    size_t GetCount() const { return m_aObjects.size(); }

    void Clear()
    {
        m_aObjects.clear();
        m_mObjects.clear();
    }

    void Reserve(size_t iCapacity)
    {
        m_aObjects.reserve(iCapacity);
        m_mObjects.reserve(iCapacity);
    }

private:
    std::vector<TObject> m_aObjects;
    ska::flat_hash_map<TID, size_t> m_mObjects;
    TID m_iIDCounter;
};