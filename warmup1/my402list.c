#include "my402list.h"
#include <stdlib.h>	
#include <string.h>
#include <malloc.h>
#include <stdbool.h>

int My402ListInit(My402List *head){
	if(head == NULL) return false;
	head->num_members = 0;
	My402ListElem *addr = &(head->anchor);
	(head->anchor).next = (head->anchor).prev = addr;
	(head->anchor).obj = NULL;
	return true;
}

int My402ListLength(My402List *head){
//	My402ListElem *ptr = &(head->anchor);
//	int count = 0;
//	if(ptr->next == ptr || ptr->prev == ptr) return 0;
//	else{		
//		My402ListElem *cur = ptr->next;
//		while(cur != ptr){
//			count++;
//			cur = cur->next;
//		}
//	}
//	return count;
	return head->num_members;
}

int My402ListEmpty(My402List *head){
	// if(head->num_members == 0) return true;
	// else return false;
//	My402ListElem *ptr = &(head->anchor);
//	if(ptr->next == ptr || ptr->prev == ptr) return TRUE;
//	else return FALSE;
	if((head->num_members) > 0) return false;
	else return true;
}

My402ListElem *My402ListFirst(My402List *head){
	if(My402ListEmpty(head)) return NULL;
	else return (head->anchor).next;
}

My402ListElem *My402ListLast(My402List *head){
	if(My402ListEmpty(head)) return NULL;
	else return (head->anchor).prev;
}

My402ListElem *My402ListNext(My402List *head,My402ListElem *obj){
	My402ListElem *cur = (My402ListElem*)obj;
	if(My402ListLast(head) == cur) return NULL;
	else{
		return cur->next;
	}
}

My402ListElem *My402ListPrev(My402List *head, My402ListElem *obj){
	if(My402ListFirst(head) == obj) return NULL;
	else{
		return obj->prev;
	}
}

My402ListElem *My402ListFind(My402List *head, void *obj){
	if(My402ListEmpty(head)) return NULL;
	My402ListElem *ptr = &(head->anchor);
	My402ListElem *anchor = &(head->anchor), *elem = NULL;
	ptr = ptr->next;
	while(ptr != anchor){
		if(ptr->obj == obj){
			elem = ptr;
			break;
		}
		ptr = ptr->next;
	}
	return elem;
}

int My402ListAppend(My402List *head, void *obj){
	if(head == NULL) return false;
	if(My402ListEmpty(head)){
		My402ListElem *elem = NULL;
		while(elem == NULL)
			elem = (My402ListElem*)malloc(sizeof(My402ListElem));
		elem->obj = obj;
		(head->anchor).next = elem;
		elem->prev = &(head->anchor);
		elem->next = &(head->anchor);
		(head->anchor).prev = elem;
		head->num_members++;
		return true;
	}else{
		My402ListElem *last = My402ListLast(head);
		My402ListElem *elem = NULL;
		while(elem == NULL)
			elem = (My402ListElem*)malloc(sizeof(My402ListElem));
		elem->obj = obj;
		last->next = elem;
		elem->prev = last;
		elem->next = &(head->anchor);
		(head->anchor).prev = elem;
		head->num_members++;
		return true;
	}
}

int My402ListPrepend(My402List *head, void *obj){
	if(head == NULL) return false;
	if(My402ListEmpty(head)){
		My402ListElem *elem = NULL;
		while(elem == NULL)
			elem = (My402ListElem*)malloc(sizeof(My402ListElem));
		elem->obj = obj;
		// My402ListElem *cur = (My402ListElem*)obj;
		(head->anchor).next = elem;
		elem->prev = &(head->anchor);
		elem->next = &(head->anchor);
		(head->anchor).prev = elem;
		head->num_members++;
		return true;
	}else{
		My402ListElem *first = My402ListFirst(head);
		My402ListElem *elem = NULL;
		while(elem == NULL)
			elem = (My402ListElem*)malloc(sizeof(My402ListElem));
		elem->obj = obj;
		(head->anchor).next = elem;
		elem->prev = &(head->anchor);
		elem->next = first;
		first->prev = elem;
		head->num_members++;
		return true;
	}
}

void My402ListUnlink(My402List *head, My402ListElem *obj){
	if(head == NULL) return;
	My402ListElem *ptr = &(head->anchor);
	while(ptr != obj)
		ptr = ptr->next; //no check of existing the element
	obj->prev->next = obj->next;
	obj->next->prev = obj->prev;
	free(obj);
	ptr = NULL;
	head->num_members--;
	return;
}

void My402ListUnlinkAll(My402List *head){
	if(head == NULL) return;
	My402ListElem *ptr = &(head->anchor), *anchor = &(head->anchor);
	ptr = ptr->next;
	while(ptr != anchor){
		My402ListElem *next = ptr->next;	
	//	free(ptr);
		My402ListUnlink(head,ptr);
		ptr = next;
	}
	head->num_members = 0;
//	(head->anchor).prev = (head->anchor).next = anchor;
//	return;
}

int My402ListInsertAfter(My402List *head, void *obj, My402ListElem *elem){
	if(head == NULL) return false;
	if(elem == NULL){
		if(My402ListAppend(head,obj)) return true;
		else return false;
	}else{
		if(My402ListEmpty(head)) return My402ListAppend(head,obj);
		else if(My402ListLast(head) == elem){
			bool result = My402ListAppend(head,obj);
			return result;
		}else{
			My402ListElem *cur = (My402ListElem*)malloc(sizeof(My402ListElem));
			cur->obj = obj;
			My402ListElem *after = elem->next;
			elem->next = cur;
			cur->prev = elem;
			cur->next = after;
			after->prev = cur;
			head->num_members++;
			return true;
		}	
	}
}

int My402ListInsertBefore(My402List *head, void *obj, My402ListElem *elem){
	if(head == NULL) return false;
	if(elem == NULL){
		if(My402ListPrepend(head,obj)) return true;
		else return false;
	}else{
		//My402ListElem *ptr = &(head->anchor);
		if(My402ListEmpty(head)) return My402ListAppend(head,obj);
		else{
			if(My402ListFirst(head) == elem){
				bool result = My402ListPrepend(head,obj);
				return result;
			}else{
				My402ListElem *before = elem->prev;
				My402ListElem *cur = (My402ListElem*)malloc(sizeof(My402ListElem));
				cur->obj = obj;
				before->next = cur;
				cur->prev = before;
				cur->next = elem;
				elem->prev = cur;
				head->num_members++;
				return true;
			}		
			// while(ptr != elem)
			// 	ptr = ptr->next;
			// if(ptr == elem){
			// 	My402ListElem *cur = (My402ListElem*)malloc(sizeof(My402ListElem));
			// 	My402ListElem *prev = elem->prev;
			// 	cur->next = elem;
			// 	elem->prev = cur;
			// 	prev->next = cur;
			// 	cur->prev = prev;
			// 	cur->obj = obj;
			// 	head->num_members++;
			// 	return	true;
		}
		}
	}




