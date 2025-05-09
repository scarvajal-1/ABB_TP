#include "abb.h"
#include "abb_estructura_privada.h"
#include <stdlib.h>

abb_t *abb_crear(int (*cmp)(const void *, const void *)) {
    abb_t *tree = malloc(sizeof(struct abb_t));

    if (tree == NULL) {
        return NULL;
    }

    tree->raiz = NULL;
    tree->comparador = cmp;
    tree->nodos = 0;

    return tree;
}


static nodo_t *create_tree_node(const void *element) {
    nodo_t *new_node = malloc(sizeof(nodo_t));
    if (new_node == NULL) {
        return NULL;
    }

    new_node->elemento = (void *)element;
    new_node->izq = NULL;
    new_node->der = NULL;

    return new_node;
}


static nodo_t *recursive_insert_node(nodo_t *current_node,
                                   const void *element,
                                   int (*comparator)(const void *, const void *),
                                   bool *was_inserted_flag) {
    if (current_node == NULL) {
        nodo_t *new_node = create_tree_node(element);
        if (new_node != NULL) {
            *was_inserted_flag = true;
        }
        return new_node;
    }

    int comparison_result = comparator(element, current_node->elemento);

    if (comparison_result < 0) {
        current_node->izq = recursive_insert_node(current_node->izq, element, comparator, was_inserted_flag);
    } else {
        current_node->der = recursive_insert_node(current_node->der, element, comparator, was_inserted_flag);
    }

    return current_node;
}


bool abb_insertar(abb_t *abb, const void *elemento) {
    if (abb == NULL) {
        return false;
    }

    bool was_inserted_successfully = false;

    abb->raiz = recursive_insert_node(abb->raiz, elemento, abb->comparador, &was_inserted_successfully);

    if (was_inserted_successfully) {
        abb->nodos++;
        return true;
    }

    return false;
}


static nodo_t *recursive_find_node(const nodo_t *current_node,
                                 const void *element,
                                 int (*comparator)(const void *, const void *)) {
    if (current_node == NULL) {
        return NULL;
    }

    int comparison_result = comparator(element, current_node->elemento);

    if (comparison_result == 0) {
        return (nodo_t *)current_node;
    } else if (comparison_result < 0) {
        return recursive_find_node(current_node->izq, element, comparator);
    } else {
        return recursive_find_node(current_node->der, element, comparator);
    }
}


bool abb_existe(const abb_t *abb, const void *elemento) {
    if (abb == NULL || abb->raiz == NULL) {
        return false;
    }

    nodo_t *found_node = recursive_find_node(abb->raiz, elemento, abb->comparador);

    return found_node != NULL;
}


void *abb_buscar(const abb_t *abb, const void *elemento) {
    if (abb == NULL || abb->raiz == NULL) {
        return NULL;
    }

    nodo_t *found_node = recursive_find_node(abb->raiz, elemento, abb->comparador);

    if (found_node != NULL) {
        return found_node->elemento;
    }

    return NULL;
}


static nodo_t *find_rightmost_node(nodo_t *current_node) {
    while (current_node != NULL && current_node->der != NULL) {
        current_node = current_node->der;
    }

    return current_node;
}


static nodo_t *recursive_remove_node(nodo_t *current_node,
                                   const void *element_to_remove,
                                   int (*comparator)(const void *, const void *),
                                   void **extracted_element_ptr,
                                   bool *node_was_removed_flag) {
    if (current_node == NULL) {
        return NULL;
    }

    int comparison_result = comparator(element_to_remove, current_node->elemento);

    if (comparison_result < 0) {
        current_node->izq = recursive_remove_node(current_node->izq, element_to_remove, comparator, extracted_element_ptr, node_was_removed_flag);
    } else if (comparison_result > 0) {
        current_node->der = recursive_remove_node(current_node->der, element_to_remove, comparator, extracted_element_ptr, node_was_removed_flag);
    } else {

        if (*extracted_element_ptr == NULL) {
            *extracted_element_ptr = current_node->elemento;
        }

        *node_was_removed_flag = true;

        if (current_node->izq == NULL) {
            nodo_t *right_child = current_node->der;
            free(current_node);
            return right_child;

        } else if (current_node->der == NULL) {
            nodo_t *left_child = current_node->izq;
            free(current_node);
            return left_child;

        } else {
            nodo_t *predecessor = find_rightmost_node(current_node->izq);
            current_node->elemento = predecessor->elemento;

            void* temp_extracted_element_for_predecessor = NULL;
            bool temp_predecessor_removed_flag = false;
            current_node->izq = recursive_remove_node(current_node->izq, predecessor->elemento, comparator, &temp_extracted_element_for_predecessor, &temp_predecessor_removed_flag);
        }
    }

    return current_node;
}


