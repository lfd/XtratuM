/*
 * $FILE: list.h
 *
 * List
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_LIST_H_
#define _XM_LIST_H_

#ifndef __ASSEMBLY__
#include <assert.h>

struct dynListNode {
    struct dynListNode *prev, *next;
};

struct dynList {
    struct dynListNode *head;
    xm_s32_t noElem;
};

static inline void DynListInit(struct dynList *l) {
    l->noElem=0;
    l->head=0;
}

static inline xm_s32_t DynListInsertHead(struct dynList *l, struct dynListNode *e) {
    ASSERT(!e->next&&!e->prev);
    if (l->head) {
	e->next=l->head;
	e->prev=l->head->prev;
	l->head->prev->next=e;
	l->head->prev=e;	
    } else
	e->prev=e->next=e;
    l->head=e;
    ASSERT(l->noElem>=0);
    l->noElem++;

    return 0;
}

static inline void *DynListRemoveTail(struct dynList *l) {
    struct dynListNode *e=0;
    if (l->head) {
	e=l->head->prev;
	e->prev->next=e->next;
	e->next->prev=e->prev;
	if (l->head==e)
	    l->head=0;
	e->prev=e->next=0;
	l->noElem--;
    }
    ASSERT(l->noElem>=0);

    return e;
}

static inline xm_s32_t DynListRemoveElement(struct dynList *l, struct dynListNode *e) {
    ASSERT(e->prev&&e->next);
    e->prev->next=e->next;
    e->next->prev=e->prev;
    e->prev=e->next=0;
    if (l->head==e)
	l->head=0;
    l->noElem--;
    ASSERT(l->noElem>=0);

    return 0;
}

#define DYNLIST_FOR_EACH_ELEMENT_BEGIN(_l, _element, _cond) do { \
    xm_s32_t __e; \
    struct dynListNode *__n; \
    for (__e=(_l)->noElem, __n=(_l)->head, _element=(void *)__n; __e && (_cond); __e--, __n=__n->next, _element=(void *)__n) {


#define DYNLIST_FOR_EACH_ELEMENT_END(_l) \
    } \
} while(0)

#define DYNLIST_FOR_EACH_ELEMENT_EXIT(_l)

#if 0
#include <stdc.h>

struct list {
    xm_s32_t maxNoElem, elemSize, elem, head, tail;
    void *e;
};

static inline void ListInit(struct list *l, void *buffer, xm_s32_t noElem, xm_s32_t elemSize) {
    l->e=buffer;
    l->maxNoElem=noElem;
    l->elemSize=elemSize;
    l->head=l->tail=l->elem=0;
}

static inline xm_s32_t ListInsertTail(struct list *l, void *e) {
    if (l->elem<l->maxNoElem) {	
	memcpy(&((xm_u8_t *)l->e)[l->tail*l->elemSize], e, l->elemSize);
	l->tail=((l->tail+1)<l->maxNoElem)?l->tail+1:0;
	l->elem++;
	return 0;
    }

    return -1;
}

static inline xm_s32_t ListExtractHead(struct list *l, void *e) {
    if (l->elem) {
	memcpy(e, &((xm_u8_t *)l->e)[l->head*l->elemSize], l->elemSize);
	l->head=((l->head+1)<l->maxNoElem)?l->head+1:0;
	l->elem--;
	return 0;
    }

    return -1;
}

#endif
#endif
#endif
