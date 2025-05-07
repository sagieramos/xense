#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include "mac_table.h"

// Min-heap entry for tracking expirations
typedef struct {
    size_t slot_index;  // Slot index in the MAC table
    time_t expiry_time; // When the entry expires (timeout_seconds)
} HeapEntry;

// Min-heap structure
typedef struct {
    HeapEntry *entries; // Array of heap entries
    size_t size;        // Current number of entries
    size_t capacity;    // Maximum capacity
} MinHeap;

// Expiry manager structure
struct MacTableExpiryManager {
    MacTable *table;          // Reference to the MAC table
    MinHeap *heap;            // Min-heap for expiration times
    TimerHandle_t expiry_timer; // FreeRTOS timer
};

// Initialize min-heap
MinHeap *min_heap_create(size_t capacity) {
    MinHeap *heap = pvPortMalloc(sizeof(MinHeap));
    if (!heap) return NULL;
    heap->entries = pvPortMalloc(sizeof(HeapEntry) * capacity);
    if (!heap->entries) {
        vPortFree(heap);
        return NULL;
    }
    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

// Free min-heap
void min_heap_free(MinHeap *heap) {
    if (heap) {
        vPortFree(heap->entries);
        vPortFree(heap);
    }
}

// Get parent index in heap
static inline size_t heap_parent(size_t index) {
    return (index - 1) / 2;
}

// Get left child index in heap
static inline size_t heap_left_child(size_t index) {
    return 2 * index + 1;
}

// Get right child index in heap
static inline size_t heap_right_child(size_t index) {
    return 2 * index + 2;
}

// Swap two heap entries
static void heap_swap(MinHeap *heap, size_t i, size_t j) {
    HeapEntry temp = heap->entries[i];
    heap->entries[i] = heap->entries[j];
    heap->entries[j] = temp;
}

// Bubble up to maintain heap property
static void heap_bubble_up(MinHeap *heap, size_t index) {
    while (index > 0) {
        size_t parent = heap_parent(index);
        if (heap->entries[index].expiry_time >= heap->entries[parent].expiry_time) {
            break;
        }
        heap_swap(heap, index, parent);
        index = parent;
    }
}

// Bubble down to maintain heap property
static void heap_bubble_down(MinHeap *heap, size_t index) {
    size_t min_index = index;
    while (1) {
        size_t left = heap_left_child(index);
        size_t right = heap_right_child(index);
        
        if (left < heap->size && heap->entries[left].expiry_time < heap->entries[min_index].expiry_time) {
            min_index = left;
        }
        if (right < heap->size && heap->entries[right].expiry_time < heap->entries[min_index].expiry_time) {
            min_index = right;
        }
        if (min_index == index) {
            break;
        }
        heap_swap(heap, index, min_index);
        index = min_index;
    }
}

// Insert entry into heap
int min_heap_insert(MinHeap *heap, size_t slot_index, time_t expiry_time) {
    if (heap->size >= heap->capacity) {
        return -1; // Heap full
    }
    heap->entries[heap->size].slot_index = slot_index;
    heap->entries[heap->size].expiry_time = expiry_time;
    heap_bubble_up(heap, heap->size);
    heap->size++;
    return 0;
}

// Remove entry from heap by slot_index
void min_heap_remove(MinHeap *heap, size_t slot_index) {
    for (size_t i = 0; i < heap->size; i++) {
        if (heap->entries[i].slot_index == slot_index) {
            heap->entries[i] = heap->entries[heap->size - 1];
            heap->size--;
            if (i < heap->size) {
                heap_bubble_up(heap, i);
                heap_bubble_down(heap, i);
            }
            break;
        }
    }
}

// Get earliest expiration time
time_t min_heap_peek(MinHeap *heap) {
    if (heap->size == 0) {
        return 0; // No entries
    }
    return heap->entries[0].expiry_time;
}

// Pop earliest entry
int min_heap_pop(MinHeap *heap, size_t *slot_index, time_t *expiry_time) {
    if (heap->size == 0) {
        return -1; // Empty heap
    }
    *slot_index = heap->entries[0].slot_index;
    *expiry_time = heap->entries[0].expiry_time;
    heap->entries[0] = heap->entries[heap->size - 1]; 
    heap->size--;
    if (heap->size > 0) {
        heap_bubble_down(heap, 0);
    }
    return 0;
}

// FreeRTOS timer callback
static void expiry_timer_callback(TimerHandle_t xTimer) {
    MacTableExpiryManager *manager = pvTimerGetTimerID(xTimer);
    MinHeap *heap = manager->heap;
    MacTable *table = manager->table;
    
    time_t now = time(NULL);
    while (heap->size > 0 && min_heap_peek(heap) <= now) {
        size_t slot_index;
        time_t expiry_time;
        if (min_heap_pop(heap, &slot_index, &expiry_time) != 0) {
            break;
        }
        
        if (slot_index < table->size && table->entries[slot_index].state == SLOT_OCCUPIED &&
            table->entries[slot_index].timeout_seconds == expiry_time) {
            if (table->on_event) {
                table->on_event(slot_index, MAC_TABLE_TIMEOUT);
            }
        }
    }
    
    // Restart timer for next expiration
    if (heap->size > 0) {
        time_t next_expiry = min_heap_peek(heap);
        time_t now = time(NULL);
        TickType_t ticks = (next_expiry > now) ? pdMS_TO_TICKS((next_expiry - now) * 1000) : 1;
        xTimerChangePeriod(xTimer, ticks, 0);
        xTimerStart(xTimer, 0);
    }
}

// Initialize expiry manager
MacTableExpiryManager *expiry_manager_create(MacTable *table) {
    MacTableExpiryManager *manager = pvPortMalloc(sizeof(MacTableExpiryManager));
    if (!manager) return NULL;
    
    manager->table = table;
    manager->heap = min_heap_create(table->size);
    if (!manager->heap) {
        vPortFree(manager);
        return NULL;
    }
    
    manager->expiry_timer = xTimerCreate("ExpiryTimer", pdMS_TO_TICKS(1000), pdFALSE, manager, expiry_timer_callback);
    if (!manager->expiry_timer) {
        min_heap_free(manager->heap);
        vPortFree(manager);
        return NULL;
    }
    
    return manager;
}

// Free expiry manager
void expiry_manager_free(MacTableExpiryManager *manager) {
    if (manager) {
        if (manager->expiry_timer) {
            xTimerStop(manager->expiry_timer, 0);
            xTimerDelete(manager->expiry_timer, 0);
        }
        min_heap_free(manager->heap);
        vPortFree(manager);
    }
}

// Notify manager of entry addition or update
void expiry_manager_add_or_update(MacTableExpiryManager *manager, size_t slot_index) {
    if (!manager || slot_index >= manager->table->size) return;
    
    MacTableEntry *entry = &manager->table->entries[slot_index];
    if (entry->state != SLOT_OCCUPIED) return;
    
    // Remove existing entry for this slot
    min_heap_remove(manager->heap, slot_index);
    
    // Add new expiration time
    time_t expiry_time = entry->timeout_seconds;
    min_heap_insert(manager->heap, slot_index, expiry_time);
    
    // Update timer
    time_t now = time(NULL);
    time_t next_expiry = min_heap_peek(manager->heap);
    if (next_expiry > 0 && (xTimerIsTimerActive(manager->expiry_timer) == pdFALSE || next_expiry <= now)) {
        TickType_t ticks = (next_expiry > now) ? pdMS_TO_TICKS((next_expiry - now) * 1000) : 1;
        xTimerChangePeriod(manager->expiry_timer, ticks, 0);
        xTimerStart(manager->expiry_timer, 0);
    }
}

// Notify manager of entry deletion
void expiry_manager_delete(MacTableExpiryManager *manager, size_t slot_index) {
    if (!manager || slot_index >= manager->table->size) return;
    min_heap_remove(manager->heap, slot_index);
    
    // Update timer if heap is not empty
    if (manager->heap->size > 0) {
        time_t now = time(NULL);
        time_t next_expiry = min_heap_peek(manager->heap);
        TickType_t ticks = (next_expiry > now) ? pdMS_TO_TICKS((next_expiry - now) * 1000) : 1;
        xTimerChangePeriod(manager->expiry_timer, ticks, 0);
        xTimerStart(manager->expiry_timer, 0);
    } else {
        xTimerStop(manager->expiry_timer, 0);
    }
}

#ifdef __cplusplus
}
#endif