void *abb_sacar(abb_t *abb, const void *elemento) {
    if (abb == NULL || abb->raiz == NULL) {
        return NULL;
    }

    void *removed_element_data = NULL;
    bool node_removed_successfully = false;

    abb->raiz = recursive_remove_node(abb->raiz, elemento, abb->comparador, &removed_element_data, &node_removed_successfully);

    if (node_removed_successfully) {
        abb->nodos--;
    }

    return removed_element_data;
}


size_t abb_tamanio(const abb_t *abb) {
    if (abb == NULL) {
        return 0;
    }

    return abb->nodos;
}


bool abb_vacio(const abb_t *abb) {
    if (abb == NULL) {
        return true;
    }

    return abb->nodos == 0;
}


static void recursive_in_order_traverse(const nodo_t *current_node,
                                        bool (*visitor_function)(void *, void *),
                                        void *context,
                                        size_t *processed_count_ptr,
                                        bool *continue_traversal_flag_ptr) {
    if (current_node == NULL || !(*continue_traversal_flag_ptr)) {
        return;
    }

    recursive_in_order_traverse(current_node->izq, visitor_function, context, processed_count_ptr, continue_traversal_flag_ptr);

    if (!(*continue_traversal_flag_ptr)) {
        return;
    }

    if (visitor_function(current_node->elemento, context)) {
        (*processed_count_ptr)++;
    } else {
        *continue_traversal_flag_ptr = false;
        return;
    }

    if (!(*continue_traversal_flag_ptr)) {
        return;
    }

    recursive_in_order_traverse(current_node->der, visitor_function, context, processed_count_ptr, continue_traversal_flag_ptr);
}


static void recursive_pre_order_traverse(const nodo_t *current_node,
                                         bool (*visitor_function)(void *, void *),
                                         void *context,
                                         size_t *processed_count_ptr,
                                         bool *continue_traversal_flag_ptr) {
    if (current_node == NULL || !(*continue_traversal_flag_ptr)) {
        return;
    }

    if (visitor_function(current_node->elemento, context)) {
        (*processed_count_ptr)++;
    } else {
        *continue_traversal_flag_ptr = false;
        return;
    }

    if (!(*continue_traversal_flag_ptr)) {
        return;
    }

    recursive_pre_order_traverse(current_node->izq, visitor_function, context, processed_count_ptr, continue_traversal_flag_ptr);

    if (!(*continue_traversal_flag_ptr)) {
        return;
    }
    
    recursive_pre_order_traverse(current_node->der, visitor_function, context, processed_count_ptr, continue_traversal_flag_ptr);
}


static void recursive_post_order_traverse(const nodo_t *current_node,
                                          bool (*visitor_function)(void *, void *),
                                          void *context,
                                          size_t *processed_count_ptr,
                                          bool *continue_traversal_flag_ptr) {
    if (current_node == NULL || !(*continue_traversal_flag_ptr)) {
        return;
    }

    recursive_post_order_traverse(current_node->izq, visitor_function, context, processed_count_ptr, continue_traversal_flag_ptr);

    if (!(*continue_traversal_flag_ptr)) {
        return;
    }

    recursive_post_order_traverse(current_node->der, visitor_function, context, processed_count_ptr, continue_traversal_flag_ptr);
    
    if (!(*continue_traversal_flag_ptr)) {
        return;
    }

    if (visitor_function(current_node->elemento, context)) {
        (*processed_count_ptr)++;
    } else {
        *continue_traversal_flag_ptr = false;
    }
}


