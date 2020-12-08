/**
 * Vector
 * CS 241 - Spring 2020
 */
#include "vector.h"
#include <assert.h>

/**
 * 'INITIAL_CAPACITY' the initial size of the dynamically.
 */
const size_t INITIAL_CAPACITY = 8;
/**
 * 'GROWTH_FACTOR' is how much the vector will grow by in automatic reallocation
 * (2 means double).
 */
const size_t GROWTH_FACTOR = 2;

struct vector {
    /* The function callback for the user to define the way they want to copy
     * elements */
    copy_constructor_type copy_constructor;

    /* The function callback for the user to define the way they want to destroy
     * elements */
    destructor_type destructor;

    /* The function callback for the user to define the way they a default
     * element to be constructed */
    default_constructor_type default_constructor;

    /* Void pointer to the beginning of an array of void pointers to arbitrary
     * data. */
    void **array;

    /**
     * The number of elements in the vector.
     * This is the number of actual objects held in the vector,
     * which is not necessarily equal to its capacity.
     */
    size_t size;

    /**
     * The size of the storage space currently allocated for the vector,
     * expressed in terms of elements.
     */
    size_t capacity;
};

/**
 * IMPLEMENTATION DETAILS
 *
 * The following is documented only in the .c file of vector,
 * since it is implementation specfic and does not concern the user:
 *
 * This vector is defined by the struct above.
 * The struct is complete as is and does not need any modifications.
 *
 * The only conditions of automatic reallocation is that
 * they should happen logarithmically compared to the growth of the size of the
 * vector inorder to achieve amortized constant time complexity for appending to
 * the vector.
 *
 * For our implementation automatic reallocation happens when -and only when-
 * adding to the vector makes its new  size surpass its current vector capacity
 * OR when the user calls on vector_reserve().
 * When this happens the new capacity will be whatever power of the
 * 'GROWTH_FACTOR' greater than or equal to the target capacity.
 * In the case when the new size exceeds the current capacity the target
 * capacity is the new size.
 * In the case when the user calls vector_reserve(n) the target capacity is 'n'
 * itself.
 * We have provided get_new_capacity() to help make this less ambigious.
 */

static size_t get_new_capacity(size_t target) {
    /**
     * This function works according to 'automatic reallocation'.
     * Start at 1 and keep multiplying by the GROWTH_FACTOR untl
     * you have exceeded or met your target capacity.
     */
    size_t new_capacity = 1;
    while (new_capacity < target) {
        new_capacity *= GROWTH_FACTOR;
    }
    return new_capacity;
}

vector *vector_create(copy_constructor_type copy_constructor,
                      destructor_type destructor,
                      default_constructor_type default_constructor) {
    // your code here
    // Casting to void to remove complier error. Remove this line when you are
    // ready.
    // (void)INITIAL_CAPACITY;
    // (void)get_new_capacity;

    vector *this = malloc(sizeof(vector));
    // Initialize size and capacity
    this->size = 0;
    this->capacity = INITIAL_CAPACITY;
    // Initialize the three function ptr
    this->copy_constructor = (copy_constructor == NULL) ? shallow_copy_constructor : copy_constructor ;
    this->destructor = (destructor == NULL) ? shallow_destructor : destructor;
    this->default_constructor = (default_constructor == NULL) ? shallow_default_constructor : default_constructor;
    // Get the default value of elements
    // void *default_val_ptr = this->default_constructor();
    // Allocate memory for array
    size_t nbytes = this->capacity * sizeof(void *);
    this->array = (void **) malloc(nbytes);
    // // Initialize elements to default values
    // // Set all ptrs in array to NULL if no default value given, 
    // if (default_val_ptr == NULL) {
    //     // memset(this->array, 0, nbytes);
    //     for (size_t i = 0; i < this->capacity; i++) {
    //         this->array[i] = NULL;
    //     }
    //     return this;
    // }
    // otherwise set all elements to the default value
    for (size_t i = 0; i < this->capacity; i++) {
        // this->array[i] = this->copy_constructor(default_val_ptr);
        this->array[i] = this->default_constructor();
    }
    // free(default_val_ptr);
    // default_val_ptr = NULL;
    return this;
}

void vector_destroy(vector *this) {
    assert(this);
    // your code here
    for (size_t i = 0; i < this->capacity; i++) {
        if (this->array[i] != NULL) {
            this->destructor(this->array[i]);
            this->array[i] = NULL;
        }
    }
    free(this->array);
    this->array = NULL;
    free(this);
    this = NULL;
}

void **vector_begin(vector *this) {
    return this->array + 0;
}

void **vector_end(vector *this) {
    return this->array + this->size;
}

size_t vector_size(vector *this) {
    assert(this);
    // your code here
    return this->size;
}

void vector_resize(vector *this, size_t n) {
    assert(this);
    // your code here

    // Do nothing if n equals to current size
    if (n == this->size) {return;}

    // Set the nth and following elements to default value
    // if n is smaller thatn current size.
    // Remember to destroy the original elements.
    if (n < this->size) {
        for (size_t i = n; i < this->size; i++) {
            if (this->array[i] != NULL) {
                free(this->array[i]);
            }
            this->array[i] = this->default_constructor();
        }
        this->size = n;
        return;
    }

    // Expand the size by filling default value 
    // if n is bigger than current size and smaller or equal to current capacity.
    if (n <= this->capacity) {
        // for (size_t i = this->size; i < this->capacity; i++) {
        //     this->array[i] = this->default_constructor();
        // }
        this->size = n;
        return;
    }

    // Reallocate memory, copy the original elements 
    // and initialize other elements to default value
    // if n is bigger than the current capacity.
    size_t new_capacity = get_new_capacity(n);
    void **new_array = (void **) malloc(new_capacity * sizeof(void *));
    // Copy all elements form the old array and free the old element pointers 
    for (size_t i = 0; i < this->capacity; i++) {
        new_array[i] = this->copy_constructor(this->array[i]);
        free(this->array[i]);
        this->array[i] = NULL;
    }
    free(this->array);
    for (size_t i = this->capacity; i < new_capacity; i++) {
        new_array[i] = this->default_constructor();
    }
    this->array = new_array;
    this->size = n;
    this->capacity = new_capacity;
    return;
}

