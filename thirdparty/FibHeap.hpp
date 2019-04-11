//
//  FibHeap.hpp
//
//  Created by Gabe Montague on 4/16/17.
//
//

#ifndef FibHeap_hpp
#define FibHeap_hpp

#include <iostream>

#include <exception>
#include <stdexcept>
#include <iostream>
#include <cassert>
#include <vector>
#include <complex>
#include <cstddef>

namespace FH {
    
    using std::logic_error;
    using std::cout;
    using std::vector;
    using std::log;
    
    static const float PHI = 1.6180339887498948482;
    
    template <typename KeyType, typename ValueType>
    class FibHeap {
    public:
        
        typedef std::size_t SizeType;
        
        // Define a type for a key-value pair
        struct KVPair {
            KeyType key;
            ValueType value;
        };
        
        FibHeap() : _n(0), _max(nullptr) {}
        
        // TODO: copying
        FibHeap(const FibHeap& that) = delete;
        FibHeap& operator=(const FibHeap&) = delete;
        
        // Gets the number of items
        SizeType getSize() const {
            return _n;
        }
        
        // Adds a new value to the heap
        void insert(const KeyType key, const ValueType value) {
            
            Node* created = new Node();
            created->key = key;
            created->value = value;
            
            if (_max == nullptr) {
                
                // Create a new root list
                _max = created;
                created->next = created;
                created->prev = created;
                
            } else {
                
                // Insert into an existing root list
                created->next = _max->next;
                created->prev = _max;
                _max->next->prev = created;
                _max->next = created;
                
                // Check if we should change the min
                if (created->key > _max->key) {
                    _max = created;
                }
            }
            
            // Increase the total number of elements
            _n += 1;
        }
        
        // Merges another Fibonacci heap to the current one, clearing the other heap
        void combineWith(FibHeap & other) {
            
            _n += other._n;
            
            // Combine the root linked lists
            Node* thisStart = _max;
            Node* thisEnd = _max->prev;
            Node* otherStart = other._max;
            Node* otherEnd = other._max->prev;
            
            thisEnd->next = otherStart;
            otherStart->prev = thisEnd;
            thisStart->prev = otherEnd;
            otherEnd->next = thisStart;
            
            // Update the minimum pointer
            if (_max == nullptr) {
                _max = other._max; // whether or not it is nullptr, other must be the min
            } else if (other._max != nullptr && other._max->key > _max->key) {
                _max = other._max;
            }
            
            // Destroy the other heap
            other._max = nullptr;
            other._n = 0;
        }
        
        // Peeks at the new value
        KVPair peekMax() const {
            return _max->getKV();
        }
        
        // Pops the value from the heap, removing and returning it
        KVPair popMax() {
            
            if (_max == nullptr) {
                throw logic_error("Attempted to pop item from empty heap");
            }
            
            const KVPair result = _max->getKV();
            const bool isLastElement = _max == _max->next && _max->child == nullptr;
            
            // Handle case of one element
            if (isLastElement) {
                delete _max;
                _max = nullptr;
            } else {
                
                // Add children of _max to the root list
                if (_max->child != nullptr) {
                    Node* firstChild = _max->child;
                    Node* lastChild = _max->child->prev;
                    Node* insertionLower = _max;
                    Node* insertionUpper = _max->next;
                    
                    insertionLower->next = firstChild;
                    insertionUpper->prev = lastChild;
                    firstChild->prev = insertionLower;
                    lastChild->next = insertionUpper;
                    
                    // Iterate over added children and set parents to null
                    firstChild->parent = nullptr;
                    Node* iteratedNode = firstChild->next;
                    while (iteratedNode != firstChild) {
                        iteratedNode->parent = nullptr;
                        iteratedNode = iteratedNode->next;
                    }
                }
                
                // Now remove the old min from the root list, and set the new min to the first child added (temporarily).
                _max->prev->next = _max->next;
                _max->next->prev = _max->prev;
                const Node* const toDelete = _max;
                _max = _max->next;
                delete toDelete;
                
                //cout << "\nPRECONSOLIDATE\n";
                //this->print();
                
                // Consolidate nodes
                _consolidate();
            }
            
            _n -= 1;
            
            return result;
        }
        
        // Run tests
        static void test();
        
        // Print
        void print() const {
            
            cout << "\nFibHeap of " << _n << " nodes:\n";
            _max->print();
            cout << "\n";
        }
        