size_t abb_recorrer(const abb_t *abb, enum abb_recorrido modo,
                    bool (*f)(void *, void *), void *ctx) {
    if (abb == NULL || f == NULL) {
        return 0;
    }

    size_t processed_count = 0;
    bool continue_traversal = true;

    switch (modo) {
        case ABB_INORDEN:
            recursive_in_order_traverse(abb->raiz, f, ctx, &processed_count, &continue_traversal);
            break;
        case ABB_PREORDEN:
            recursive_pre_order_traverse(abb->raiz, f, ctx, &processed_count, &continue_traversal);
            break;
        case ABB_POSTORDEN:
            recursive_post_order_traverse(abb->raiz, f, ctx, &processed_count, &continue_traversal);
            break;
    }

    return processed_count;
}


static void recursive_in_order_vectorize(const nodo_t *current_node,
                                         void **output_vector,
                                         size_t vector_capacity,
                                         size_t *current_index_ptr) {
    if (current_node == NULL || *current_index_ptr >= vector_capacity) {
        return;
    }

    recursive_in_order_vectorize(current_node->izq, output_vector, vector_capacity, current_index_ptr);

    if (*current_index_ptr >= vector_capacity) {
        return;
    }

    output_vector[*current_index_ptr] = current_node->elemento;
    (*current_index_ptr)++;

    if (*current_index_ptr >= vector_capacity) {
        return;
    }

    recursive_in_order_vectorize(current_node->der, output_vector, vector_capacity, current_index_ptr);
}


static void recursive_pre_order_vectorize(const nodo_t *current_node,
                                          void **output_vector,
                                          size_t vector_capacity,
                                          size_t *current_index_ptr) {
    if (current_node == NULL || *current_index_ptr >= vector_capacity) {
        return;
    }

    output_vector[*current_index_ptr] = current_node->elemento;
    (*current_index_ptr)++;

    if (*current_index_ptr >= vector_capacity) {
        return;
    }

    recursive_pre_order_vectorize(current_node->izq, output_vector, vector_capacity, current_index_ptr);

    if (*current_index_ptr >= vector_capacity) {
        return;
    }

    recursive_pre_order_vectorize(current_node->der, output_vector, vector_capacity, current_index_ptr);
}


static void recursive_post_order_vectorize(const nodo_t *current_node,
                                           void **output_vector,
                                           size_t vector_capacity,
                                           size_t *current_index_ptr) {
    if (current_node == NULL || *current_index_ptr >= vector_capacity) {
        return;
    }

    recursive_post_order_vectorize(current_node->izq, output_vector, vector_capacity, current_index_ptr);

    if (*current_index_ptr >= vector_capacity) {
        return;
    }

    recursive_post_order_vectorize(current_node->der, output_vector, vector_capacity, current_index_ptr);

    if (*current_index_ptr >= vector_capacity) {
        return;
    }
    
    output_vector[*current_index_ptr] = current_node->elemento;
    (*current_index_ptr)++;
}


size_t abb_vectorizar(const abb_t *abb, enum abb_recorrido modo, void **vector,
                      size_t capacidad) {
    if (abb == NULL || vector == NULL || capacidad == 0) {
        return 0;
    }

    size_t stored_count = 0;

    switch (modo) {
        case ABB_INORDEN:
            recursive_in_order_vectorize(abb->raiz, vector, capacidad, &stored_count);
            break;
        case ABB_PREORDEN:
            recursive_pre_order_vectorize(abb->raiz, vector, capacidad, &stored_count);
            break;
        case ABB_POSTORDEN:
            recursive_post_order_vectorize(abb->raiz, vector, capacidad, &stored_count);
            break;
    }

    return stored_count;
}


static void recursive_destroy_nodes(nodo_t *current_node, void (*element_destructor)(void *)) {
    if (current_node == NULL) {
        return;
    }

    recursive_destroy_nodes(current_node->izq, element_destructor);
    recursive_destroy_nodes(current_node->der, element_destructor);

    if (element_destructor != NULL) {
        element_destructor(current_node->elemento);
    }
    
    free(current_node);
}


void abb_destruir(abb_t *abb) {
    if (abb == NULL) {
        return;
    }

    recursive_destroy_nodes(abb->raiz, NULL);
    free(abb);
}


void abb_destruir_todo(abb_t *abb, void (*destructor)(void *)) {
    if (abb == NULL) {
        return;
    }

    recursive_destroy_nodes(abb->raiz, destructor);
    free(abb);
}