#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Nef_3/SNC_indexed_items.h>
#include <CGAL/Nef_3/ID_support_handler.h>
#include <CGAL/Nef_S2/Sphere_circle.h>
#include <iostream>

typedef CGAL::Exact_predicates_exact_constructions_kernel K;
typedef CGAL::Sphere_circle<K> Sphere_circle;

// Mock-up classes to satisfy the Decorator requirements in ID_support_handler
struct MockSHalfedge;
struct MockSVertex;
struct MockHalffacet;

struct MockSHalfedge {
    int id;
    MockSHalfedge* twin_ptr;
    Sphere_circle circle_obj;
    int index;
    int index2;
    
    MockSHalfedge(int i) : id(i), twin_ptr(nullptr), circle_obj(1,0,0), index(0), index2(0) {}
    
    MockSHalfedge* twin() { return twin_ptr; }
    const MockSHalfedge* twin() const { return twin_ptr; }
    
    Sphere_circle circle() const { return circle_obj; }
    
    int get_index() const { return index; }
    void set_index(int i) { index = i; }
};

struct MockSVertex {
    int index;
    int get_index() const { return index; }
    void set_index(int i) { index = i; }
};

struct MockHalffacet {
    // Not needed for the specific SHalfedge overload we are testing
};

struct MockDecorator {
    typedef MockSVertex* SVertex_handle;
    typedef MockSHalfedge* SHalfedge_handle;
    typedef const MockSVertex* SVertex_const_handle;
    typedef const MockSHalfedge* SHalfedge_const_handle;
    typedef void* SHalfloop_const_handle;
    typedef void* Halffacet_const_handle;
};

int main() {
    CGAL::ID_support_handler<CGAL::SNC_indexed_items, MockDecorator> handler;

    // Create mock SHalfedges
    MockSHalfedge se(1), se1(2), se1t(3), se2(4), se2t(5);
    MockSHalfedge set(10);
    se.twin_ptr = &set; // se's twin, we don't care much about it
    se1.twin_ptr = &se1t;
    se1t.twin_ptr = &se1;
    se2.twin_ptr = &se2t;
    se2t.twin_ptr = &se2;

    // We want to trigger line 276:
    // index1 = get_hash(se1->twin()->get_index());
    // index2 = get_hash(se2->twin()->get_index());
    // if(index1 < index2) { ... set_hash(se1->twin()->get_index(), index1); }
    // Typo: should be set_hash(se2->twin()->get_index(), index1);

    se1.index = 10;
    se1t.index = 11;
    se2.index = 20;
    se2t.index = 21;

    handler.initialize_hash(se1.index);
    handler.initialize_hash(se1t.index);
    handler.initialize_hash(se2.index);
    handler.initialize_hash(se2t.index);

    // Call handle_support. 
    // index1 will be 11, index2 will be 21. 11 < 21, so it enters the if branch.
    handler.handle_support(&se, &se1, &se2);

    std::cout << "After handle_support:" << std::endl;
    std::cout << "Hash(11) = " << handler.get_hash(11) << std::endl;
    std::cout << "Hash(21) = " << handler.get_hash(21) << std::endl;

    if (handler.get_hash(21) == 11) {
        std::cout << "SUCCESS: Indices unified correctly." << std::endl;
        return 0;
    } else {
        std::cout << "FAILURE: Twin indices NOT unified! (Expected get_hash(21) == 11, got " << handler.get_hash(21) << ")" << std::endl;
        return 1;
    }
}