size_t vector_capacity(vector *this) {
    assert(this);
    // your code here
    return this->capacity;
}

bool vector_empty(vector *this) {
    assert(this);
    // your code here
    return !(this->size);
}

void vector_reserve(vector *this, size_t n) {
    assert(this);
    // your code here
    if (n > this->capacity) {
        vector_resize(this, n);
    }
    // void **left = vector_front(this);
    // void **right = vector_back(this);
    // void *tmp = NULL;
    // while (left < right) {
    //     tmp = *left;
    //     *left = *right;
    //     *right = tmp;
    //     left++;
    //     right--;
    // }
}

void **vector_at(vector *this, size_t position) {
    assert(this);
    // your code here
    assert(position < this->size);
    return &this->array[position];
}

void vector_set(vector *this, size_t position, void *element) {
    assert(this);
    // your code here
    assert(position < this->size);
    if (this->array[position] != NULL) {
        free(this->array[position]);
    } 
    this->array[position] = this->copy_constructor(element);
}

void *vector_get(vector *this, size_t position) {
    assert(this);
    // your code here
    assert(position < this->capacity);
    return this->array[position];
}

void **vector_front(vector *this) {
    assert(this);
    // your code here
    return this->array + 0;
}

void **vector_back(vector *this) {
    assert(this);
    // your code here
    return this->array + this->size - 1;
}

void vector_push_back(vector *this, void *element) {
    assert(this);
    // your code here
    size_t old_size = this->size;
    if (this->size == this->capacity) {
        vector_resize(this, this->size + 1);
    }
    if (this->array[old_size] != NULL) {
        free(this->array[old_size]);
    }
    this->array[old_size] = this->copy_constructor(element);
    this->size = old_size + 1;
}

void vector_pop_back(vector *this) {
    assert(this);
    // your code here
    void **vec_back = vector_back(this);
    free(*vec_back);
    *vec_back = this->default_constructor();
    this->size--;
}

void vector_insert(vector *this, size_t position, void *element) {
    assert(this);
    // your code here
    assert(position <= this->size);
    // Push to the back if position equals to vector size
    if (position == this->size) {
        vector_push_back(this, element);
        return;
    }
    // Resize if capacity reached
    size_t old_size = this->size;
    if (this->size == this->capacity) {
        vector_resize(this, this->size + 1);
    }

    // Use two temp variables to help switch elements around
    // Don't forget destroy the end elem which was the default value.
    void *prev_elem = this->array[position]; 
    void *curr_elem = NULL;
    this->array[position] = this->copy_constructor(element);
    for (size_t i = position + 1; i < old_size; i++) {
        curr_elem = this->array[i];
        this->array[i] = prev_elem;
        prev_elem = curr_elem;
    }
    if (this->array[old_size] != NULL) {
        free(this->array[old_size]);
    }
    this->array[old_size] = prev_elem;
    this->size = old_size + 1;
}

void vector_erase(vector *this, size_t position) {
    assert(this);
    assert(position < vector_size(this));
    // your code here
    free(this->array[position]);
    for (size_t i = position; i < this->size - 1; i++) {
        this->array[i] = this->array[i + 1];
    }
    this->array[this->size - 1] = this->default_constructor();
    this->size--;
}

void vector_clear(vector *this) {
    // your code here
    assert(this);
    size_t nelems = this->size;
    for (size_t i = 0; i < nelems; i++) {
        vector_erase(this, 0);
    }
}

// The following is code generated:
vector *shallow_vector_create() {
    return vector_create(shallow_copy_constructor, shallow_destructor,
                         shallow_default_constructor);
}
vector *string_vector_create() {
    return vector_create(string_copy_constructor, string_destructor,
                         string_default_constructor);
}
vector *char_vector_create() {
    return vector_create(char_copy_constructor, char_destructor,
                         char_default_constructor);
}
vector *double_vector_create() {
    return vector_create(double_copy_constructor, double_destructor,
                         double_default_constructor);
}
vector *float_vector_create() {
    return vector_create(float_copy_constructor, float_destructor,
                         float_default_constructor);
}
vector *int_vector_create() {
    return vector_create(int_copy_constructor, int_destructor,
                         int_default_constructor);
}
vector *long_vector_create() {
    return vector_create(long_copy_constructor, long_destructor,
                         long_default_constructor);
}
vector *short_vector_create() {
    return vector_create(short_copy_constructor, short_destructor,
                         short_default_constructor);
}
vector *unsigned_char_vector_create() {
    return vector_create(unsigned_char_copy_constructor,
                         unsigned_char_destructor,
                         unsigned_char_default_constructor);
}
vector *unsigned_int_vector_create() {
    return vector_create(unsigned_int_copy_constructor, unsigned_int_destructor,
                         unsigned_int_default_constructor);
}
vector *unsigned_long_vector_create() {
    return vector_create(unsigned_long_copy_constructor,
                         unsigned_long_destructor,
                         unsigned_long_default_constructor);
}
vector *unsigned_short_vector_create() {
    return vector_create(unsigned_short_copy_constructor,
                         unsigned_short_destructor,
                         unsigned_short_default_constructor);
}