        // Destructor
        virtual ~FibHeap() {
            if (_max != nullptr) {
                _max->deleteTree();
            }
        }
        
    private:
        
        // Number of elements
        SizeType _n;
        
        class Node {
        public:
            KeyType key;
            ValueType value;
            SizeType degree;
            
            Node* next;
            Node* prev;
            Node* parent;
            Node* child;
            
            Node() : degree(0), next(nullptr), prev(nullptr), parent(nullptr), child(nullptr) {}
            
            KVPair getKV() const {
                KVPair kv;
                kv.key = key;
                kv.value = value;
                return kv;
            }
            
            void print(const Node* start=nullptr) const {
                
                // Base case: no more siblings
                if (this == start) {
                    return;
                }
                
                // Print key and children
                cout << "(" << key << ",{";
                if (child != nullptr) {
                    child->print();
                }
                cout << "}),";
                
                // Print siblings
                next->print(start == nullptr ? this : start);
            }
            
            void deleteTree(const Node* start=nullptr) {
                
                // Base case: no more siblings - delete self
                if (this == start) {
                    delete this;
                    return;
                }
                
                // Delete children
                if (child != nullptr) {
                    child->deleteTree();
                }
                
                // Delete siblings
                next->deleteTree(start == nullptr ? this : start);
            }
        };
        
        // Pointer to the minimum element. Don't throw away the value or you leak the entire heap.
        Node* _max;
        
        void _consolidate() {
            
            // The non-inclusive upper bound of the degree of any tree
            const SizeType maxDegree = static_cast<SizeType>(floor(log(_n) / log(PHI))) + 1;
            
            // Allocate a vector with space maxDegree - all null.
            auto a = vector<Node*>(maxDegree, nullptr);
            
            // Iterate over the root list
            const Node* const last = _max->prev;
            Node* iteratedNode = _max;
            
            while (true) {
                
                Node* const iteratedNodeNext = iteratedNode->next;
                
                SizeType degree = iteratedNode->degree;
                
                // See if there is an existing tree we have already iterated over with matching degree
                Node* smaller = iteratedNode;
                while (a[degree] != nullptr) {
                    
                    // This will have the same degree as the iterated node
                    Node* otherNode = a[degree]; // Called y in notes
                    
                    const bool iteratedIsSmaller = smaller->key >= otherNode->key;
                    Node* const smallerSave = smaller;
                    smaller = iteratedIsSmaller ? smaller : otherNode;
                    Node* larger = iteratedIsSmaller ? otherNode : smallerSave;
                    
                    // Parent the smaller under the larger: first remove larger from the root list
                    larger->prev->next = larger->next;
                    larger->next->prev = larger->prev;
                    
                    // Now parent the larger under the smaller
                    larger->parent = smaller;
                    if (smaller->child == nullptr) {
                        larger->next = larger;
                        larger->prev = larger;
                        smaller->child = larger;
                    } else {
                        Node* sibling = smaller->child;
                        larger->next = sibling;
                        larger->prev = sibling->prev;
                        sibling->prev->next = larger;
                        sibling->prev = larger;
                        smaller->child = larger;
                    }
                    smaller->degree++;
                    
                    a[degree] = nullptr;
                    
                    // Now set the while loop to search for trees of the new degree
                    degree++;
                }
                
                // Record this tree in the array as having degree degree
                a[degree] = smaller;
                
                // Break out of the loop if we have exhausted nodes in our original root list
                if (iteratedNode == last) {
                    break;
                }
                
                iteratedNode = iteratedNodeNext;
            }
            
            _max = nullptr; // We are safe to do this because currently all nodes are stored in the degree array
            
            // Recalculate the min, and reform connections (necessary?)
            for (int i = 0; i < maxDegree; i++) {
                
                Node* node = a[i];
                
                // Skip null entries in the degree array
                if (node == nullptr) {
                    continue;
                }
                
                // Create a new root list from a single node
                if (_max == nullptr) {
                    node->parent = nullptr;
                    node->next = node;
                    node->prev = node;
                    _max = node;
                    
                    // ...or grow the new root list if it's been made
                } else {
                    
                    // Insert node
                    node->next = _max->next;
                    node->prev = _max;
                    _max->next->prev = node;
                    _max->next = node;
                    
                    // Update the min if necessary
                    if (node->key > _max->key) {
                        _max = node;
                    }
                }
            }
        }
    };
}

#endif /* FibHeap_hpp */